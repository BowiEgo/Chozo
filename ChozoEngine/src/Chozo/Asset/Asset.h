#pragma once

#include "Chozo/Core/UUID.h"
#include "AssetType.h"

#include "czpch.h"

namespace Chozo {

	using AssetHandle = UUID;

	class Asset : public RefCounted
	{
	public:
		AssetHandle Handle = 0;

		virtual ~Asset() {}

		virtual AssetType GetAssetType() const { return AssetType::None; }

		virtual bool operator==(const Asset& other) const
		{
			return Handle == other.Handle;
		}
		
		virtual bool operator!=(const Asset& other) const
		{
			return !(*this == other);
		}
	};

    struct AssetMetadata
	{
		AssetHandle Handle = 0;
		AssetType Type;

		fs::path FilePath;
		uint64_t FileSize;
		bool IsDataLoaded = false;
		bool IsMemoryAsset = false;

		bool IsValid() const { return Handle != 0 && !IsMemoryAsset; }
	};

	struct Texture2DMetadata
	{
		uint16_t Format;

		uint32_t Samples;
		uint32_t Width, Height;
        uint8_t Mipmap;
		uint16_t WrapR;
		uint16_t WrapS;
		uint16_t WrapT;
		uint16_t MinFilter;
		uint16_t MagFilter;
	};
}
