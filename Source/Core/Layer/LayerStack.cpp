#include <Core/Layer/LayerStack.hpp>

#include <Core/Log/LogMacros.hpp>

namespace CZ {

LayerStack::LayerStack() = default;

LayerStack::~LayerStack() {}

void LayerStack::PushLayer(Layer* layer) {
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
    m_LayerInsertIndex++;
    layer->OnAttach();
}

void LayerStack::PopLayer(Layer* layer) {
    if (auto it = std::find(m_Layers.begin(), m_Layers.end(), layer); it != m_Layers.end()) {
        layer->OnDetach();
        Delete(layer);

        m_Layers.erase(it);
        if (m_LayerInsertIndex > 0) m_LayerInsertIndex--;
    }
}

void LayerStack::Clear() {
    while (!m_Layers.empty()) {
        Layer* layer = m_Layers.back();
        layer->OnDetach();
        Delete(layer);
        m_Layers.pop_back();
    }
    m_Layers.clear();
    m_LayerInsertIndex = 0;
}

} // namespace CZ