#pragma once
#include "VertexBufferLayout.h"

#include <memory>

namespace Boon
{
	class VertexBuffer
	{
	public:
		VertexBuffer() = default;
		virtual ~VertexBuffer() = default;

		VertexBuffer(const VertexBuffer& other) = delete;
		VertexBuffer(VertexBuffer&& other) = delete;
		VertexBuffer& operator=(const VertexBuffer& other) = delete;
		VertexBuffer& operator=(VertexBuffer&& other) = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void SetData(const void* data, uint32_t size) = 0;

		virtual const VertexBufferLayout& GetLayout() const = 0;
		virtual void SetLayout(const VertexBufferLayout& layout) = 0;

		static std::shared_ptr<VertexBuffer> Create(float* vertices, uint32_t size);
		static std::shared_ptr<VertexBuffer> Create(uint32_t size);
	};
}