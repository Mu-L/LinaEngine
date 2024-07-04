/*
This file is a part of: Lina Engine
https://github.com/inanevin/LinaEngine

Author: Inan Evin
http://www.inanevin.com

Copyright (c) [2018-] [Inan Evin]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Core/Graphics/Resource/Texture.hpp"
#include "Common/Math/Math.hpp"
#include "Core/Resources/ResourceManager.hpp"
#include "Common/System/System.hpp"
#include "Core/Graphics/GfxManager.hpp"

namespace Lina
{
	void Texture::Metadata::SaveToStream(OStream& out) const
	{
		const uint8 formatInt	   = static_cast<uint8>(format);
		const uint8 mipmapModeInt  = static_cast<uint8>(mipmapMode);
		const uint8 mipFilterInt   = static_cast<uint8>(mipFilter);
		const uint8 channelMaskInt = static_cast<uint8>(channelMask);
		out << formatInt << mipmapModeInt << mipFilterInt << channelMaskInt;
		out << samplerSID << isLinear << generateMipmaps;
	}

	void Texture::Metadata::LoadFromStream(IStream& in)
	{
		uint8 formatInt = 0, mipmapModeInt = 0, mipFilterInt = 0, channelMaskInt = 0;
		in >> formatInt >> mipmapModeInt >> mipFilterInt >> channelMaskInt;
		format		= static_cast<LinaGX::Format>(formatInt);
		mipmapMode	= static_cast<LinaGX::MipmapMode>(mipmapModeInt);
		mipFilter	= static_cast<LinaGX::MipmapFilter>(mipFilterInt);
		channelMask = static_cast<LinaGX::ImageChannelMask>(channelMaskInt);
		in >> samplerSID >> isLinear >> generateMipmaps;
	}

	Texture::Texture(System* system, const String& path, StringID sid) : Resource(system, path, sid, GetTypeID<Texture>())
	{
		m_gfxManager = m_system->CastSubsystem<GfxManager>(SubsystemType::GfxManager);
	};

	Texture::~Texture()
	{
		for (const auto& b : m_allLevels)
			LINA_ASSERT(b.pixels == nullptr, "Texture buffers are still filled, are you trying to delete mid-transfer?");

		if (!m_gpuHandleExists)
			return;
		GfxManager::GetLGX()->DestroyTexture(m_gpuHandle);
		m_gpuHandleExists = false;
	}

	uint32 Texture::GetSamplerSID() const
	{
		if (m_sampler == 0)
			return m_meta.samplerSID;
		return m_sampler;
	}

	void Texture::DestroySW()
	{
		for (auto& b : m_allLevels)
		{
			if (m_useGlobalDelete)
				delete[] b.pixels;
			else
				LinaGX::FreeImage(b.pixels);

			b.pixels = nullptr;
		}
		m_allLevels.clear();
	}

	void Texture::LoadFromBuffer(uint8* pixels, uint32 width, uint32 height, uint32 bytesPerPixel, LinaGX::ImageChannelMask channelMask, LinaGX::Format format, bool generateMipMaps)
	{
		DestroySW();
		m_size = Vector2ui(width, height);

		LinaGX::TextureBuffer level0 = {
			.width		   = width,
			.height		   = height,
			.bytesPerPixel = bytesPerPixel,
		};
		m_meta.channelMask = channelMask;
		m_bytesPerPixel	   = bytesPerPixel;

		const size_t sz = width * height * bytesPerPixel;
		level0.pixels	= new uint8[sz];
		MEMCPY(level0.pixels, pixels, sz);

		m_allLevels.push_back(level0);

		if (generateMipMaps)
		{
			LINAGX_VEC<LinaGX::TextureBuffer> mipData;
			LinaGX::GenerateMipmaps(m_allLevels[0], mipData, m_meta.mipFilter, m_meta.channelMask, m_meta.isLinear);
			for (const auto& mp : mipData)
				m_allLevels.push_back(mp);
		}

		m_useGlobalDelete = true;
		m_meta.format	  = format;
	}

	void Texture::LoadFromFile(const char* path)
	{
		DestroySW();
		LinaGX::TextureBuffer outBuffer = {};
		LinaGX::LoadImageFromFile(path, outBuffer, m_meta.channelMask);
		m_bytesPerPixel = outBuffer.bytesPerPixel;
		LINA_ASSERT(outBuffer.pixels != nullptr, "Failed loading texture! {0}", path);

		m_size.x = outBuffer.width;
		m_size.y = outBuffer.height;

		m_allLevels.push_back(outBuffer);

		if (m_meta.generateMipmaps)
		{
			LINAGX_VEC<LinaGX::TextureBuffer> mipData;
			LinaGX::GenerateMipmaps(outBuffer, mipData, m_meta.mipFilter, m_meta.channelMask, m_meta.isLinear);

			for (const auto& mp : mipData)
				m_allLevels.push_back(mp);
		}
	}

	void Texture::LoadFromStream(IStream& stream)
	{
		DestroySW();
		m_meta.LoadFromStream(stream);

		stream >> m_bytesPerPixel;

		uint32 allLevels = 0;
		stream >> allLevels;
		m_useGlobalDelete = true;

		for (uint32 i = 0; i < allLevels; i++)
		{
			uint32				  pixelSize = 0;
			LinaGX::TextureBuffer buffer;
			stream >> buffer.width >> buffer.height >> pixelSize;

			if (i == 0)
			{
				m_size.x = buffer.width;
				m_size.y = buffer.height;
			}

			if (pixelSize != 0)
			{
				buffer.pixels = new uint8[pixelSize];
				stream.ReadEndianSafe(buffer.pixels, pixelSize);
			}

			buffer.bytesPerPixel = m_bytesPerPixel;

			m_allLevels.push_back(buffer);
		}
	}

	void Texture::SaveToStream(OStream& stream) const
	{
		m_meta.SaveToStream(stream);
		stream << m_bytesPerPixel;

		const uint32 allLevels = static_cast<uint32>(m_allLevels.size());
		stream << allLevels;

		for (const auto& buffer : m_allLevels)
		{
			const uint32 pixelSize = buffer.width * buffer.height * m_bytesPerPixel;
			stream << buffer.width << buffer.height << pixelSize;

			if (pixelSize != 0)
				stream.WriteEndianSafe(buffer.pixels, pixelSize);
		}
	}

	Vector2ui Texture::GetSize()
	{
		return m_size;
	}

	Vector2 Texture::GetSizeF()
	{
		return Vector2(static_cast<float>(m_size.x), static_cast<float>(m_size.y));
	}

	void Texture::GenerateHWFromDesc(const LinaGX::TextureDesc& desc)
	{
		LINA_ASSERT(m_gpuHandle == false, "");

		m_gpuHandle		  = GfxManager::GetLGX()->CreateTexture(desc);
		m_gpuHandleExists = true;
		m_size			  = Vector2ui(desc.width, desc.height);
	}

	void Texture::GenerateHW()
	{
		LINA_ASSERT(m_gpuHandle == false, "");

		LinaGX::TextureDesc desc = LinaGX::TextureDesc{
			.format	   = m_meta.format,
			.flags	   = LinaGX::TextureFlags::TF_Sampled | LinaGX::TextureFlags::TF_CopyDest,
			.width	   = m_allLevels[0].width,
			.height	   = m_allLevels[0].height,
			.mipLevels = static_cast<uint32>(m_allLevels.size()),
			.debugName = m_path.c_str(),
		};
		m_gpuHandle		  = GfxManager::GetLGX()->CreateTexture(desc);
		m_gpuHandleExists = true;
	}

	void Texture::BatchLoaded()
	{
		AddToUploadQueue();
	}

	void Texture::AddToUploadQueue()
	{
		if (!m_gpuHandleExists)
			GenerateHW();
		m_gfxManager->GetResourceUploadQueue().AddTextureRequest(this, [this]() { DestroySW(); });
	}

} // namespace Lina
