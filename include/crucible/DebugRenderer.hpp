#pragma once

#include <crucible/MeshFactory.hpp>
#include <crucible/AABB.hpp>
#include <crucible/Camera.hpp>
#include <crucible/Shader.hpp>

class DebugRenderer {
private:
    MeshFactory debugRendererFactory;
    Shader debugShader;

public:
    void init();

    /**
     * Render a line of the specified color from point v1 to v2
     */
    void renderDebugLine(vec3 v1, vec3 v2, vec3 color);

    /**
     * Draw a cube with specified color and min and max corners.
     */
    void renderDebugAABB(vec3 v1, vec3 v2, vec3 color);

    /**
     * Draw a cube with specified color and AABB.
     */
    void renderDebugAABB(AABB aabb, vec3 color);

    void flush(Camera cam);
};