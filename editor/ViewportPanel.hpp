#pragma once

#include "EditorContext.hpp"


#include <crucible/Camera.hpp>

class ViewportPanel {
private:
    EditorContext &context;
    Camera cam;

    void renderGrid();

public:

    ViewportPanel(EditorContext &context);

    void renderContents();
};