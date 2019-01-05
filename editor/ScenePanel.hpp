#pragma once

#include "EditorContext.hpp"


#include <crucible/Camera.hpp>

class ScenePanel {
private:
    EditorContext &context;

    void renderGrid();

public:

    ScenePanel(EditorContext &context);

    void renderContents();
};