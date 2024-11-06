#pragma once

#include "czpch.h"
#include "Chozo/Utilities/StringUtils.h"

namespace Chozo {

	#define FOREACH_ASSET_TYPE(TYPE) \
        	TYPE(Scene) \
        	TYPE(Texture) \
        	TYPE(Material) \
        	TYPE(MeshSource) \

	enum class AssetType : uint16_t {
		#define GENERATE_ENUM(ENUM) ENUM,
		FOREACH_ASSET_TYPE(GENERATE_ENUM)
		#undef GENERATE_ENUM
		None
	};

	inline static std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
	{
		// Chozo types
		{ ".scene", AssetType::Scene },
		{ ".material", AssetType::Material },

		// mesh/animation source
		{ ".fbx", AssetType::MeshSource },
		{ ".gltf", AssetType::MeshSource },
		{ ".glb", AssetType::MeshSource },
		{ ".obj", AssetType::MeshSource },

		// Textures
		{ ".png", AssetType::Texture },
		{ ".jpg", AssetType::Texture },
		{ ".jpeg", AssetType::Texture },
		// { ".hdr", AssetType::EnvMap },
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

		inline AssetType GetAssetTypeFromExtension(const std::string& extension)
		{
			std::string ext = Utils::String::ToLowerCopy(extension);
			if (s_AssetExtensionMap.find(ext) == s_AssetExtensionMap.end())
				return AssetType::None;

			return s_AssetExtensionMap.at(ext.c_str());
		}
	}
}