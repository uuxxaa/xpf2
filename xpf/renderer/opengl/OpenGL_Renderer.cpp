#include <common/Common_Renderer.h>
#include <opengl/Shader.h>
#include <core/Image.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <renderer/IRenderer.h>
#include <renderer/ITexture.h>

namespace xpf::resources {
std::string_view opengl_shader_vert();
std::string_view opengl_shader_frag();
}

namespace xpf {

typedef uint32_t vao_t;
typedef uint32_t vbo_t;
typedef uint32_t ebo_t;
typedef uint32_t textureid_t;

std::shared_ptr<ITexture> OpenGL_CreateTexture(const Image& img); // defined in OpenGL_Texture.cpp
std::shared_ptr<IBuffer> OpenGL_CreateBuffer(const byte_t* pbyte, size_t size); // defined in OpenGL_Buffer.cpp

class OpenGLRenderer : public CommonRenderer
{
protected:
    xpf::Shader m_shader;
    vao_t m_vao = 0;
    vbo_t m_vbo = 0;
    ebo_t m_ebo = 0;

    // vertex shader
    static inline int32_t u_projection;
    static inline int32_t u_view_matrix;
    static inline int32_t u_transform;
    static inline int32_t u_command_id;
    static inline int32_t u_corner_radius;
    static inline int32_t u_border_thickness;
    static inline int32_t u_border_color;
    static inline int32_t u_size;

public:
    OpenGLRenderer() = default;

    virtual ~OpenGLRenderer()
    {
        if (m_vbo != 0) glDeleteBuffers(1, &m_vbo);
        if (m_ebo != 0) glDeleteBuffers(1, &m_ebo);
        if (m_vao != 0) glDeleteVertexArrays(1, &m_vao);
    }

