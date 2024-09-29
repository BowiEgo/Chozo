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

		std::filesystem::path FilePath;
		uint64_t FileSize;
		bool IsDataLoaded = false;
		bool IsMemoryAsset = false;

		bool IsValid() const { return Handle != 0 && !IsMemoryAsset; }
	};

	struct Texture2DMetadata
	{
		uint32_t Width, Height;
		uint16_t Format;
	};
}
