#pragma once

#include "Ref.h"
#include "UUID.h"

using FAssetHandle = FUUID;

enum class EAssetType { Unknown, Scene, Texture, Mesh, Shader, Material };

class IAsset : public FRefCounted {
public:
    IAsset() : m_Handle(FAssetHandle::Invalid()) {}
    explicit IAsset(const FAssetHandle& handle) : m_Handle(handle) {}
    ~IAsset() override = default;

    virtual bool operator==(const IAsset& other) const { return m_Handle == other.m_Handle; }
    virtual bool operator!=(const IAsset& other) const { return !(*this == other); }
    virtual const std::string GetName() const = 0;
    virtual const EAssetType GetType() const  = 0;

    const FAssetHandle& GetHandle() const { return m_Handle; }
    void SetHandle(const FAssetHandle& handle) { m_Handle = handle; }

protected:
    FAssetHandle m_Handle;
    EAssetType m_Type = EAssetType::Unknown;
};