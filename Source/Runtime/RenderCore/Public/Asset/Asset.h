#pragma once

#include "Ref.h"
#include "UUID.h"

using FAssetHandle = FUUID;

// class FAssetHandle {
// public:
//     FAssetHandle() : m_UUID(UUID::Invalid()) {}
//     explicit FAssetHandle(const UUID& uuid) : m_UUID(uuid) {}
//     explicit FAssetHandle(uint64_t low, uint64_t high) : m_UUID(low, high) {}

//     static FAssetHandle FromUInt64(uint64_t value) { return FAssetHandle(value, 0); }

//     bool IsValid() const { return m_UUID.IsValid(); }
//     const UUID& GetUUID() const { return m_UUID; }

//     bool operator==(const FAssetHandle& other) const { return m_UUID == other.m_UUID; }
//     bool operator!=(const FAssetHandle& other) const { return m_UUID != other.m_UUID; }

//     std::string ToString() const { return m_UUID.ToString(); }

//     static FAssetHandle Invalid() { return FAssetHandle(UUID::Invalid()); }
//     static FAssetHandle Generate() { return FAssetHandle(UUID::Generate()); }

// private:
//     UUID m_UUID;
// };

// namespace std {
// template <> struct hash<FAssetHandle> {
//     size_t operator()(const FAssetHandle& handle) const noexcept {
//         return hash<uint64_t>()(handle.GetUUID());
//     }
// };
// } // namespace std

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