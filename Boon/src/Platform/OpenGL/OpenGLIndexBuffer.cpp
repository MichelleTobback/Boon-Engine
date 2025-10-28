#include "OpenGLIndexBuffer.h"

#include <glad/glad.h>

using namespace Boon;

Boon::OpenGLIndexBuffer::OpenGLIndexBuffer(uint32_t* indices, uint32_t count)
	: m_Count{count}
{
	Init(indices, count);
}

Boon::OpenGLIndexBuffer::~OpenGLIndexBuffer()
{
	glDeleteBuffers(1, &m_ID);
}

void Boon::OpenGLIndexBuffer::Bind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ID);
}

void Boon::OpenGLIndexBuffer::Unbind() const
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Boon::OpenGLIndexBuffer::Init(uint32_t* indices, uint32_t count)
{
	glCreateBuffers(1, &m_ID);
	glBindBuffer(GL_ARRAY_BUFFER, m_ID);
	glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
}
