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
    void renderDebugLine(const vec3 &v1, const vec3 &v2, const vec3 &color);

    /**
     * Draw a cube with specified color and min and max corners.
     */
    void renderDebugAABB(const vec3 &v1, const vec3 &v2, const vec3 &color);

    void renderDebugAABB(const AABB &aabb, const vec3 &color);

    void renderDebugSphere(const vec3 &pos, float radius, const vec3 &color);

    /**
     * Renders the accumulated debug calls into one big mesh. Generally the renderer will call this on it's own.
     */
    void flush(const Camera &cam);
};