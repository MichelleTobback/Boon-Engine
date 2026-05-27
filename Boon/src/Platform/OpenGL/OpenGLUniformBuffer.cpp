#include "OpenGLUniformBuffer.h"
#include "BoonDebug/Logger.h"
#include <glad/glad.h>

using namespace Boon;

Boon::OpenGLUniformBuffer::OpenGLUniformBuffer(size_t size, uint32_t binding)
    : m_Size(size)
    , m_Binding(binding)
{
    glCreateBuffers(1, &m_ID);
    glNamedBufferData(m_ID, m_Size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, m_Binding, m_ID);
}

Boon::OpenGLUniformBuffer::~OpenGLUniformBuffer()
{
	glDeleteBuffers(1, &m_ID);
}

void Boon::OpenGLUniformBuffer::SetData(const void* data, size_t size, uint32_t offset)
{
    if (!data || size == 0)
        return;

    if (offset + size > m_Size)
    {
        BOON_LOG_ERROR("UniformBuffer SetData out of bounds. Offset: {}, Size: {}, BufferSize: {}", offset, size, m_Size);

        return;
    }

    glNamedBufferSubData(m_ID, offset, size, data);
}
