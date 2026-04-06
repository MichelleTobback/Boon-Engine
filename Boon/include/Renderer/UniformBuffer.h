#pragma once

#include <memory>

namespace Boon
{
	class UniformBuffer
	{
	public:
		UniformBuffer() = default;

		/**
		 * @brief Virtual destructor for uniform buffer implementations.
		 */
		virtual ~UniformBuffer() = default;

		UniformBuffer(const UniformBuffer& other) = delete;
		UniformBuffer(UniformBuffer&& other) = delete;
		UniformBuffer& operator=(const UniformBuffer& other) = delete;
		UniformBuffer& operator=(UniformBuffer&& other) = delete;

		/**
		 * @brief Set a block of data in the uniform buffer.
		 *
		 * @param data Pointer to the source data.
		 * @param size Number of bytes to copy.
		 * @param offset Byte offset within the buffer to write to.
		 */
		virtual void SetData(const void* data, uint32_t size, uint32_t offset = 0) = 0;

		template <typename T>
		void SetValue(const T& data, uint32_t offset = 0)
		{
			SetData(&data, sizeof(T), offset);
		}

		/**
		 * @brief Create a uniform buffer of given size bound to the specified binding point.
		 *
		 * @param size Size in bytes of the buffer to create.
		 * @param binding Binding index for shader access.
		 * @return Shared pointer to the created UniformBuffer.
		 */
		static std::shared_ptr<UniformBuffer> Create(uint32_t size, uint32_t binding);

		template <typename T>
		static std::shared_ptr<UniformBuffer> Create(uint32_t binding)
		{
			return UniformBuffer::Create(sizeof(T), binding);
		}
	};
}