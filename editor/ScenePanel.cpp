#include "ScenePanel.hpp"

#include <imgui.h>

#include <crucible/Primitives.hpp>

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

            context.scene.createMeshObject(*mesh, *mat, Transform(), "Cube");
        }

        ImGui::Text("%i", context.selectedObject);
        ImGui::Text("Meshes");
        ImGui::Separator();

        for (int n = 0; n < context.scene.numObjects(); n++)
        {
            GameObject &object = context.scene.getObject(n);


            ImGui::PushID(n);
            if (ImGui::Selectable(object.getName().c_str(), context.selectedObject == n)) {
                context.selectedObject = n;
            }
            ImGui::PopID();
        }

    }
}