#include "PropertiesPanel.hpp"

#include <imgui.h>
#include <crucible/Path.hpp>
#include <tinyfiledialogs.h>

bool PropertiesPanel::ImGuiMaterialEditBool(std::string label, std::string id, Material &mat) {
    bool &val = mat.getUniformBool(id);

    ImGui::Checkbox(label.c_str(), &val);

    return val;
}

float PropertiesPanel::ImGuiMaterialEditFloat(std::string label, std::string id, Material &mat, float min, float max) {
    float &val = mat.getUniformFloat(id);
    ImGui::SliderFloat(label.c_str(), &val, min, max);

    return val;
}

vec3 PropertiesPanel::ImGuiMaterialEditVec3(std::string label, std::string id, Material &mat) {
    vec3 &val = mat.getUniformVec3(id);

    ImGui::ColorEdit3(label.c_str(), (float*)&val);

    return val;
}

void PropertiesPanel::ImGuiMaterialEditTexture(std::string label, std::string id, Material &mat, unsigned int unit) {
    Texture value;

    auto search = mat.getTextureUniforms().find(id);
    if(search != mat.getTextureUniforms().end()) {
        value = search->second.tex;
    }
    else {
        mat.setUniformTexture(id, value, unit);
    }

    ImGui::TextWrapped(value.getFilepath().c_str());
    ImGui::Image(ImTextureID((long long) value.getID()), ImVec2(210, 210), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    if (ImGui::Button(std::string("Browse " + label).c_str(), ImVec2(212, 20))) {

        char const *file = tinyfd_openFileDialog("Open Texture", "", 0, NULL, "Texture file", 0);

        if (file != NULL) {
            std::string sfile = std::string(file);
            Path::format(sfile);
            value.load(sfile.c_str());
        }

        mat.setUniformTexture(id, value, unit);
    }
}

void PropertiesPanel::renderMaterial(Material &mat) {
    char newText[256];
    strncpy(newText, mat.name.c_str(), sizeof(newText));
    ImGui::InputText("Object Name", newText, 256);
    mat.name = newText;

    ImGui::Separator();

    if (ImGuiMaterialEditBool("Albedo Textured", "albedoTextured", mat)) {
        ImGuiMaterialEditTexture("Albedo", "albedoTex", mat, 0);
    }
    else {
        ImGuiMaterialEditVec3("Albedo", "albedoColor", mat);
    }

    ImGui::Separator();

    ImGuiMaterialEditBool("Invert Roughness", "invertRoughness", mat);

    if (mat.getUniformBool("metallicTextured")) {
        ImGuiMaterialEditBool("Use Metallic Alpha", "roughnessMetallicAlpha", mat);
    }
    else {
        mat.setUniformBool("roughnessMetallicAlpha", false); // if metallicTextured gets turned off we need to turn this off as well
    }

    if (!mat.getUniformBool("roughnessMetallicAlpha")) { //if "use metallic alpha" is enabled we don't need these options
        if (ImGuiMaterialEditBool("Roughness Textured", "roughnessTextured", mat)) {
            ImGuiMaterialEditTexture("Roughness", "roughnessTex", mat, 1);
        }
        else {
            ImGuiMaterialEditFloat("Roughness", "roughnessColor", mat, 0.0f, 1.0f);
        }
    }


    ImGui::Separator();

    if (ImGuiMaterialEditBool("Metallic Textured", "metallicTextured", mat)) {
        ImGuiMaterialEditTexture("Metallic", "metallicTex", mat, 2);
    }
    else {
        ImGuiMaterialEditFloat("Metallic", "metallicColor", mat, 0.0f, 1.0f);
    }

    ImGui::Separator();

    if (ImGuiMaterialEditBool("Normal Textured", "normalTextured", mat)) {
        ImGuiMaterialEditTexture("Normals", "normalTex", mat, 3);
    }

    static vec2 size;
}


PropertiesPanel::PropertiesPanel(EditorContext &context): context(context) {

}

void PropertiesPanel::renderContents() {
    if (ImGui::Begin("Properties"))
    {
        if (context.selectedObject >= 0) {
            GameObject &obj = context.scene.getObject(context.selectedObject);

            char newText[256];
            strncpy(newText, obj.getName().c_str(), sizeof(newText));
            ImGui::InputText("Material Name", newText, 256);
            obj.setName(newText);

            ImGui::DragFloat3("Position", &obj.transform.position.x, 0.1f);
            ImGui::DragFloat3("Scale", &obj.transform.scale.x, 0.1f);

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Material"))
            {
                renderMaterial(obj.getComponent<ModelComponent>()->getMaterial());
            }
        }

        ImGui::End();
    }
}