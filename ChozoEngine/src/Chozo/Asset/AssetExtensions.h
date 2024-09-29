#pragma once

#include "AssetType.h"

namespace Chozo {

    inline static std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
	{
		// Textures
		{ ".png", AssetType::Texture },
		{ ".jpg", AssetType::Texture },
		{ ".jpeg", AssetType::Texture },
	};
}