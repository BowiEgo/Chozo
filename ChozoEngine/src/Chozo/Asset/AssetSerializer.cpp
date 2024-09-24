#include "AssetSerializer.h"

#include "Chozo/Project/Project.h"
#include "Chozo/Renderer/Texture.h"

namespace Chozo {

    //////////////////////////////////////////////////////////////////////////////////
	// TextureSerializer
	//////////////////////////////////////////////////////////////////////////////////

    bool TextureSerializer::TryLoadData(const AssetMetadata &metadata, Ref<Asset> &asset) const
    {
		// asset = Texture2D::Create(Project::GetAssetManager()->GetFileSystemPathString(metadata));
		asset->Handle = metadata.Handle;

		// bool result = asset.As<Texture2D>()->Loaded();

        return true;
    }
}
