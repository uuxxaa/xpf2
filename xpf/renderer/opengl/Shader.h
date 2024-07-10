#pragma once
#include <stdint.h>
#include <math/m3_t.h>
#include <math/m4_t.h>

namespace xpf {

struct Color;

typedef int32_t shader_t;

class Shader
{
public:
    enum shader_type
    {
        fragment,
        vertex,
        geometry,
    };

protected:
    shader_t m_id = -1;

protected:
    Shader(shader_t id) : m_id(id) { }

public:
    Shader() = default;
    Shader(Shader&& other) : m_id(other.m_id) { other.m_id = -1; }
    ~Shader();

    Shader& operator=(Shader&& other) { m_id = other.m_id; other.m_id = -1; return *this; }

    bool is_valid() const { return m_id != -1; }
    shader_t id() const { return m_id; }

#pragma region uniform
public:
    int32_t get_uniform_location(std::string_view name) const;

    static void set_uniform(int32_t id, bool value);
    static void set_uniform(int32_t id, int32_t value);
    static void set_uniform(int32_t id, float value);
    static void set_uniform(int32_t id, float x, float y);
    static void set_uniform(int32_t id, float x, float y, float z);
    static void set_uniform(int32_t id, float x, float y, float z, float w);

    static void set_uniform(int32_t id, const v2_t& value);
    static void set_uniform(int32_t id, const v3_t& value);
    static void set_uniform(int32_t id, const v4_t& value);

    static void set_uniform(int32_t id, const m3_t& value);
    static void set_uniform(int32_t id, const m4_t& value);

    static void set_uniform(int32_t id, const Color& color);
#pragma endregion

#pragma region loading
public:
    static Shader load(const std::vector<std::pair<Shader::shader_type, std::string_view>>& code);

private:
    static Shader load_from_file(const std::vector<std::pair<Shader::shader_type, std::string_view>>& shader_files);
    static std::string load_files(std::initializer_list<std::string_view> files);
    static std::string load_shader_from_file(std::string_view file);
    static int32_t compile_shader(std::string_view shader_code, shader_type type);
#pragma endregion
};

} // xpf