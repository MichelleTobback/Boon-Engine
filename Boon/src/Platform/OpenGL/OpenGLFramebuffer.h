#include "Renderer/Framebuffer.h"

#include <vector>

namespace Boon
{
	class OpenGLFramebuffer : public Framebuffer
	{
	public:
		OpenGLFramebuffer(const FramebufferDescriptor& desc);
		virtual ~OpenGLFramebuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) override;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) override;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const override { return m_ColorAttachments[index]; }

		inline virtual const FramebufferDescriptor& GetDescriptor() const override { return m_Desc; }

	private:
		FramebufferDescriptor m_Desc;
		uint32_t m_ID = 0;

		std::vector<FramebufferTextureDescriptor> m_ColorAttachmentDescriptors;
		FramebufferTextureDescriptor m_DepthAttachmentDescriptor{ FramebufferTextureFormat::None };

		std::vector<uint32_t> m_ColorAttachments;
		uint32_t m_DepthAttachment{};
	};
}