    virtual GLFWwindow* Initialize(RendererOptions&& optionsIn) override
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, optionsIn.api_version_major);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, optionsIn.api_version_minor);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Don't use old openGL
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // OSX Requires forward compatibility
        // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        if (optionsIn.sample_count > 0)
            glfwWindowHint(GLFW_SAMPLES, optionsIn.sample_count);

        if (CommonRenderer::Initialize(std::move(optionsIn)) == nullptr)
            return nullptr;

        glfwMakeContextCurrent(m_pWindow);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            return nullptr;
        }

        LoadShader();

        glfwSwapInterval(m_options.enable_vsync ? 1 : 0);

        return m_pWindow;
    }

    virtual void Shutdown() override {}

    virtual void OnResize(int32_t width, int32_t height) override
    {
        CommonRenderer::OnResize(width, height);
        glViewport(0, 0, width, height);
    }

    virtual void Render() override
    {
        m_frame_count++;
        m_renderStats = {};
        const ITexture* pCurrentTexture = nullptr;

        m4_t transform = m4_t::identity;
        std::vector<m4_t> transforms({transform});
        std::vector<rectui_t> clipRegions;

        glClearColor(m_background_color.r, m_background_color.g, m_background_color.b, m_background_color.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, m_options.width, m_options.height);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_builder.Build().ForEachCommand([&](auto& spCommand)
        {
            if (spCommand->commandId == RenderCommandId::transform)
            {
                const RenderTransformCommand& command = *static_cast<RenderTransformCommand*>(spCommand.get());
                switch (command.type)
                {
                    case RenderTransformCommand::MultiplyPush:
                        transform = transform * command.transform;
                        transforms.push_back(transform);
                        break;
                    case RenderTransformCommand::Push:
                        transform = command.transform;
                        transforms.push_back(transform);
                        break;
                    case RenderTransformCommand::Pop:
                        transforms.pop_back();
                        transform = transforms.back();
                        break;
                }
            }
            else if (spCommand->commandId == RenderCommandId::clip)
            {
                const RenderClipCommand& command = *static_cast<RenderClipCommand*>(spCommand.get());
                switch (command.type)
                {
                    case RenderClipCommand::Push:
                        if (clipRegions.empty())
                            glEnable(GL_SCISSOR_TEST);
                        clipRegions.push_back(command.clipRegion);
                        break;
                    case RenderClipCommand::Pop:
                        clipRegions.pop_back();
                        if (clipRegions.empty())
                            glDisable(GL_SCISSOR_TEST);
                        break;
                }
            }
            else
            {
                const RenderDrawCommand& command = *static_cast<RenderDrawCommand*>(spCommand.get());
                glUseProgram(m_shader.id());
                // for vertex shader
                xpf::Shader::set_uniform(u_projection, m_projection_matrix);
                xpf::Shader::set_uniform(u_view_matrix, m4_t::identity);
                xpf::Shader::set_uniform(u_transform, transform);

                // for frag shader
                xpf::Shader::set_uniform(u_command_id, int32_t(command.commandId));

                if (spCommand->commandId == RenderCommandId::rounded_rectangle ||
                    spCommand->commandId == RenderCommandId::rounded_rectangle_with_border ||
                    spCommand->commandId == RenderCommandId::rounded_rectangle_with_border_dots)
                {
                    const RenderRoundedRectangleCommand& cmd = *static_cast<RenderRoundedRectangleCommand*>(spCommand.get());
                    xpf::Shader::set_uniform(u_size, cmd.size);
                    xpf::Shader::set_uniform(u_corner_radius, cmd.cornerRadius.v);
                    xpf::Shader::set_uniform(u_border_thickness, cmd.borderThickness.get_v4());
                    xpf::Shader::set_uniform(u_border_color, cmd.borderColor);
                }

                if (command.spTexture != nullptr) {
                    const ITexture* pTexture = command.spTexture.get();
                    if (pCurrentTexture != pTexture) {
                        pCurrentTexture = pTexture;
                        glActiveTexture(GL_TEXTURE0 + 0);
                        glBindTexture(GL_TEXTURE_2D, pTexture->GetId());
                        m_renderStats.textureSwitches++;

                        switch(pCurrentTexture->GetInterpolation())
                        {
                            case ITexture::Interpolation::None:
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                                break;
                            case ITexture::Interpolation::Linear:
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                                break;
                        }
                    }
                }

                uint32_t vertex_size = 8;

                if (m_vao == 0)
                    glGenVertexArrays(1, &m_vao);

                glBindVertexArray(m_vao);

                if (m_vbo == 0) {
                    glGenBuffers(1, &m_vbo);
                }

                glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
                glBufferData(GL_ARRAY_BUFFER, command.verts.size() * sizeof(command.verts[0]), &command.verts[0], GL_DYNAMIC_DRAW);

                // pos vec2
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float), nullptr);

                // color vec4
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float), (void*)(2 * sizeof(float)));

                // texture coords vec2
                glEnableVertexAttribArray(2);
                glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, vertex_size * sizeof(float), (void*)(6 * sizeof(float)));

                if (!clipRegions.empty())
                {
                    auto clipRect = clipRegions.back();
                    glScissor(clipRect.x, m_options.height - clipRect.y - clipRect.h, clipRect.w, clipRect.h);
                }

                glBindVertexArray(m_vao);
                glDrawArrays(GL_TRIANGLES, 0, command.count);
                m_renderStats.vertexCount += command.count;
                m_renderStats.drawCount++;
            }
        });

        if (m_captureScreen_frameCount > 0)
        {
            static std::vector<byte_t> s_bytes;
            s_bytes.resize(m_captureScreen_region.width * m_captureScreen_region.height * 4);
            glReadPixels(
                    m_captureScreen_region.x,
                    m_options.height - m_captureScreen_region.y - m_captureScreen_region.height,
                    m_captureScreen_region.width,
                    m_captureScreen_region.height,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    s_bytes.data());
            Image img(s_bytes, m_captureScreen_region.w, m_captureScreen_region.h, xpf::PixelFormat::R8G8B8A8);
            img.VerticalFlip();

            m_captureScreen_frameCount--;
            m_captureScreen_callback(std::move(img));

            s_bytes.clear();
        }

        glfwSwapBuffers(m_pWindow);
    }

    virtual std::shared_ptr<ITexture> CreateTexture(const Image& img) override
    {
        return OpenGL_CreateTexture(img);
    }

    virtual std::shared_ptr<ITexture> CreateTexture(std::string_view filename) override
    {
        return CreateTexture(Image::LoadImage(filename));
    }

    virtual std::shared_ptr<IBuffer> CreateBuffer(const byte_t* pbyte, size_t size) override
    {
        return OpenGL_CreateBuffer(pbyte, size);
    }
protected:

    void LoadShader()
    {
        m_shader = xpf::Shader::load({
            {xpf::Shader::vertex, xpf::resources::opengl_shader_vert()},
            {xpf::Shader::fragment, xpf::resources::opengl_shader_frag()}});

        u_projection = m_shader.get_uniform_location("u_projection");
        u_view_matrix = m_shader.get_uniform_location("u_view_matrix");
        u_transform = m_shader.get_uniform_location("u_transform");

        u_command_id = m_shader.get_uniform_location("u_command_id");
        u_corner_radius = m_shader.get_uniform_location("u_corner_radius");
        u_border_thickness = m_shader.get_uniform_location("u_border_thickness");
        u_border_color = m_shader.get_uniform_location("u_border_color");
        u_size = m_shader.get_uniform_location("u_size");
    }
};

std::unique_ptr<IRenderer> create_opengl_renderer()
{
    return std::make_unique<OpenGLRenderer>();
}

#if defined(PLATFORM_APPLE)
std::unique_ptr<IRenderer> create_directx_renderer()
{
    return std::make_unique<OpenGLRenderer>();
}
#elif defined(PLATFORM_WINDOWS)
std::unique_ptr<IRenderer> create_metal_renderer()
{
    return std::make_unique<OpenGLRenderer>();
}
#endif

} // xpf