#include "OpenGLTexture.h"

using namespace Boon;

namespace Utils 
{

	static GLenum ImageFormatToGLDataFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB8:  return GL_RGB;
		case ImageFormat::RGBA8: return GL_RGBA;
		}

		return 0;
	}

	static GLenum ImageFormatToGLInternalFormat(ImageFormat format)
	{
		switch (format)
		{
		case ImageFormat::RGB8:  return GL_RGB8;
		case ImageFormat::RGBA8: return GL_RGBA8;
		}

		return 0;
	}

}

Boon::OpenGLTexture2D::OpenGLTexture2D(const TextureDescriptor& descriptor)
	: m_Descriptor{descriptor}
{
	m_InternalFormat = Utils::ImageFormatToGLInternalFormat(m_Descriptor.Format);
	m_DataFormat = Utils::ImageFormatToGLDataFormat(m_Descriptor.Format);

	glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
	glTextureStorage2D(m_RendererID, 1, m_InternalFormat, descriptor.Width, descriptor.Height);

	glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

Boon::OpenGLTexture2D::~OpenGLTexture2D()
{
	glDeleteTextures(1, &m_RendererID);
}

void Boon::OpenGLTexture2D::SetData(void* data, uint32_t size)
{
	uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
	glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Descriptor.Width, m_Descriptor.Height, m_DataFormat, GL_UNSIGNED_BYTE, data);
}

void Boon::OpenGLTexture2D::Bind(uint32_t slot) const
{
	glBindTextureUnit(slot, m_RendererID);
}
