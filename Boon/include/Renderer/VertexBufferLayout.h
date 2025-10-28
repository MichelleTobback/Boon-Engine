#pragma once

#include <initializer_list>
#include <string>
#include <vector>

namespace Boon
{
	enum class ShaderDataType
	{
		None = 0, Float, Float2, Float3, Float4, Mat3, Mat4, Int, Int2, Int3, Int4, Bool
	};

	static uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case ShaderDataType::Float:    return 4;
		case ShaderDataType::Float2:   return 4 * 2;
		case ShaderDataType::Float3:   return 4 * 3;
		case ShaderDataType::Float4:   return 4 * 4;
		case ShaderDataType::Mat3:     return 4 * 3 * 3;
		case ShaderDataType::Mat4:     return 4 * 4 * 4;
		case ShaderDataType::Int:      return 4;
		case ShaderDataType::Int2:     return 4 * 2;
		case ShaderDataType::Int3:     return 4 * 3;
		case ShaderDataType::Int4:     return 4 * 4;
		case ShaderDataType::Bool:     return 1;
		}
		return 0;
	}

	class VertexBufferLayout final
	{
	public:
		struct Element
		{
			std::string Name;
			ShaderDataType Type;
			uint32_t Size;
			size_t Offset;
			bool Normalized;

			Element() = default;
			Element(ShaderDataType type, const std::string& name, bool normalized = false);

			uint32_t GetComponentCount() const;
		};

		VertexBufferLayout() = default;
		VertexBufferLayout(std::initializer_list<Element> elements);

		virtual ~VertexBufferLayout() = default;

		VertexBufferLayout(const VertexBufferLayout& other) = default;
		VertexBufferLayout(VertexBufferLayout&& other) = default;
		VertexBufferLayout& operator=(const VertexBufferLayout& other) = default;
		VertexBufferLayout& operator=(VertexBufferLayout&& other) = default;

		inline uint32_t GetStride() const { return m_Stride; }
		inline const std::vector<Element>& GetElements() const { return m_Elements; }

		inline std::vector<Element>::iterator begin() { return m_Elements.begin(); }
		inline std::vector<Element>::iterator end() { return m_Elements.end(); }
		inline std::vector<Element>::const_iterator begin() const { return m_Elements.begin(); }
		inline std::vector<Element>::const_iterator end() const { return m_Elements.end(); }

	private:
		void CalculateOffsetsAndStride();

		std::vector<Element> m_Elements;
		uint32_t m_Stride = 0;
	};
}