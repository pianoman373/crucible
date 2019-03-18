#include "PropertiesPanel.hpp"
#include "PrefabView.hpp"

#include <imgui.h>
#include <crucible/Path.hpp>
#include <tinyfiledialogs.h>
#include <btBulletDynamicsCommon.h>

#include <crucible/GameObject.hpp>

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
            value.load(Path(file));
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


PropertiesPanel::PropertiesPanel(PrefabView &view): view(view) {

}

void PropertiesPanel::renderContents() {
    if (ImGui::Begin("Properties"))
    {
        if (view.selectedObject != nullptr) {
            GameObject *obj = view.selectedObject;


            char newText[256];
            strncpy(newText, obj->getName().c_str(), sizeof(newText));
            ImGui::InputText("Material Name", newText, 256);
            obj->setName(newText);

            ImGui::DragFloat3("Position", &obj->transform.position.x, 0.1f);
            ImGui::DragFloat3("Scale", &obj->transform.scale.x, 0.1f);

            static vec3 euler;
            static GameObject *lastObj = obj;

            if (lastObj != obj) {
                euler = obj->transform.rotation.toEuler();
                lastObj = obj;
            }

            ImGui::DragFloat3("Rotation", &euler.x, 0.1f);

            obj->transform.rotation = quaternion(vec3(0.0f, 0.0f, 1.0f), radians(euler.z)) * quaternion(vec3(0.0f, 1.0f, 0.0f), radians(euler.y)) * quaternion(vec3(1.0f, 0.0f, 0.0f), radians(euler.x));

            ImGui::Separator();

            if (ImGui::CollapsingHeader("Material"))
            {
                renderMaterial(obj->getComponent<ModelComponent>()->getMaterial());
            }
        }

        ImGui::End();
    }
}