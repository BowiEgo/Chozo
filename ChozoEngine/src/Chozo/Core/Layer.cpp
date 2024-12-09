#include "Layer.h"

namespace Chozo
{
    Layer::Layer(std::string  debugName)
        : m_DebugName(std::move(debugName))
    {
    }

    Layer::~Layer() = default;
}
