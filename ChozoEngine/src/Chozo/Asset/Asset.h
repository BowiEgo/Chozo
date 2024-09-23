#pragma once

#include "Chozo/Core/UUID.h"

namespace Chozo {

	using AssetHandle = UUID;

	class Asset
	{
	public:
		AssetHandle Handle = 0;

		virtual ~Asset() {}

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

		std::filesystem::path FilePath;
	};
}
