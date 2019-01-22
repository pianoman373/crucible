#include "ScenePanel.hpp"

#include <imgui.h>

#include <crucible/Primitives.hpp>

void ScenePanel::renderNode(GameObject &obj, int &i) {
    i++;
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ((selection == i) ? ImGuiTreeNodeFlags_Selected : 0);

    if (obj.getNumChildren() > 0) {
        ImGui::PushID(i);

        bool node_open = ImGui::TreeNodeEx(obj.getName().c_str(), node_flags);
        ImGui::PopID();
        if (ImGui::IsItemClicked()) {
            selection = i;
            context.selectedObject = &obj;
        }

        if (node_open)
        {
            for (int n = 0; n < obj.getNumChildren(); n++)
            {

                renderNode(obj.getChild(n), i);
            }

            ImGui::TreePop();
        }
    }
    else {
        ImGui::PushID(i);
        node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        bool node_open = ImGui::TreeNodeEx(obj.getName().c_str(), node_flags);
        ImGui::PopID();
        if (ImGui::IsItemClicked()) {
            selection = i;
            context.selectedObject = &obj;
        }
    }
}

ScenePanel::ScenePanel(EditorContext &context) : context(context) {

}

void ScenePanel::renderContents() {
    if (ImGui::Begin("Scene")) {
        if (ImGui::Button("Add Cube")) {
            Material *mat = new Material();
            mat->setPBRUniforms(vec3(0.5f), 0.4f, 0.0f);

            context.materialCache.push_back(mat);

            Mesh *mesh = new Mesh();
            Primitives::cube(*mesh, 1.0f, 1.0f, 1.0f);

            context.meshCache.push_back(mesh);

            GameObject &obj = context.scene.createMeshObject(*mesh, *mat, Transform(), "Cube");
            GameObject &child = obj.createChild(Transform(vec3(3.0f, 0.0f, 0.0f)), "Cube Child");
            child.addComponent(new ModelComponent(*mesh, *mat));
        }

        ImGui::Text("%i", selection);

        int i = 0;
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ((selection == -1) ? ImGuiTreeNodeFlags_Selected : 0);
        ImGui::SetNextTreeNodeOpen(true);
        bool node_open = ImGui::TreeNodeEx("Scene", node_flags);
        if (ImGui::IsItemClicked()) {
            selection = -1;
            context.selectedObject = nullptr;
        }
        if (node_open)
        {
            for (int n = 0; n < context.scene.numObjects(); n++)
            {
                GameObject &object = context.scene.getObject(n);

                renderNode(object, i);
            }

            ImGui::TreePop();
        }
    }
}