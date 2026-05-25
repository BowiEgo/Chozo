#pragma once

#include <Core/EntityRegistry/EntityRegistry.hpp>
#include <Core/FileSystem/VFS.hpp>
#include <Core/Header/Assert.hpp>
#include <Core/Log/LogMacros.hpp>
#include <Runtime/RenderCore/Asset.hpp>

namespace CZ {

template <typename T> struct ResourceLoaderTraits {
    static Scope<T> Load(const std::string& virtualPath) {
        CZ_CORE_ASSERT(false, "ResourceLoaderTraits not specialized for this type");
    }
};

template <typename T> struct ResourceStoragePolicy {
    T* Allocate(const std::string& path) {
        auto it = m_Cache.find(path);
        if (it != m_Cache.end()) return it->second.get();

        auto obj      = ResourceLoaderTraits<T>::Load(path);
        T* ptr        = obj.get();
        m_Cache[path] = std::move(obj);
        return ptr;
    }

    void Deallocate(T* ptr) {}

    void Shutdown() { m_Cache.clear(); }

    T* Get(T* ptr) { return ptr; }

    auto begin() { return m_Cache.begin(); }
    auto end() { return m_Cache.end(); }

private:
    std::unordered_map<std::string, Scope<T>> m_Cache;
};

template <typename T> class AssetRegistry : public EntityRegistry<T, ResourceStoragePolicy<T>> {
public:
    using EntityRegistry<T, ResourceStoragePolicy<T>>::EntityRegistry;
    using AssetType = typename AssetTraits<T>::AssetType;

    AssetType LoadAsset(const std::string& path) {
        T* ptr = this->Allocate(path);
        AssetType asset(ptr);

        AssetHandle handle = AssetHandle::Generate();
        asset.SetHandle(handle);
        m_Cache[handle] = asset;

        return asset;
    }

    void Clear() {
        for (auto& [_, asset] : m_Cache) {
            asset.Destroy();
        }
        m_Cache.clear();
        this->ResourceStoragePolicy<T>::Shutdown();
    }

private:
    std::unordered_map<AssetHandle, AssetType> m_Cache;
};

} // namespace CZ