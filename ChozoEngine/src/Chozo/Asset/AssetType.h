#pragma once

#include "czpch.h"

namespace Chozo {

	enum class AssetType : uint16_t
	{
		None = 0,
		Scene,
		Prefab,
		Mesh,
		StaticMesh,
		MeshSource,
		Material,
		Texture,
		EnvMap,
		Audio,
		SoundConfig,
		SpatializationConfig,
		Font,
		Script,
		ScriptFile,
		MeshCollider,
		SoundGraphSound,
		Skeleton,
		Animation,
		AnimationGraph
	};
}