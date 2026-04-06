#pragma once
#include "VertexBuffer.h"
#include "IndexBuffer.h"

#include <memory>

namespace Boon
{
	class VertexInput
	{
	public:
		VertexInput() = default;

		/**
		 * @brief Virtual destructor for vertex input implementations.
		 */
		virtual ~VertexInput() = default;

		VertexInput(const VertexInput& other) = delete;
		VertexInput(VertexInput&& other) = delete;
		VertexInput& operator=(const VertexInput& other) = delete;
		VertexInput& operator=(VertexInput&& other) = delete;

		/**
		 * @brief Bind the vertex input for rendering.
		 */
		virtual void Bind() const = 0;

		/**
		 * @brief Unbind the vertex input.
		 */
		virtual void Unbind() const = 0;

		/**
		 * @brief Attach a vertex buffer to the input.
		 */
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& pVertexBuffer) = 0;

		/**
		 * @brief Set the index buffer used by this vertex input.
		 */
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& pVertexBuffer) = 0;

		/**
		 * @brief Get the index buffer associated with this input.
		 */
		virtual std::shared_ptr<IndexBuffer> GetIndexBuffer() = 0;

		static std::shared_ptr<VertexInput> Create();
	};
}