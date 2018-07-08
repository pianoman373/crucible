#pragma once

#include <crucible/Mesh.hpp>
#include <crucible/AABB.hpp>
#include <crucible/Camera.hpp>
#include <crucible/Shader.hpp>

#include <vector>

class DebugRenderer {
private:
    Shader debugShader;
    Mesh debugRendererMesh;

    std::vector<vec3> positions;
    std::vector<vec3> colors;

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

    void renderDebugAABB(AABB aabb, vec3 color);

    void renderDebugSphere(vec3 pos, float radius, vec3 color);

    /**
     * Renders the accumulated debug calls into one big mesh. Generally the renderer will call this on it's own.
     */
    void flush(Camera cam);
};