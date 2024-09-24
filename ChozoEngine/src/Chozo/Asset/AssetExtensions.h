#pragma once

#include "AssetType.h"

namespace Chozo {

    inline static std::unordered_map<std::string, AssetType> s_AssetExtensionMap =
	{
		// Chozo types
		{ ".czscene", AssetType::Scene },
		{ ".czmesh", AssetType::Mesh },
		{ ".czsmesh", AssetType::StaticMesh },
		{ ".czmaterial", AssetType::Material },
		{ ".czskel", AssetType::Skeleton },
		{ ".czanim", AssetType::Animation },
		{ ".czanimgraph", AssetType::AnimationGraph },
		{ ".czprefab", AssetType::Prefab },
		{ ".czsoundc", AssetType::SoundConfig },

		{ ".cs", AssetType::ScriptFile },

		// mesh/animation source
		{ ".fbx", AssetType::MeshSource },
		{ ".gltf", AssetType::MeshSource },
		{ ".glb", AssetType::MeshSource },
		{ ".obj", AssetType::MeshSource },

		// Textures
		{ ".png", AssetType::Texture },
		{ ".jpg", AssetType::Texture },
		{ ".jpeg", AssetType::Texture },
		{ ".hdr", AssetType::EnvMap },

		// Audio
		{ ".wav", AssetType::Audio },
		{ ".ogg", AssetType::Audio },

		// Fonts
		{ ".ttf", AssetType::Font },
		{ ".ttc", AssetType::Font },
		{ ".otf", AssetType::Font },
		
		// Mesh Collider
		{ ".hmc", AssetType::MeshCollider },

		// Graphs
		{ ".sound_graph", AssetType::SoundGraphSound }
	};
}