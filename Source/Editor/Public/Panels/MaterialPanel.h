#pragma once

#include "Material.h"
#include "Panel.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMaterialPanel, Info);

class MaterialPanel : public Panel {
public:
    MaterialPanel() {}
    ~MaterialPanel() {}

    void SetMaterial(const TRef<CMaterial> mat) { m_Material = mat; }

    virtual void Draw(const char* title) override;

    bool DrawColumnProperties(const std::string& name, IParams* params);

private:
    TRef<CMaterial> m_Material;
};