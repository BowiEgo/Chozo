#include "Layer.h"

ILayer::ILayer(std::string debugName) : m_DebugName(std::move(debugName)) {}

ILayer::~ILayer() = default;
