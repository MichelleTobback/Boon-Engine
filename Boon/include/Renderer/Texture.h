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

		virtual const TextureDescriptor& GetDescriptor() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;
		virtual void SetData(Buffer& buffer) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture
	{
	public:
		static std::shared_ptr<Texture2D> Create(const TextureDescriptor& descriptor);
	};

}