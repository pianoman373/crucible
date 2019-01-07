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
#include "ScenePanel.hpp"
#include "PropertiesPanel.hpp"
#include "ProjectPanel.hpp"

static EditorContext context;

static Cubemap cubemap;
static Texture cubemapRaw;

static bool showGraphicsSettings = false;

static void ShowExampleAppDockSpace(bool* p_open)
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
    ImGui::Begin("DockSpace Demo", p_open, window_flags);
    ImGui::PopStyleVar(3);

    ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);

    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                char const *file = tinyfd_openFileDialog("Open model file", "", 0, NULL, "3d model file", 0);

                if (file != NULL) {
                    Path filename = file;

                    json j;
                    std::ifstream o(filename);
                    o >> j;

                    Model *m = new Model();

                    m->fromJson(j, filename.getParent());
                }
            }
            if (ImGui::MenuItem("Open Project")) {
                char const *folder = tinyfd_selectFolderDialog("Open Project Folder", context.projectPath.c_str());

                std::cout << folder << std::endl;

                context.projectPath = folder;
                context.projectPath += "/";
            }
            if (ImGui::MenuItem("Import", "Ctrl+I")) {
                char const *file = tinyfd_openFileDialog("Open model file", "", 0, NULL, "3d model file", 0);

                if (file != NULL) {
                    Assimp::Importer importer;

                    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices);


                    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
                    {
                        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
                    }

                    for (int i = 0; i < scene->mNumMaterials; i++) {
                        aiMaterial *aMaterial = scene->mMaterials[i];

                        Material *material = new Material();

                        material->setShader(Renderer::standardShader);
                        material->setDefaultPBRUniforms();

                        aiString materialName;
                        aMaterial->Get(AI_MATKEY_NAME, materialName);

                        material->name = materialName.C_Str();

                        //context.materialCache.push_back(material);
                    }

                    for (int i = 0; i < scene->mNumMeshes; i++) {
                        aiMesh *aMesh = scene->mMeshes[i];

                        Mesh *mesh = new Mesh();

                        mesh->positions.resize(aMesh->mNumVertices);
                        mesh->normals.resize(aMesh->mNumVertices);
                        mesh->indices.resize(aMesh->mNumFaces * 3);
                        mesh->tangents.resize(aMesh->mNumVertices);

                        if (aMesh->mNumUVComponents > 0) {
                            mesh->uvs.resize(aMesh->mNumVertices);
                        }

                        for (unsigned int i = 0; i < aMesh->mNumVertices; ++i) {
                            mesh->positions[i] = vec3(aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z);
                            mesh->normals[i] = vec3(aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z);
                            mesh->tangents[i] = vec3(aMesh->mTangents[i].x, aMesh->mTangents[i].y, aMesh->mTangents[i].z);

                            if (isnan(mesh->tangents[i].x) || isnan(mesh->tangents[i].y) ||  isnan(mesh->tangents[i].z)) {
                                mesh->tangents[i] = {0.0f, 0.0f, 0.0f};
                            }

                            if (aMesh->mTextureCoords[0]) {
                                mesh->uvs[i] = vec2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y);
                            }
                        }

                        for (unsigned int f = 0; f < aMesh->mNumFaces; ++f) {
                            for (unsigned int i = 0; i < 3; ++i) {
                                mesh->indices[f * 3 + i] = aMesh->mFaces[f].mIndices[i];
                            }
                        }

                        mesh->generate();

                        context.meshCache.push_back(mesh);


                        Material *material = new Material();

                        material->setShader(Renderer::standardShader);
                        material->setDefaultPBRUniforms();
                        material->name = aMesh->mName.C_Str();

                        context.materialCache.push_back(material);

                        context.scene.createMeshObject(*mesh, *material, Transform(), aMesh->mName.C_Str());
                    }
                }
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                char const *filterPatterns[1] = {"*.crmodel"};
                char const *file = tinyfd_saveFileDialog("Open model file", "", 1, filterPatterns, "3d model file");

                if (file != NULL) {

                    //json j = context.model.toJson(Path::getWorkingDirectory(path));

                    //std::ofstream o(file);
                    //o << std::setw(4) << j << std::endl;
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

        ImGui::EndMenuBar();
    }

    ImGui::End();

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
            ImGui::Image(ImTextureID((long long) cubemapRaw.getID()), ImVec2(220, 110), ImVec2(0, 0),
                         ImVec2(1, 1), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));

            if (ImGui::Button("Browse")) {
                char const *file = tinyfd_openFileDialog("Open environment map", "", 0, NULL, "environment map", 0);

                if (file != NULL) {
                    cubemap.loadEquirectangular(file);
                    cubemapRaw.load(file);

                    Renderer::environment = cubemap;
                    IBL::generateIBLmaps(vec3(), Renderer::irradiance, Renderer::specular);
                    cubemap.setID(0);
                    Renderer::environment = cubemap;
                }
            }


        }
        ImGui::End();
    }
}


int main() {
    context.loadFromConfig();

    Window::create({ 1280, 720 }, "test", false, false);

    Renderer::init(true, 2048, 1280, 720);
    Renderer::settings.tonemap = true;
    Renderer::settings.vignette = false;
    Renderer::settings.bloom = true;
    Renderer::settings.ssao = true;


    cubemap.loadEquirectangular("resources/canyon.hdr");
    cubemapRaw.load("resources/canyon.hdr");

    Renderer::environment = cubemap;
    IBL::generateIBLmaps(vec3(), Renderer::irradiance, Renderer::specular);
    Renderer::environment.setID(0);

    Renderer::setSun({ vec3(1.05f, -1.2f, -1.3f), vec3(10.0f, 10.0f, 10.0f) });

    ViewportPanel viewport(context);
    ScenePanel scene(context);
    PropertiesPanel properties(context);
    ProjectPanel project(context);


    while (Window::isOpen()) {
        Window::begin();

        bool p_open = true;


        ShowExampleAppDockSpace(&p_open);
        viewport.renderContents();
        scene.renderContents();
        properties.renderContents();
        project.renderContents();

        Window::end();
    }

    context.saveToConfig();
}
