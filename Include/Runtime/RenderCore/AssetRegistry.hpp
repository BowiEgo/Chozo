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

/// OWNS the assets loaded from disk (a `Scope<T>` per unique path) and de-duplicates by path.
template <typename T> struct ResourceStoragePolicy {
    T* Allocate(const std::string& path) {
        std::lock_guard<std::mutex> lock(m_CacheMutex);

        auto it = m_Cache.find(path);
        if (it != m_Cache.end()) return it->second.get();

        Scope<T> obj = ResourceLoaderTraits<T>::Load(path);
        if (!obj) return nullptr;

        T* ptr        = obj.get();
        m_Cache[path] = std::move(obj);
        return ptr;
    }

    T* Get(T* ptr) { return ptr; }

    /// Releases every asset owned by this policy.
    void Shutdown() {
        std::lock_guard<std::mutex> lock(m_CacheMutex);
        m_Cache.clear();
    }

    auto begin() { return m_Cache.begin(); }
    auto end() { return m_Cache.end(); }

private:
    std::unordered_map<std::string, Scope<T>> m_Cache;
    std::mutex m_CacheMutex;
};

/**
 * Asset registry.
 *
 * The registry is the single owner of every asset it hands out: generated assets live in
 * `m_Owned`, file assets are owned by `ResourceStoragePolicy`. Callers only ever hold
 * non-owning `AssetClass` views, so `Clear()` is the only place assets are released.
 */
template <typename T> class AssetRegistry : public EntityRegistry<T, ResourceStoragePolicy<T>> {
public:
    using EntityRegistry<T, ResourceStoragePolicy<T>>::EntityRegistry;
    using AssetClass = typename AssetTraits<T>::AssetClass;

    void Init() override;
    void Shutdown() override;

    template <typename... Args> AssetClass GenerateAsset(Args&&... args) {
        Scope<T> obj = ResourceGeneratorTraits<T>::Generate(std::forward<Args>(args)...);
        if (!obj) return AssetClass();

        const AssetHandle handle = AssetHandle::Generate();
        T* ptr                   = obj.get();

        {
            std::lock_guard<std::mutex> lock(m_CacheMutex);
            m_Owned[handle] = std::move(obj);
            m_Views[handle] = ptr;
        }

        return MakeView(handle, ptr);
    }

    AssetClass LoadAsset(const std::string& path) {
        {
            std::lock_guard<std::mutex> lock(m_CacheMutex);
            if (auto it = m_PathToHandle.find(path); it != m_PathToHandle.end()) {
                return MakeView(it->second);
            }
        }

        T* ptr = this->Allocate(path); // owned by the storage policy
        if (!ptr) return AssetClass();

        const AssetHandle handle = AssetHandle::Generate();

        {
            std::lock_guard<std::mutex> lock(m_CacheMutex);
            m_PathToHandle[path] = handle;
            m_Views[handle]      = ptr;
        }

        return MakeView(handle, ptr);
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

        auto it = m_Views.find(handle);
        if (it == m_Views.end()) return AssetClass();

        return MakeView(handle, it->second);
    }

    /// Releases every asset owned by this registry.
    void Clear() {
        {
            std::lock_guard<std::mutex> lock(m_CacheMutex);

            m_Owned.clear(); // frees generated assets
            m_Views.clear();
            m_PathToHandle.clear();
        }

        ResourceStoragePolicy<T>::Shutdown(); // frees file assets
    }

private:
    AssetClass MakeView(AssetHandle handle) {
        auto it = m_Views.find(handle);
        return it != m_Views.end() ? MakeView(handle, it->second) : AssetClass();
    }

    static AssetClass MakeView(AssetHandle handle, T* ptr) {
        if (!ptr) return AssetClass();

        AssetClass asset(ptr);
        asset.SetHandle(handle);
        return asset;
    }

    std::unordered_map<AssetHandle, Scope<T>> m_Owned;
    std::unordered_map<AssetHandle, T*> m_Views;
    std::unordered_map<std::string, AssetHandle> m_PathToHandle;
    std::mutex m_CacheMutex;
};

} // namespace CZ
