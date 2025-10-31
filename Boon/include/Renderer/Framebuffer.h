#pragma once

#include <vector>
#include <memory>

namespace Boon 
{
	enum class FramebufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	struct FramebufferTextureDescriptor
	{
		FramebufferTextureDescriptor() = default;
		FramebufferTextureDescriptor(FramebufferTextureFormat format)
			: TextureFormat(format) {}

		FramebufferTextureFormat TextureFormat = FramebufferTextureFormat::None;
	};

	struct FramebufferAttachmentDescriptor
	{
		FramebufferAttachmentDescriptor() = default;
		FramebufferAttachmentDescriptor(std::initializer_list<FramebufferTextureDescriptor> attachments)
			: Attachments(attachments) {}

		std::vector<FramebufferTextureDescriptor> Attachments;
	};

	struct FramebufferDescriptor
	{
		uint32_t Width = 0, Height = 0;
		FramebufferAttachmentDescriptor Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class Framebuffer
	{
	public:
		virtual ~Framebuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual const FramebufferDescriptor& GetDescriptor() const = 0;

		static std::shared_ptr<Framebuffer> Create(const FramebufferDescriptor& spec);
	};


}