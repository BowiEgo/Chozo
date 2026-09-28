#pragma once
#include <vector>

#include <cstdint>

#include <Core/Math/Matrix4.hpp>
#include <Core/Math/Quaternion.hpp>
#include <Core/Math/Vector3.hpp>
#include <Runtime/RenderCore/Scene/Entity.hpp>

namespace CZ {

struct SceneObj;

class TransformSystem {
public:
    TransformSystem(SceneObj* scene) : m_SceneObj(scene) {}

    void Update();
    void MarkDirty(Entity entity);

private:
    void ComputeDepth(Entity entity, uint32_t depth);

    std::vector<Entity> GetUpdateOrder();

    void UpdateEntity(Entity entity, uint32_t recursionDepth = 0);

    SceneObj* m_SceneObj;
    std::vector<Entity> m_DirtySet;
    bool m_bNeedSort = true;
};

} // namespace CZ
