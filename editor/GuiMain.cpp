#include "GuiMain.hpp"

#include <imgui.h>
#include <tinyfiledialogs.h>

#include <crucible/Window.hpp>
#include <crucible/Renderer.hpp>
#include <crucible/Path.hpp>

#include <fstream>

bool ImGuiMaterialEditBool(std::string label, std::string id, Material *mat) {
    bool &val = mat->getUniformBool(id);

    ImGui::Checkbox(label.c_str(), &val);

    return val;
}

float ImGuiMaterialEditFloat(std::string label, std::string id, Material *mat, float min, float max) {
    float &val = mat->getUniformFloat(id);
    ImGui::SliderFloat(label.c_str(), &val, min, max);

    return val;
}

vec3 ImGuiMaterialEditVec3(std::string label, std::string id, Material *mat) {
    vec3 &val = mat->getUniformVec3(id);

    ImGui::ColorEdit3(label.c_str(), (float*)&val);

    return val;
}

void ImGuiMaterialEditTexture(std::string label, std::string id, Material *mat, unsigned int unit) {
    Texture value;

    auto search = mat->getTextureUniforms().find(id);
    if(search != mat->getTextureUniforms().end()) {
        value = search->second.tex;
    }
    else {
        mat->setUniformTexture(id, value, unit);
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

        mat->setUniformTexture(id, value, unit);
    }
}

void GuiMain::renderMaterial(Material *mat) {
    ImGui::Text("Material Editor");
    ImGui::Separator();


    char newText[256];
    strncpy(newText, mat->name.c_str(), sizeof(newText));
    ImGui::InputText("Name", newText, 256);
    mat->name = newText;

    ImGui::Separator();

    if (ImGuiMaterialEditBool("Albedo Textured", "albedoTextured", mat)) {
        ImGuiMaterialEditTexture("Albedo", "albedoTex", mat, 0);
    }
    else {
        ImGuiMaterialEditVec3("Albedo", "albedoColor", mat);
    }

    ImGui::Separator();

    ImGuiMaterialEditBool("Invert Roughness", "invertRoughness", mat);

    if (mat->getUniformBool("metallicTextured")) {
        ImGuiMaterialEditBool("Use Metallic Alpha", "roughnessMetallicAlpha", mat);
    }
    else {
        mat->setUniformBool("roughnessMetallicAlpha", false); // if metallicTextured gets turned off we need to turn this off as well
    }

    if (!mat->getUniformBool("roughnessMetallicAlpha")) { //if "use metallic alpha" is enabled we don't need these options
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

    if (ImGuiMaterialEditBool("Normal Textured", "normalTextured", mat)) {
        ImGuiMaterialEditTexture("Normals", "normalTex", mat, 3);
    }
}


GuiMain::GuiMain(Model *model) {
    this->model = model;
}

void GuiMain::render() {
    static bool p_open;
    static bool showGraphicsSettings = false;

    float f;

    ImGui::GetStyle().WindowRounding = 0;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(Window::getWindowSize().x, Window::getWindowSize().y));

    //left sidebar
    ImGui::SetNextWindowSize(ImVec2(230, Window::getWindowSize().y - 18));
    ImGui::SetNextWindowPos(ImVec2(0, 18));
    ImGui::SetNextWindowBgAlpha(1.0f);
    if (ImGui::Begin("Example: Property editor", &p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
    {
        ImGui::Text("%i", selected);
        ImGui::Text("Meshes");
        ImGui::Separator();

        for (int n = 0; n < model->nodes.size(); n++)
        {
            if (ImGui::Selectable(model->nodes[n].name.c_str(), selected == n))
                selected = n;
        }

        ImGui::Separator();
        ImGui::Text("Materials");
        ImGui::Separator();

        for (int n = 0; n < model->materials.size(); n++)
        {
            if (ImGui::Selectable(model->materials[n].name.c_str(), selected == n+model->nodes.size()))
                selected = n+model->nodes.size();
        }



        ImGui::End();
    }

    //right sidebar
    ImGui::SetNextWindowSize(ImVec2(230, Window::getWindowSize().y - 18));
    ImGui::SetNextWindowPos(ImVec2(Window::getWindowSize().x-230, 18));
    ImGui::SetNextWindowBgAlpha(1.0f);
    if (ImGui::Begin("right sidebar", &p_open, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings))
    {
        if (selected >= 0) {
            if (selected > model->nodes.size()-1) {
                // material is selected
                renderMaterial(&model->materials[selected - model->nodes.size()]);

            }
            else {
                // mesh is selected
                ImGui::Text("Mesh");
            }
        }

        ImGui::End();
    }


    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                char const *file = tinyfd_openFileDialog("Open model file", "", 0, NULL, "3d model file", 0);

                if (file != NULL) {
                    std::string filename = file;
                    Path::format(filename);

                    json j;
                    std::ifstream o(filename);
                    o >> j;

                    model->fromJson(j, Path::getWorkingDirectory(filename));
                }
            }
            if (ImGui::MenuItem("Import", "Ctrl+I")) {
                char const *file = tinyfd_openFileDialog("Open model file", "", 0, NULL, "3d model file", 0);

                if (file != NULL) {
                    model->importFile(file, true);
                }
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                char const * filterPatterns[1] = { "*.crmodel"};
                char const *file = tinyfd_saveFileDialog("Open model file", "", 1, filterPatterns, "3d model file");

                if (file != NULL) {
                    std::string path = file;
                    Path::format(path);

                    json j = model->toJson(Path::getWorkingDirectory(path));

                    std::ofstream o(file);
                    o << std::setw(4) << j << std::endl;
                }
            }

            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("WIP")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Graphics settings", NULL, &showGraphicsSettings);
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    if (showGraphicsSettings) {
        ImGui::SetNextWindowSize(ImVec2(230, 500));
        if (ImGui::Begin("Graphics settings", &showGraphicsSettings, ImGuiWindowFlags_NoResize))
        {
            ImGui::Text("graphics n stuff");
            ImGui::Checkbox("fxaa", &Renderer::settings.fxaa);
            ImGui::Checkbox("bloom", &Renderer::settings.bloom);
            ImGui::DragFloat("bloomStrength", &Renderer::settings.bloomStrength, 0.01f, 0.0f, 1.0f);
            ImGui::Checkbox("ssao", &Renderer::settings.ssao);
            ImGui::Separator();
            ImGui::Text("Environment map");
//            ImGui::Image(ImTextureID((long long) cubemapRaw.getID()), ImVec2(220, 110), ImVec2(0, 0),
//                         ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
//
//            if (ImGui::Button("Browse")) {
//                char const *file = tinyfd_openFileDialog("Open environment map", "", 0, NULL, "environment map", 0);
//
//                if (file != NULL) {
//                    cubemap.loadEquirectangular(file);
//                    cubemapRaw.load(file);
//
//                    Renderer::environment = cubemap;
//                    Renderer::generateIBLmaps();
//                }
//            }

        }
        ImGui::End();
    }
}