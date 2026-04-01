#pragma once

#include "Entity.h"
#include "Matrix4.h"
#include "Quaternion.h"
#include "Vector3.h"

class FScene;

class FTransformSystem {
public:
    FTransformSystem(class FScene* scene);

    void Update();
    void MarkDirty(FEntity entity);
    // void SetParent(FEntity child, FEntity parent);
    // void SetLocalTransform(FEntity entity, const FVector3& pos, const FQuaternion& rot,
    //                        const FVector3& scale);

private:
    void ComputeDepth(FEntity entity, uint32_t depth);

    // 拓扑排序：按深度升序排列需要更新的实体
    std::vector<FEntity> GetUpdateOrder();

    void UpdateEntity(FEntity entity);

    FScene* m_Scene;
    std::vector<FEntity> m_DirtySet; // 需要更新的实体列表（可选）
    bool m_bNeedSort = true;
};