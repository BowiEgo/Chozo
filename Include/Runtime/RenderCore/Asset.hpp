#pragma once

#include <Core/Header/Handle.hpp>
#include <Core/Header/UUID.hpp>
#include <Runtime/RHI/RHITypes.hpp>

namespace CZ {

using AssetHandle = UUID;

enum class AssetType { Unknown, Scene, Texture, Mesh, Shader, Material };

template <typename T> struct AssetTraits;

template <typename TObject> class Asset : public Handle<TObject> {
    template <typename T> T* As() { return static_cast<T*>(InternalHandleReader::Unwrap(*this)); }

public:
    // Asset() : m_Handle(AssetHandle::Invalid()) {}
    explicit Asset(TObject* ptr) : Handle<TObject>(ptr) {}
    // explicit Asset(const AssetHandle& handle) : m_Handle(handle) {}
    Asset()  = default;
    ~Asset() = default;

    virtual bool operator==(const Asset& other) const { return m_Handle == other.m_Handle; }
    virtual bool operator!=(const Asset& other) const { return !(*this == other); }

    virtual AssetType GetType() const         = 0;
    virtual const std::string GetName() const = 0;

    const AssetHandle& GetHandle() const { return m_Handle; }
    void SetHandle(const AssetHandle& handle) { m_Handle = handle; }

protected:
    AssetHandle m_Handle;
    AssetType m_Type = AssetType::Unknown;
};

} // namespace CZ
