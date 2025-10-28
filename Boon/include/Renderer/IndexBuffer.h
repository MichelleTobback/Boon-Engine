#pragma once

#include <memory>

namespace Boon
{
	class IndexBuffer
	{
	public:
		IndexBuffer() = default;
		virtual ~IndexBuffer() = default;

		IndexBuffer(const IndexBuffer& other) = delete;
		IndexBuffer(IndexBuffer&& other) = delete;
		IndexBuffer& operator=(const IndexBuffer& other) = delete;
		IndexBuffer& operator=(IndexBuffer&& other) = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual uint32_t GetCount() const = 0;

		static std::shared_ptr<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};
}