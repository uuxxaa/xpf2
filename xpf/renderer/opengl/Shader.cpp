#include "Shader.h"
#include <core/Color.h>
#include <core/Log.h>
#include <fstream>
#include <sstream>
#include <glad/glad.h>

namespace xpf {

Shader::~Shader() {
    if (m_id >= 0)
        glDeleteProgram(m_id);
}

#pragma region uniform
int32_t Shader::get_uniform_location(std::string_view name) const {
    return glGetUniformLocation(m_id, name.data());
}

void Shader::set_uniform(int32_t id, bool value) {
    glUniform1i(id, value);
}

void Shader::set_uniform(int32_t id, int32_t value) {
    glUniform1i(id, value);
}

void Shader::set_uniform(int32_t id, float value) {
    glUniform1f(id, value);
}

void Shader::set_uniform(int32_t id, float x, float y) {
    glUniform2f(id, x, y);
}

void Shader::set_uniform(int32_t id, float x, float y, float z) {
    glUniform3f(id, x, y, z);
}

void Shader::set_uniform(int32_t id, float x, float y, float z, float w) {
    glUniform4f(id, x, y, z, w);
}

void Shader::set_uniform(int32_t id, const v2_t& vec) {
    glUniform2fv(id, 1, vec.f);
}

void Shader::set_uniform(int32_t id, const v3_t& vec) {
    glUniform3fv(id, 1, vec.f);
}

void Shader::set_uniform(int32_t id, const v4_t& vec) {
    glUniform4fv(id, 1, vec.f);
}

// https://austinmorlan.com/posts/opengl_matrices/
void Shader::set_uniform(int32_t id, const m3_t& mat) {
    glUniformMatrix3fv(id, 1, /*transpose:*/ GL_FALSE, mat.f);
}

void Shader::set_uniform(int32_t id, const m4_t& mat) {
    glUniformMatrix4fv(id, 1, /*transpose:*/ GL_FALSE, mat.f);
}

void Shader::set_uniform(int32_t id, const xpf::Color& color) {
    glUniform4fv(id, 1, color.get_vec4().f);
}
#pragma endregion

#pragma region loading
/*static*/ Shader Shader::load_from_file(const std::vector<std::pair<Shader::shader_type, std::string_view>>& shader_files) {
    std::vector<std::pair<Shader::shader_type, std::string_view>> shader_code;
    for (const auto& entry : shader_files) {
        shader_code.emplace_back(entry.first, Shader::load_shader_from_file(entry.second));
    }

    return Shader::load(shader_code);
}

/*static*/ Shader Shader::load(const std::vector<std::pair<Shader::shader_type, std::string_view>>& code) {
    std::vector<int32_t> shader_ids;
    for (const auto& entry : code) {
        int32_t id = compile_shader(entry.second, entry.first);
        if (id < 0) {
            Log::error("failed to compile shader: " + std::to_string(entry.first));
            for (int i : shader_ids)
                glDeleteShader(i);
            throw std::exception();
        }

        shader_ids.push_back(id);
    }

    GLuint shader_id = glCreateProgram();
    if (shader_id == 0) {
        Log::error("failed to create shader program");
        throw std::exception();
    }

    for (int32_t id : shader_ids)
        glAttachShader(shader_id, id);

    glLinkProgram(shader_id);

    GLint result = GL_FALSE;
    int infolog_length = 0;
    glGetProgramiv(shader_id, GL_LINK_STATUS, &result);
    glGetProgramiv(shader_id, GL_INFO_LOG_LENGTH, &infolog_length);

    if (infolog_length > 0)
    {
        std::string error;
        error.resize(infolog_length);

        glGetShaderInfoLog(shader_id, infolog_length, nullptr, &error[0]);
        Log::error(std::string("failed to link shaders: ") + error);
        throw std::exception();
    }

    for (int32_t id : shader_ids) {
        glDetachShader(shader_id, id);
        glDeleteShader(id);
    }

    return Shader(shader_id);
}

/*static*/ int32_t Shader::compile_shader(std::string_view shader_code, shader_type type) {
    const char* pstr = shader_code.data();
    if (shader_code.empty())
        return -1;

    GLuint shader_id = glCreateShader(type == shader_type::vertex
        ? GL_VERTEX_SHADER
        : (type == shader_type::fragment ? GL_FRAGMENT_SHADER : GL_GEOMETRY_SHADER));

    glShaderSource(shader_id, 1, &pstr, nullptr);
    glCompileShader(shader_id);

    GLint result = GL_FALSE;
    int infolog_length = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &infolog_length);

    if (infolog_length > 0)
    {
        std::string error;
        error.resize(infolog_length);

        glGetShaderInfoLog(shader_id, infolog_length, nullptr, &error[0]);
        Log::error("failed to compile shader (" + std::to_string(type) + "): " + error);
        glDeleteShader(shader_id);
        return -1;
    }

    return shader_id;
}

/*static*/ std::string Shader::load_files(std::initializer_list<std::string_view> files) {
    std::stringstream sstr;

    for (const auto& file : files) {
        std::ifstream stream(file.data(), std::ios::in);
        if (!stream.is_open())
            throw std::exception();

        sstr << stream.rdbuf();
        stream.close();
    }

    return sstr.str();
}

/*static*/ std::string Shader::load_shader_from_file(std::string_view file_path) {
    std::ifstream stream(file_path.data(), std::ios::in);
    if (!stream.is_open()) {
        Log::error(std::string("failed to open ") + std::string(file_path));
        throw std::exception();
    }

    std::stringstream sstr;
    sstr << stream.rdbuf();
    stream.close();

    return sstr.str();
}
#pragma endregion

void dump_shader_variables(uint32_t shader_id) {
    int32_t count;
    GLint size; // size of the variable
    GLenum type; // type of the variable (float, vec3 or mat4, etc)

    const GLsizei bufSize = 1024; // maximum name length
    GLchar name[bufSize]; // variable name in GLSL
    GLsizei length; // name length

    glGetProgramiv(shader_id, GL_ACTIVE_ATTRIBUTES, &count);
    printf("Active Attributes: %d\n", count);

    for (int32_t i = 0; i < count; i++)
    {
        glGetActiveAttrib(shader_id, (GLuint)i, bufSize, &length, &size, &type, name);

        printf("Attribute #%d Type: %u Name: %s\n", i, type, name);
    }

    glGetProgramiv(shader_id, GL_ACTIVE_UNIFORMS, &count);
    for (int32_t i = 0; i < count; i++)
    {
        glGetActiveUniform(shader_id, (GLuint)i, bufSize, &length, &size, &type, name);
        printf("Uniform #%d Type: %u Name: %s\n", i, type, name);
    }
}
} // xpf