#pragma once

#include <memory>

namespace Boon
{
	class UniformBuffer
	{
	public:
		UniformBuffer() = default;
		virtual ~UniformBuffer() = default;

		UniformBuffer(const UniformBuffer& other) = delete;
		UniformBuffer(UniformBuffer&& other) = delete;
		UniformBuffer& operator=(const UniformBuffer& other) = delete;
		UniformBuffer& operator=(UniformBuffer&& other) = delete;

		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		template <typename T>
		void SetValue(const T& data, uint32_t offset = 0)
		{
			SetData(&data, sizeof(T), offset);
		}

		static std::shared_ptr<UniformBuffer> Create(uint32_t size, uint32_t binding);

		template <typename T>
		static std::shared_ptr<UniformBuffer> Create(uint32_t binding)
		{
			return UniformBuffer::Create(sizeof(T), binding);
		}
	};
}