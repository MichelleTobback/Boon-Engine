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
		virtual ~VertexInput() = default;

		VertexInput(const VertexInput& other) = delete;
		VertexInput(VertexInput&& other) = delete;
		VertexInput& operator=(const VertexInput& other) = delete;
		VertexInput& operator=(VertexInput&& other) = delete;

		virtual void Bind() const = 0;
		virtual void Unbind() const = 0;
		virtual void AddVertexBuffer(const std::shared_ptr<VertexBuffer>& pVertexBuffer) = 0;
		virtual void SetIndexBuffer(const std::shared_ptr<IndexBuffer>& pVertexBuffer) = 0;

		static std::shared_ptr<VertexInput> Create();
	};
}