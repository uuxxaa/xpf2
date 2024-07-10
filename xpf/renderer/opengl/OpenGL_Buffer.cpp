#include <renderer/IBuffer.h>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

namespace xpf {

class OpenGLBuffer : public IBuffer
{
protected:
    uint32_t         m_id = 0;          // buffer id - managed by OpenGl
    size_t           m_size = 0;       // size in bytes;

public:
    OpenGLBuffer(uint32_t id, size_t size)
        : m_id(id)
        , m_size(size)
    {}

    ~OpenGLBuffer()
    {
        if (m_id != 0)
            glDeleteBuffers(1, &m_id);
    }

    virtual uint32_t GetId() const override { return m_id; }
    virtual size_t GetSize() const override { return m_size; }
};

std::shared_ptr<IBuffer> OpenGL_CreateBuffer(const byte_t* pbyte, size_t size)
{
    uint32_t id = 0;
    glGenBuffers(1, &id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, id);
    glBufferData(GL_SHADER_STORAGE_BUFFER, size, pbyte, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // unbind
    return std::make_shared<OpenGLBuffer>(id, size);
}
} // xpf
