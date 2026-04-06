#pragma once

#include <memory>

namespace Boon
{
	class IndexBuffer
	{
	public:
		IndexBuffer() = default;

		/**
		 * @brief Virtual destructor for index buffer implementations.
		 */
		virtual ~IndexBuffer() = default;

		IndexBuffer(const IndexBuffer& other) = delete;
		IndexBuffer(IndexBuffer&& other) = delete;
		IndexBuffer& operator=(const IndexBuffer& other) = delete;
		IndexBuffer& operator=(IndexBuffer&& other) = delete;

		/**
		 * @brief Bind the index buffer.
		 */
		virtual void Bind() const = 0;

		/**
		 * @brief Unbind the index buffer.
		 */
		virtual void Unbind() const = 0;

		/**
		 * @brief Get the number of indices stored in the buffer.
		 *
		 * @return Index count.
		 */
		virtual uint32_t GetCount() const = 0;

		/**
		 * @brief Create an index buffer from raw index data.
		 *
		 * @param indices Pointer to index data.
		 * @param count Number of indices.
		 * @return Shared pointer to the created IndexBuffer.
		 */
		static std::shared_ptr<IndexBuffer> Create(uint32_t* indices, uint32_t count);
	};
}