#pragma once
#include "Core/Memory/Buffer.h"
#include <string>
#include <memory>

namespace Boon 
{

	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F
	};

	enum class ImageFilter
	{
		Nearest,
		Linear
	};

	struct TextureDescriptor
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		ImageFilter MinFilter = ImageFilter::Nearest;
		ImageFilter MagFilter = ImageFilter::Nearest;
		bool GenerateMips = false;
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		/**
		 * @brief Get the descriptor describing the texture properties.
		 *
		 * @return Const reference to the TextureDescriptor used when creating the texture.
		 */
		virtual const TextureDescriptor& GetDescriptor() const = 0;

		/**
		 * @brief Get the texture width in pixels.
		 */
		virtual uint32_t GetWidth() const = 0;

		/**
		 * @brief Get the texture height in pixels.
		 */
		virtual uint32_t GetHeight() const = 0;

		/**
		 * @brief Get the renderer-specific identifier for the texture.
		 *
		 * The meaning of the id is renderer-dependent.
		 */
		virtual uint32_t GetRendererID() const = 0;

		/**
		 * @brief Upload raw pixel data to the texture.
		 *
		 * @param data Pointer to the pixel data.
		 * @param size Size in bytes of the data buffer.
		 */
		virtual void SetData(void* data, uint32_t size) = 0;

		/**
		 * @brief Upload pixel data from a Buffer abstraction.
		 *
		 * @param buffer Buffer containing pixel data.
		 */
		virtual void SetData(Buffer& buffer) = 0;

		/**
		 * @brief Bind the texture to the given slot for rendering.
		 *
		 * @param slot Texture unit index to bind to.
		 */
		virtual void Bind(uint32_t slot = 0) const = 0;

		/**
		 * @brief Compare textures for equality.
		 *
		 * The exact equality semantics are renderer-defined.
		 */
		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static std::shared_ptr<Texture2D> Create(const TextureDescriptor& descriptor);
	};

}