#include "HeirarchyPanel.hpp"
#include "PrefabView.hpp"

#include <imgui.h>

#include <crucible/Primitives.hpp>

void HeirarchyPanel::renderNode(GameObject &obj, int &i) {
    i++;
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ((selection == i) ? ImGuiTreeNodeFlags_Selected : 0);

    if (obj.getNumChildren() > 0) {
        ImGui::PushID(i);

        bool node_open = ImGui::TreeNodeEx(obj.getName().c_str(), node_flags);
        ImGui::PopID();
        if (ImGui::IsItemClicked()) {
            selection = i;
            view.selectedObject = &obj;
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
            view.selectedObject = &obj;
        }
    }
}

HeirarchyPanel::HeirarchyPanel(PrefabView &view) : view(view) {

}

void HeirarchyPanel::renderContents() {
    if (ImGui::Begin("Heirarchy")) {
        if (ImGui::Button("Add Cube")) {
            Material *mat = new Material();
            mat->setPBRUniforms(vec3(0.5f), 0.4f, 0.0f);

            view.context.materialCache.push_back(mat);

            Mesh *mesh = new Mesh();
            *mesh = Primitives::cube();

            view.context.meshCache.push_back(mesh);

            GameObject &obj = view.scene.createMeshObject(*mesh, *mat, Transform(), "Cube");
        }

        //ImGui::Text("%i", selection);

        int i = 0;
        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ((selection == -1) ? ImGuiTreeNodeFlags_Selected : 0);
        ImGui::SetNextTreeNodeOpen(true);
        bool node_open = ImGui::TreeNodeEx("Root", node_flags);
        if (ImGui::IsItemClicked()) {
            selection = -1;
            view.selectedObject = nullptr;
        }
        if (node_open)
        {
            for (int n = 0; n < view.scene.numObjects(); n++)
            {
                GameObject &object = view.scene.getObject(n);

                renderNode(object, i);
            }

            ImGui::TreePop();
        }

        ImGui::End();
    }
}