#pragma once

#include "Renderer/Pipeline.h"
#include "Renderer/Texture.h"
#include "Renderer/UniformBuffer.h"
#include "Renderer/ShaderCompiler/ShaderReflection.h"
#include "Core/Memory/Buffer.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <type_traits>
#include <cstring>
#include <utility>

namespace Boon
{
	struct MaterialTextureBinding
	{
		std::shared_ptr<Texture2D> Texture = nullptr;
		uint32_t Slot = 0;
	};

	class Material
	{
	public:
		explicit Material(std::shared_ptr<Pipeline> pipeline, const MaterialLayout& layout)
			: m_Pipeline(std::move(pipeline))
			, m_MaterialLayout(layout)
			, m_Data(layout.UniformBufferSize)
			, m_UniformBinding(layout.UniformBinding)
		{
			if (layout.UniformBufferSize > 0)
				m_UniformBuffer = UniformBuffer::Create(layout.UniformBufferSize, layout.UniformBinding);
		}

		void Bind()
		{
			if (!m_Pipeline)
				return;

			m_Pipeline->Bind();

			if (m_Dirty && m_UniformBuffer && m_Data.Size() > 0)
			{
				m_UniformBuffer->SetData(m_Data.Data(), m_Data.Size());
				m_Dirty = false;
			}

			for (auto& [name, binding] : m_Textures)
			{
				if (binding.Texture)
					binding.Texture->Bind(binding.Slot);
			}
		}

		void Unbind()
		{
			if (m_Pipeline)
				m_Pipeline->Unbind();
		}

		template<typename T>
		void SetValue(const std::string& name, const T& value)
		{
			const MaterialParameter* parameter = m_MaterialLayout.FindParameter(name);
			if (!parameter)
				return;

			SetValue(parameter->Offset, value);
		}

		template<typename T>
		void SetValue(size_t offset, const T& value)
		{
			static_assert(std::is_trivially_copyable_v<T>);

			EnsureDataSize(offset + sizeof(T));

			std::memcpy(m_Data.DataAt(offset), &value, sizeof(T));
			m_Dirty = true;
		}

		void SetRaw(size_t offset, const void* data, size_t size)
		{
			if (!data || size == 0)
				return;

			EnsureDataSize(offset + size);

			std::memcpy(m_Data.DataAt(offset), data, size);
			m_Dirty = true;
		}

		void SetTexture(const std::string& name, std::shared_ptr<Texture2D> texture, uint32_t slot)
		{
			auto& binding = m_Textures[name];

			if (binding.Texture == texture && binding.Slot == slot)
				return;

			binding.Texture = std::move(texture);
			binding.Slot = slot;
		}

		std::shared_ptr<Texture2D> GetTexture(const std::string& name) const
		{
			auto it = m_Textures.find(name);
			if (it == m_Textures.end())
				return nullptr;

			return it->second.Texture;
		}

		template<typename T>
		const T* GetDataAs() const
		{
			if (m_Data.Size() < sizeof(T))
				return nullptr;

			return reinterpret_cast<const T*>(m_Data.Data());
		}

		std::shared_ptr<Material> CreateInstance() const
		{
			auto instance = std::make_shared<Material>(m_Pipeline, m_MaterialLayout);

			instance->m_Data = m_Data;
			instance->m_Dirty = true;
			instance->m_Textures = m_Textures;

			return instance;
		}

		std::shared_ptr<Pipeline> GetPipeline() const { return m_Pipeline; }

		const Buffer& GetData() const { return m_Data; }
		Buffer& GetData() { return m_Data; }

	private:
		void EnsureDataSize(size_t requiredSize)
		{
			if (requiredSize <= m_Data.Size())
				return;

			m_Data.Resize(requiredSize);

			m_UniformBuffer = UniformBuffer::Create(static_cast<uint32_t>(m_Data.Size()), m_UniformBinding);

			m_Dirty = true;
		}

		std::shared_ptr<Pipeline> m_Pipeline = nullptr;

		Buffer m_Data;
		MaterialLayout m_MaterialLayout;
		std::shared_ptr<UniformBuffer> m_UniformBuffer = nullptr;
		bool m_Dirty = true;
		uint32_t m_UniformBinding = 2;

		std::unordered_map<std::string, MaterialTextureBinding> m_Textures;
	};
}