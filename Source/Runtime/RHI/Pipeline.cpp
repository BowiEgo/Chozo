#include <Runtime/RHI/Pipeline.hpp>

namespace CZ {

template <> void Handle<PipelineObj>::Destroy() {
    if (m_Obj) {
        Delete(m_Obj);
        m_Obj = nullptr;
    }
}

} // namespace CZ