#pragma once

#include "Ref.h"
#include "UUID.h"

using FAssetHandle = FUUID;

class IAsset : public FRefCounted {
public:
    IAsset() : m_Handle(FAssetHandle::Invalid()) {}
    explicit IAsset(const FAssetHandle& handle) : m_Handle(handle) {}
    ~IAsset() override = default;

    virtual bool operator==(const IAsset& other) const { return m_Handle == other.m_Handle; }
    virtual bool operator!=(const IAsset& other) const { return !(*this == other); }

    const FAssetHandle& GetHandle() const { return m_Handle; }
    void SetHandle(const FAssetHandle& handle) { m_Handle = handle; }

protected:
    FAssetHandle m_Handle;
};