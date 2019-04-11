#include <crucible/crucible.hpp>
#include <crucible/Path.hpp>
#include <tinyfiledialogs.h>
#include <imgui.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <fstream>

#include "EditorContext.hpp"
#include "ViewportPanel.hpp"
#include "HeirarchyPanel.hpp"
#include "PropertiesPanel.hpp"
#include "ProjectPanel.hpp"
#include "View.hpp"
#include "PrefabView.hpp"

static EditorContext context;

static Cubemap cubemap;
static Texture cubemapRaw;

static bool showGraphicsSettings = false;

static int selectedView = 0;
static std::vector<View*> views;


static bool context1Open = true;
static bool context2Open = true;

static void renderMenuBar() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Open Project", "Ctrl+O")) {
            char const *folder = tinyfd_selectFolderDialog("Open Project Folder", context.projectPath.toString().c_str());

            std::cout << folder << std::endl;

            context.projectPath = Path(std::string(folder) + "/");
        }
        if (ImGui::MenuItem("Import", "Ctrl+I")) {
            char const *file = tinyfd_openFileDialog("Open model file", "", 0, NULL, "3d model file", 0);

            if (file != NULL) {

            }
        }
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
//            char const *filterPatterns[1] = {"*.crmodel"};
//            char const *file = tinyfd_saveFileDialog("Open model file", "", 1, filterPatterns, "3d model file");
//
//            if (file != NULL) {
//
//                //json j = context.model.toJson(Path::getWorkingDirectory(path));
//
//                //std::ofstream o(file);
//                //o << std::setw(4) << j << std::endl;
//            }
        }

        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("WIP")) {}
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("View")) {
//        ImGui::MenuItem("Graphics settings", NULL, &showGraphicsSettings);
        ImGui::EndMenu();
    }
}

static void ShowApplication()
{
    static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;

    // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace Demo", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);

    if (ImGui::BeginMenuBar())
    {
        renderMenuBar();

        if (ImGui::BeginTabBar("TopTab")) {

            if (ImGui::BeginTabItem("View 1", &context1Open)) {

                selectedView = 0;

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("View 2", &context2Open)) {
                selectedView = 1;

                ImGui::EndTabItem();
            }


            ImGui::EndTabBar();
        };






        ImGui::EndMenuBar();
    }

    ImGui::End();

    if (showGraphicsSettings) {
        ImGui::SetNextWindowSize(ImVec2(230, 500));
        if (ImGui::Begin("Graphics settings", &showGraphicsSettings, ImGuiWindowFlags_NoResize))
        {
            // ImGui::Text("graphics n stuff");
            // ImGui::Checkbox("fxaa", &Renderer::settings.fxaa);
            // ImGui::Checkbox("bloom", &Renderer::settings.bloom);
            // ImGui::DragFloat("bloomStrength", &Renderer::settings.bloomStrength, 0.01f, 0.0f, 1.0f);
            // ImGui::Checkbox("ssao", &Renderer::settings.ssao);
            // ImGui::Separator();
            // ImGui::Text("Environment map");
            // ImGui::Image(ImTextureID((long long) cubemapRaw.getID()), ImVec2(220, 110), ImVec2(0, 0),
            //              ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));

            // if (ImGui::Button("Browse")) {
            //     char const *file = tinyfd_openFileDialog("Open environment map", "", 0, NULL, "environment map", 0);

            //     if (file != NULL) {
            //         cubemap.loadEquirectangular(file);
            //         cubemapRaw.load(file);

            //         Renderer::environment = cubemap;
            //         IBL::generateIBLmaps(vec3(), Renderer::irradiance, Renderer::specular);
            //         cubemap.setID(0);
            //         Renderer::environment = cubemap;
            //     }
            // }


        }
        ImGui::End();
    }
}

int main() {
    context.loadFromConfig();

    Window::create({ 1280, 720 }, "test", false, false);

    Renderer::init(true, 2048, 1280, 720);


    cubemap.loadEquirectangular("resources/canyon.hdr");

    Renderer::environment = cubemap;
    IBL::generateIBLmaps(vec3(), Renderer::irradiance, Renderer::specular);
    Renderer::environment.setID(0);

    Renderer::setSun({ vec3(1.05f, -1.2f, -1.3f), vec3(10.0f, 10.0f, 10.0f) });

    views.push_back(new PrefabView(context));
    views.push_back(new PrefabView(context));




    while (Window::isOpen()) {
        Window::begin();

        bool p_open = true;


        ShowApplication();

//        if (selectedContext == 0) {
//            context.renderContents();
//        }

        views[selectedView]->render();


        Window::end();
    }

    context.saveToConfig();
}
