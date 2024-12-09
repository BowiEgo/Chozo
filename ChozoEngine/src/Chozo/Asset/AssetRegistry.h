#pragma once

#include "Asset.h"

#include "czpch.h"

namespace Chozo {

    class AssetRegistry
	{
	public:
		AssetMetadata& operator[](AssetHandle handle);
		// AssetMetadata& Get(const AssetHandle handle);
		// const AssetMetadata& Get(const AssetHandle handle) const;

		bool Empty() const {return m_AssetRegistry.empty(); } // NOLINT
		size_t Count() const { return m_AssetRegistry.size(); } // NOLINT
		bool Contains(AssetHandle handle) const; // NOLINT
		size_t Remove(AssetHandle handle);
		// void Clear();

		auto begin() { return m_AssetRegistry.begin(); }
		auto end() { return m_AssetRegistry.end(); }
		auto begin() const { return m_AssetRegistry.cbegin(); } // NOLINT
		auto end() const { return m_AssetRegistry.cend(); } // NOLINT
	private:
		std::unordered_map<AssetHandle, AssetMetadata> m_AssetRegistry;
	};
}