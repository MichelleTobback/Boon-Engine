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

		/**
		 * @brief Bind the framebuffer for rendering.
		 */
		virtual void Bind() = 0;

		/**
		 * @brief Unbind the framebuffer.
		 */
		virtual void Unbind() = 0;

		/**
		 * @brief Resize the framebuffer and its attachments.
		 *
		 * @param width New width in pixels.
		 * @param height New height in pixels.
		 */
		virtual void Resize(uint32_t width, uint32_t height) = 0;

		/**
		 * @brief Read a pixel value from a color attachment.
		 *
		 * @param attachmentIndex Index of the attachment to read from.
		 * @param x X coordinate in pixels.
		 * @param y Y coordinate in pixels.
		 * @return Integer value read from the attachment. Interpretation is format-dependent.
		 */
		virtual int ReadPixel(uint32_t attachmentIndex, int x, int y) = 0;

		/**
		 * @brief Clear an attachment to a given integer value.
		 *
		 * @param attachmentIndex Index of the attachment to clear.
		 * @param value Value to write into the attachment.
		 */
		virtual void ClearAttachment(uint32_t attachmentIndex, int value) = 0;

		/**
		 * @brief Get the renderer-specific id for a color attachment texture.
		 *
		 * @param index Attachment index (default 0).
		 * @return Renderer id for the attachment texture.
		 */
		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		/**
		 * @brief Get the descriptor used to create this framebuffer.
		 *
		 * @return Const reference to the FramebufferDescriptor.
		 */
		virtual const FramebufferDescriptor& GetDescriptor() const = 0;

		/**
		 * @brief Create a framebuffer matching the provided descriptor.
		 *
		 * @param spec Framebuffer creation parameters.
		 * @return Shared pointer to the created Framebuffer.
		 */
		static std::shared_ptr<Framebuffer> Create(const FramebufferDescriptor& spec);
	};


}