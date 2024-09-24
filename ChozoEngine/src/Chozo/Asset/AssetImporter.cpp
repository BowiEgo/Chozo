#include "AssetImporter.h"

namespace Chozo
{

    bool AssetImporter::TryLoadData(const AssetMetadata &metadata, Ref<Asset> &asset)
    {
        return false;
        // if (s_Serializers.find(metadata.Type) == s_Serializers.end())
        // {
		// 	CZ_CORE_WARN("There's currently no importer for assets of type {0}", metadata.FilePath.stem().string());
        //     return false;
        // }

        // return s_Serializers[metadata.Type]->TryLoadData(metadata, asset);
    }
}
