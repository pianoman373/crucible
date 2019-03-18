#include "PrefabView.hpp"

#include <imgui.h>
#include <tinyfiledialogs.h>

#include <fstream>

PrefabView::PrefabView(EditorContext &ctx): context(ctx), viewportPanel(*this), scenePanel(*this), propertiesPanel(*this), projectPanel(*this) {

}

void PrefabView::render() {
    viewportPanel.renderContents();
    scenePanel.renderContents();
    propertiesPanel.renderContents();
    projectPanel.renderContents();
}