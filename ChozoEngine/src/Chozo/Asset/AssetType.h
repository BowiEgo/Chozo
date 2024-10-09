#pragma once

#include "czpch.h"

namespace Chozo {

	#define FOREACH_ASSET_TYPE(TYPE) \
        	TYPE(Texture) \
        	TYPE(Material) \

	enum class AssetType : uint16_t {
		#define GENERATE_ENUM(ENUM) ENUM,
		FOREACH_ASSET_TYPE(GENERATE_ENUM)
		#undef GENERATE_ENUM
		None
	};

	namespace Utils {

		inline AssetType StringToAssetType(std::string_view assetType) {
			#define GENERATE_IF(ENUM) if (assetType == #ENUM) return AssetType::ENUM;
			FOREACH_ASSET_TYPE(GENERATE_IF)
			#undef GENERATE_IF
			return AssetType::None;
		}

		inline const char* AssetTypeToString(AssetType assetType) {
			switch (assetType) {
				#define GENERATE_CASE(ENUM) case AssetType::ENUM: return #ENUM;
				FOREACH_ASSET_TYPE(GENERATE_CASE)
				#undef GENERATE_CASE
				default: return "Unknown";
			}
		}
	}
}