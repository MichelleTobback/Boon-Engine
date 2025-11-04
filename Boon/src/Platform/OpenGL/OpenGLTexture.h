#include "Renderer/Texture.h"

#include <glad/glad.h>

namespace Boon 
{

	class OpenGLTexture2D final : public Texture2D
	{
	public:
		OpenGLTexture2D(const TextureDescriptor& descriptor);
		virtual ~OpenGLTexture2D();

		inline virtual const TextureDescriptor& GetDescriptor() const override { return m_Descriptor; }

		inline virtual uint32_t GetWidth() const override { return m_Descriptor.Width; }
		inline virtual uint32_t GetHeight() const override { return m_Descriptor.Height; }
		inline virtual uint32_t GetRendererID() const override { return m_RendererID; }

		virtual void SetData(void* data, uint32_t size) override;

		virtual void Bind(uint32_t slot = 0) const override;

		virtual bool operator==(const Texture& other) const override
		{
			return m_RendererID == other.GetRendererID();
		}
	private:
		TextureDescriptor m_Descriptor;

		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
}