#pragma once

#include "EditorContext.hpp"


class PropertiesPanel {
private:
    EditorContext &context;

    bool ImGuiMaterialEditBool(std::string label, std::string id, Material &mat);

    float ImGuiMaterialEditFloat(std::string label, std::string id, Material &mat, float min, float max);

    vec3 ImGuiMaterialEditVec3(std::string label, std::string id, Material &mat);

    void ImGuiMaterialEditTexture(std::string label, std::string id, Material &mat, unsigned int unit);

    void renderMaterial(Material &mat);

public:

    PropertiesPanel(EditorContext &context);

    void renderContents();
};