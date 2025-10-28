#include "OpenGLVertexBuffer.h"

#include <glad/glad.h>

using namespace Boon;

Boon::OpenGLVertexBuffer::OpenGLVertexBuffer(float* vertices, uint32_t size)
{
	Init(vertices, size);
}

Boon::OpenGLVertexBuffer::OpenGLVertexBuffer(uint32_t size)
{
	Init(nullptr, size);
}

Boon::OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
	glDeleteBuffers(1, &m_ID);
}

void Boon::OpenGLVertexBuffer::Bind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
}

void Boon::OpenGLVertexBuffer::Unbind() const
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Boon::OpenGLVertexBuffer::SetData(const void* data, uint32_t size)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
}

void Boon::OpenGLVertexBuffer::Init(float* vertices, uint32_t size)
{
	glCreateBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, vertices ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
}
