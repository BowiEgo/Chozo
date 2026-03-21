#pragma once

#include "AssetExport.h"
#include "CoreMinimal.h"

#include "UUID.h"

class ASSET_API FAsset {
public:
    virtual ~FAsset() = default;

    UUID GetID() const { return m_ID; }
    void SetID(const UUID& id) { m_ID = id; }

    // virtual void Serialize(FArchive& ar) = 0;
    // virtual void Deserialize(FArchive& ar) = 0;

protected:
    UUID m_ID;
};