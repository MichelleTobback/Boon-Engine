#pragma once
#include "VertexBufferLayout.h"

#include <memory>

namespace Boon
{
	class VertexBuffer
	{
	public:
		VertexBuffer() = default;

		/**
		 * @brief Virtual destructor for vertex buffer implementations.
		 */
		virtual ~VertexBuffer() = default;

		VertexBuffer(const VertexBuffer& other) = delete;
		VertexBuffer(VertexBuffer&& other) = delete;
		VertexBuffer& operator=(const VertexBuffer& other) = delete;
		VertexBuffer& operator=(VertexBuffer&& other) = delete;

		/**
		 * @brief Bind the vertex buffer for use.
		 */
		virtual void Bind() const = 0;

		/**
		 * @brief Unbind the vertex buffer.
		 */
		virtual void Unbind() const = 0;

		/**
		 * @brief Upload data into the buffer.
		 *
		 * @param data Pointer to source data.
		 * @param size Size in bytes of the data.
		 */
		virtual void SetData(const void* data, uint32_t size) = 0;

		/**
		 * @brief Get the current vertex buffer layout.
		 *
		 * @return Const reference to the layout object.
		 */
		virtual const VertexBufferLayout& GetLayout() const = 0;

		/**
		 * @brief Set the vertex buffer layout describing vertex attributes.
		 *
		 * @param layout Layout to associate with this buffer.
		 */
		virtual void SetLayout(const VertexBufferLayout& layout) = 0;

		/**
		 * @brief Create a vertex buffer initialized with vertex data.
		 */
		static std::shared_ptr<VertexBuffer> Create(float* vertices, uint32_t size);

		/**
		 * @brief Create an empty vertex buffer of given size.
		 */
		static std::shared_ptr<VertexBuffer> Create(uint32_t size);
	};
}