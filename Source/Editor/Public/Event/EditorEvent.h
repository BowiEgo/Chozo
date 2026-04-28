#pragma once

#include "Event.h"
#include "Material.h"

class FEditorEvent : public IEvent {
public:
    EVENT_CLASS_CATEGORY(EventCategory_Editor)
protected:
    explicit FEditorEvent() {}
};

class FOpenMaterialPanelEvent : public FEditorEvent {
public:
    FOpenMaterialPanelEvent(const FAssetHandle handle) : m_MaterialHandle(handle) {}

    FAssetHandle GetMaterialHandle() const { return m_MaterialHandle; }

    std::string ToString() const override {
        std::stringstream ss;
        ss << "OpenMaterialPanelEvent: " << m_MaterialHandle.ToString();
        return ss.str();
    }

    EVENT_CLASS_TYPE(OpenMaterialPanel);

private:
    FAssetHandle m_MaterialHandle;
};