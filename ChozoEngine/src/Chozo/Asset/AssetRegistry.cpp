#include "AssetRegistry.h"

namespace Chozo {

	static std::mutex s_AssetRegistryMutex;

    AssetMetadata &AssetRegistry::operator[](const AssetHandle handle)
    {
        std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);

		return m_AssetRegistry[handle];
    }

    bool AssetRegistry::Contains(const AssetHandle handle) const
    {
		std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);

		return m_AssetRegistry.find(handle) != m_AssetRegistry.end();
    }

    size_t AssetRegistry::Remove(const AssetHandle handle)
    {
		std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);

		m_AssetRegistry.erase(handle);

        return Count();
    }

    void AssetRegistry::Clear()
	{
    	std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);
    	
		m_AssetRegistry.clear();
    }
}
