#pragma once

#include "EditorContext.hpp"


#include <crucible/Camera.hpp>

class ScenePanel {
private:
    EditorContext &context;

    int selection = -1;

    void renderNode(GameObject &obj, int &i);

public:

    ScenePanel(EditorContext &context);

    void renderContents();
};