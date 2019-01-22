#include "ProjectPanel.hpp"
#include "tinydir.h"

#include <imgui.h>

ProjectPanel::ProjectPanel(EditorContext &context): context(context) {

}


void ProjectPanel::renderContents() {
    if (ImGui::Begin("Project"))
    {
        if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
            if (previousDirs.size() > 0) {
                path = previousDirs.top();
                previousDirs.pop();
            }
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {

        }

        ImGui::SameLine();
        ImGui::Text(context.projectPath.appendPath(path).toString().c_str());

        ImGui::Separator();

        ImGui::BeginChild("files");


        tinydir_dir dir;
        int i;
        tinydir_open_sorted(&dir, (context.projectPath.appendPath(path)).toString().c_str());

        for (i = 0; i < dir.n_files; i++) {
            tinydir_file file;
            tinydir_readfile_n(&dir, &file, i);

            if (file.name[0] == '.') {
                continue;
            }

            if (file.is_dir) {
                if (ImGui::Selectable((std::string(file.name) + std::string("/")).c_str(), selectedItem == i, ImGuiSelectableFlags_AllowDoubleClick)) {
                    selectedItem = i;

                    if (ImGui::IsMouseDoubleClicked(0)) {
                        path.appendFolder(file.name);
                    }
                }
            }
            else {
                if (ImGui::Selectable(file.name, selectedItem == i, ImGuiSelectableFlags_AllowDoubleClick)) {
                    selectedItem = i;
                }
            }
        }

        tinydir_close(&dir);

        ImGui::EndChild();
        ImGui::End();
    }
}