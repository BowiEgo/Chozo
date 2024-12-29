#pragma once

#include <utility>

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

		void RegisterOnModifyCallback(const std::function<void()>& callback)
		{
			onModifyCallback = callback;
		}

		void HandleModified() const
		{
			if (onModifyCallback)
				onModifyCallback();
		}
	private:
		std::function<void()> onModifyCallback;
	};

    struct AssetMetadata
	{
		AssetHandle Handle = 0;
		AssetType Type;

		fs::path FilePath;
		uint64_t FileSize = 0;
    	bool IsDataLoaded = false;
		bool IsMemoryAsset = false;
    	bool IsFileMissing = false;

		uint64_t CreatedAt = Utils::Time::CreateTimestamp();
    	uint64_t ModifiedAt = Utils::Time::CreateTimestamp();
    	uint64_t LastModifiedAt = 0;

		bool IsValid() const { return Handle != 0 && !IsMemoryAsset; } // NOLINT
		bool IsModified() const { return ModifiedAt != LastModifiedAt; } // NOLINT
	};
}
