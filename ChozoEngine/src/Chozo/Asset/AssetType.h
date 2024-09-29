#pragma once

#include "czpch.h"

namespace Chozo {

	enum class AssetType : uint16_t
	{
		None = 0,
		Texture,
	};

	namespace Utils {

		inline AssetType AssetTypeFromString(std::string_view assetType)
		{
			if (assetType == "None")                return AssetType::None;
			if (assetType == "Texture")             return AssetType::Texture;

			return AssetType::None;
		}

		inline const char* AssetTypeToString(AssetType assetType)
		{
			switch (assetType)
			{
				case AssetType::None:                return "None";
				case AssetType::Texture:             return "Texture";
			}

			CZ_CORE_ASSERT(false, "Unknown Asset Type");
			return "None";
		}
	}
}