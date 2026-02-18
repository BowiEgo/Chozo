#pragma once

#include "Layer.h"

class CORE_API ILayerStack {
public:
    ILayerStack();
    ~ILayerStack();

    void PushLayer(ILayer* layer);
    void PushOverlay(ILayer* overlay);
    void PopLayer(const ILayer* layer);
    void PopOverlay(const ILayer* overlay);
    void Clear() {
        for (ILayer* layer : m_Layers) {
            layer->OnDetach();
            delete layer;
        }
        m_Layers.clear();
        m_LayerInsertIndex = 0;
    }

    std::vector<ILayer*>::iterator begin() { return m_Layers.begin(); }
    std::vector<ILayer*>::iterator end() { return m_Layers.end(); }

private:
    std::vector<ILayer*> m_Layers;
    unsigned int m_LayerInsertIndex = 0;
};