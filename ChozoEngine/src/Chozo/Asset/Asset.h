#pragma once

#include "Chozo/Core/UUID.h"
#include "Chozo/Utilities/TimeUtils.h"
#include "AssetType.h"

namespace Chozo {

	using AssetHandle = UUID;

	class Asset : public RefCounted
	{
	public:
		AssetHandle Handle = 0;

		~Asset() override = default;

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
		uint64_t FileSize = 0;
		bool IsDataLoaded = false;
		bool IsMemoryAsset = false;

		uint64_t CreateAt = Utils::Time::CreateTimestamp();
		uint64_t ModifiedAt = Utils::Time::CreateTimestamp();

		bool IsValid() const { return Handle != 0 && !IsMemoryAsset; } // NOLINT
	};
}
