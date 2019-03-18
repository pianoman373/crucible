#pragma once

#include <string>
#include <crucible/Material.hpp>
#include <crucible/Math.hpp>

class PrefabView;

class PropertiesPanel {
private:
    PrefabView &view;

    bool ImGuiMaterialEditBool(std::string label, std::string id, Material &mat);

    float ImGuiMaterialEditFloat(std::string label, std::string id, Material &mat, float min, float max);

    vec3 ImGuiMaterialEditVec3(std::string label, std::string id, Material &mat);

    void ImGuiMaterialEditTexture(std::string label, std::string id, Material &mat, unsigned int unit);

    void renderMaterial(Material &mat);

public:

    PropertiesPanel(PrefabView &view);

    void renderContents();
};