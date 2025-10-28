#include "Renderer/VertexBufferLayout.h"

using namespace Boon;

Boon::VertexBufferLayout::Element::Element(ShaderDataType type, const std::string& name, bool normalized)
	: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
{
}

uint32_t Boon::VertexBufferLayout::Element::GetComponentCount() const
{
	switch (Type)
	{
	case ShaderDataType::Float:   return 1;
	case ShaderDataType::Float2:  return 2;
	case ShaderDataType::Float3:  return 3;
	case ShaderDataType::Float4:  return 4;
	case ShaderDataType::Mat3:    return 3; // 3* float3
	case ShaderDataType::Mat4:    return 4; // 4* float4
	case ShaderDataType::Int:     return 1;
	case ShaderDataType::Int2:    return 2;
	case ShaderDataType::Int3:    return 3;
	case ShaderDataType::Int4:    return 4;
	case ShaderDataType::Bool:    return 1;
	}
	return 0;
}

Boon::VertexBufferLayout::VertexBufferLayout(std::initializer_list<Element> elements)
	: m_Elements(elements)
{
	CalculateOffsetsAndStride();
}

void Boon::VertexBufferLayout::CalculateOffsetsAndStride()
{
	size_t offset{};
	m_Stride = 0;
	for (auto& element : m_Elements)
	{
		element.Offset = offset;
		offset += element.Size;
		m_Stride += element.Size;
	}
}
