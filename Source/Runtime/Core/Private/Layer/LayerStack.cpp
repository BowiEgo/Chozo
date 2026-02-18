#include "LayerStack.h"

ILayerStack::ILayerStack() = default;

ILayerStack::~ILayerStack() {}

void ILayerStack::PushLayer(ILayer* layer) {
    m_Layers.emplace(m_Layers.begin() + m_LayerInsertIndex, layer);
    m_LayerInsertIndex++;
}

void ILayerStack::PushOverlay(ILayer* overlay) { m_Layers.emplace_back(overlay); }

void ILayerStack::PopLayer(const ILayer* layer) {
    if (const auto it = std::find(m_Layers.begin(), m_Layers.end(), layer); it != m_Layers.end()) {
        m_Layers.erase(it);
        m_LayerInsertIndex--;
    }
}

void ILayerStack::PopOverlay(const ILayer* overlay) {
    if (const auto it = std::find(m_Layers.begin(), m_Layers.end(), overlay);
        it != m_Layers.end()) {
        m_Layers.erase(it);
    }
}