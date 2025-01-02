#pragma once

#include <utility>

#include "Chozo/Core/UUID.h"
#include "Chozo/Core/Observer.h"
#include "Chozo/Utilities/TimeUtils.h"
#include "AssetType.h"

namespace Chozo {

	using AssetHandle = UUID;

	class Asset : public RefCounted
	{
	public:
		using UpdatedCb = std::function<void()>;

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

		void OnUpdated(const UpdatedCb& cb)
		{
			m_UpdatedCbs.push_back(cb);
		}

		void HandleUpdated() const {
			for (auto& cb : m_UpdatedCbs)
				cb();
		}
	private:
		std::vector<UpdatedCb> m_UpdatedCbs;
	};

    struct AssetMetadata
	{
		AssetHandle Handle = 0;
		AssetType Type = AssetType::None;

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
