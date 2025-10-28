#include "OpenGLVertexInput.h"

#include <glad/glad.h>

using namespace Boon;

static GLenum ShaderDataTypeToOpenGLBaseType(ShaderDataType type)
{
	switch (type)
	{
	case ShaderDataType::Float:    return GL_FLOAT;
	case ShaderDataType::Float2:   return GL_FLOAT;
	case ShaderDataType::Float3:   return GL_FLOAT;
	case ShaderDataType::Float4:   return GL_FLOAT;
	case ShaderDataType::Mat3:     return GL_FLOAT;
	case ShaderDataType::Mat4:     return GL_FLOAT;
	case ShaderDataType::Int:      return GL_INT;
	case ShaderDataType::Int2:     return GL_INT;
	case ShaderDataType::Int3:     return GL_INT;
	case ShaderDataType::Int4:     return GL_INT;
	case ShaderDataType::Bool:     return GL_BOOL;
	}
	return 0;
}

Boon::OpenGLVertexInput::OpenGLVertexInput()
{
	glCreateVertexArrays(1, &m_ID);
}

Boon::OpenGLVertexInput::~OpenGLVertexInput()
{
	glDeleteVertexArrays(1, &m_ID);
}

void Boon::OpenGLVertexInput::Bind() const
{
	glBindVertexArray(m_ID);
}

void Boon::OpenGLVertexInput::Unbind() const
{
	glBindVertexArray(0);
}

void Boon::OpenGLVertexInput::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& pVertexBuffer)
{
	glBindVertexArray(m_ID);
	pVertexBuffer->Bind();

	const auto& layout = pVertexBuffer->GetLayout();
	for (const auto& element : layout)
	{
		switch (element.Type)
		{
		case ShaderDataType::Float:
		case ShaderDataType::Float2:
		case ShaderDataType::Float3:
		case ShaderDataType::Float4:
		{
			glEnableVertexAttribArray(m_VertexBufferIndex);
			glVertexAttribPointer(m_VertexBufferIndex,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				element.Normalized ? GL_TRUE : GL_FALSE,
				layout.GetStride(),
				(const void*)element.Offset);
			m_VertexBufferIndex++;
			break;
		}
		case ShaderDataType::Int:
		case ShaderDataType::Int2:
		case ShaderDataType::Int3:
		case ShaderDataType::Int4:
		case ShaderDataType::Bool:
		{
			glEnableVertexAttribArray(m_VertexBufferIndex);
			glVertexAttribIPointer(m_VertexBufferIndex,
				element.GetComponentCount(),
				ShaderDataTypeToOpenGLBaseType(element.Type),
				layout.GetStride(),
				(const void*)element.Offset);
			m_VertexBufferIndex++;
			break;
		}
		case ShaderDataType::Mat3:
		case ShaderDataType::Mat4:
		{
			uint8_t count = element.GetComponentCount();
			for (uint8_t i = 0; i < count; i++)
			{
				glEnableVertexAttribArray(m_VertexBufferIndex);
				glVertexAttribPointer(m_VertexBufferIndex,
					count,
					ShaderDataTypeToOpenGLBaseType(element.Type),
					element.Normalized ? GL_TRUE : GL_FALSE,
					layout.GetStride(),
					(const void*)(element.Offset + sizeof(float) * count * i));
				glVertexAttribDivisor(m_VertexBufferIndex, 1);
				m_VertexBufferIndex++;
			}
			break;
		}
		}
	}

	m_pVertexBuffers.push_back(pVertexBuffer);
}

void Boon::OpenGLVertexInput::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& pIndexBuffer)
{
	glBindVertexArray(m_ID);
	pIndexBuffer->Bind();

	m_pIndexBuffer = pIndexBuffer;
}
