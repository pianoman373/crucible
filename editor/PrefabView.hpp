#pragma once

#include <crucible/Scene.hpp>

#include "View.hpp"


#include "ViewportPanel.hpp"
#include "HeirarchyPanel.hpp"
#include "PropertiesPanel.hpp"
#include "ProjectPanel.hpp"
#include "EditorContext.hpp"

class PrefabView : public View {
private:
    ViewportPanel viewportPanel;
    HeirarchyPanel scenePanel;
    PropertiesPanel propertiesPanel;
    ProjectPanel projectPanel;


public:
    EditorContext &context;

    Scene scene;

    GameObject *selectedObject = nullptr;

    PrefabView(EditorContext &ctx);

    void render();
};