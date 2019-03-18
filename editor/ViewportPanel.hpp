#pragma once

class PrefabView;


#include <crucible/Camera.hpp>

class ViewportPanel {
private:
    PrefabView &view;
    Camera cam;

    vec3 camDirection = normalize(vec3(0.0f, 0.3f, 1.0f));
    float camDistance = 10.0f;
    vec3 camOffset = vec3();


    void renderGrid();

    void orbitCamera(Camera &cam);

public:

    ViewportPanel(PrefabView &view);

    void renderContents();
};