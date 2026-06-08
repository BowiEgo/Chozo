#pragma once

#include <Core/EntityRegistry/EntityRegistry.hpp>
#include <Core/FileSystem/VFS.hpp>
#include <Core/Header/Assert.hpp>
#include <Core/JobSystem/JobSystem.h>
#include <Core/Log/LogMacros.hpp>
#include <Runtime/RenderCore/Asset.hpp>

#include <future>

namespace CZ {

template <typename T> struct ResourceGeneratorTraits {
    template <typename... Args> static Scope<T> Generate(Args&&... args) = delete;
};

template <typename T> struct ResourceLoaderTraits {
    static Scope<T> Load(const std::string& virtualPath) = delete;
};

template <typename T> struct ResourceStoragePolicy {
    T* Allocate(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_CacheMutex);

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
    std::mutex m_CacheMutex;
};

template <typename T> class AssetRegistry : public EntityRegistry<T, ResourceStoragePolicy<T>> {
public:
    using EntityRegistry<T, ResourceStoragePolicy<T>>::EntityRegistry;
    using AssetClass = typename AssetTraits<T>::AssetClass;

    void Init() override;
    void Shutdown() override;

    template <typename... Args> AssetClass GenerateAsset(Args&&... args) {
        Scope<T> obj = ResourceGeneratorTraits<T>::Generate(std::forward<Args>(args)...);
        T* ptr       = obj.release();

        AssetClass asset(ptr);
        AssetHandle handle = AssetHandle::Generate();
        asset.SetHandle(handle);

        {
            std::lock_guard<std::mutex> lock(m_CacheMutex);
            m_MemoryCache[handle] = asset;
        }

        return asset;
    }

    template <typename... Args> AssetClass CreateMemoryAssetInstance(Args&&... args) {
        auto obj = ResourceLoaderTraits<T>::Create(std::forward<Args>(args)...);
        T* ptr   = obj.get();

        AssetClass asset(ptr);

        AssetHandle handle = AssetHandle::Generate();
        asset.SetHandle(handle);
        m_MemoryCache[handle] = asset;

        return asset;
    }

    AssetClass LoadAsset(const std::string& path) {
        T* ptr = this->Allocate(path);

        std::lock_guard<std::mutex> lock(m_CacheMutex);

        for (auto& [handle, asset] : m_DiskCache) {
            if (asset.EqualObj(ptr)) return asset;
        }

        AssetClass asset(ptr);

        AssetHandle handle = AssetHandle::Generate();
        asset.SetHandle(handle);
        m_DiskCache[handle] = asset;

        return asset;
    }

    std::future<AssetClass> LoadAssetAsync(const std::string& path) {
        struct TaskData {
            std::string Path;
            AssetRegistry* Registry;
            std::promise<AssetClass> Promise;
        };

        auto* rawData     = new TaskData;
        rawData->Path     = path;
        rawData->Registry = this;
        auto future       = rawData->Promise.get_future();

        JobHeader job{};
        job.OnExecute = [](void* user) {
            auto* data       = static_cast<TaskData*>(user);
            AssetClass asset = data->Registry->LoadAsset(data->Path);
            data->Promise.set_value(std::move(asset));
        };
        job.OnComplete = [](void* user) { delete static_cast<TaskData*>(user); };
        job.User       = rawData;
        job.Type       = 0;

        JobSystem::Get().Submit(&job, JOB_DISPATCH_STANDARD);

        return future;
    }

    AssetClass GetAsset(AssetHandle handle) {
        std::lock_guard<std::mutex> lock(m_CacheMutex);

        auto it = m_DiskCache.find(handle);
        if (it != m_DiskCache.end()) return it->second;

        auto memIt = m_MemoryCache.find(handle);
        if (memIt != m_MemoryCache.end()) return memIt->second;

        return AssetClass();
    }

    void Clear() {
        for (auto& [_, asset] : m_DiskCache) {
            asset.Destroy();
        }
        m_DiskCache.clear();
        this->ResourceStoragePolicy<T>::Shutdown();
    }

private:
    std::unordered_map<AssetHandle, AssetClass> m_DiskCache;
    std::unordered_map<AssetHandle, AssetClass> m_MemoryCache;
    std::mutex m_CacheMutex;
};

} // namespace CZ