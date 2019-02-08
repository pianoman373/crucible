#include <crucible/DebugRenderer.hpp>
#include <crucible/Renderer.hpp>

#include <glad/glad.h>

#include <Resource.h>

void DebugRenderer::init() {
    debugShader.load(LOAD_RESOURCE(src_shaders_debug_vsh).data(), LOAD_RESOURCE(src_shaders_debug_fsh).data());
}

void DebugRenderer::renderDebugLine(const vec3 &v1, const vec3 &v2, const vec3 &color) {
    positions.push_back(v1);
    colors.push_back(color);

    positions.push_back(v2);
    colors.push_back(color);
}
// ------------------------------------------------------------------------
void DebugRenderer::renderDebugAABB(const vec3 &v1, const vec3 &v2, const vec3 &color) {
    //top square
    renderDebugLine(vec3(v1.x, v1.y, v1.z), vec3(v2.x, v1.y, v1.z), color);
    renderDebugLine(vec3(v1.x, v1.y, v1.z), vec3(v1.x, v1.y, v2.z), color);
    renderDebugLine(vec3(v2.x, v1.y, v1.z), vec3(v2.x, v1.y, v2.z), color);
    renderDebugLine(vec3(v1.x, v1.y, v2.z), vec3(v2.x, v1.y, v2.z), color);

    //bottom square
    renderDebugLine(vec3(v2.x, v2.y, v2.z), vec3(v1.x, v2.y, v2.z), color);
    renderDebugLine(vec3(v2.x, v2.y, v2.z), vec3(v2.x, v2.y, v1.z), color);
    renderDebugLine(vec3(v1.x, v2.y, v1.z), vec3(v1.x, v2.y, v2.z), color);
    renderDebugLine(vec3(v1.x, v2.y, v1.z), vec3(v2.x, v2.y, v1.z), color);

    //top/bottom square connections
    renderDebugLine(vec3(v1.x, v1.y, v1.z), vec3(v1.x, v2.y, v1.z), color);
    renderDebugLine(vec3(v2.x, v2.y, v2.z), vec3(v2.x, v1.y, v2.z), color);
    renderDebugLine(vec3(v2.x, v1.y, v1.z), vec3(v2.x, v2.y, v1.z), color);
    renderDebugLine(vec3(v1.x, v1.y, v2.z), vec3(v1.x, v2.y, v2.z), color);
}
// ------------------------------------------------------------------------
void DebugRenderer::renderDebugAABB(const AABB &aabb, const vec3 &color) {
    renderDebugAABB(aabb.min, aabb.max, color);
}

void DebugRenderer::renderDebugSphere(const vec3 &pos, float radius, const vec3 &color) {
    float pi = 3.141592f;

    for (int i = 0; i < 64; i++) {
        float x1 = sin(i * ((pi*2)/64)) * radius;
        float z1 = cos(i * ((pi*2)/64)) * radius;

        float x2 = sin((i+1) * ((pi*2)/64)) * radius;
        float z2 = cos((i+1) * ((pi*2)/64)) * radius;

        renderDebugLine(pos + vec3(x1, 0, z1), pos + vec3(x2, 0, z2), color);
        renderDebugLine(pos + vec3(x1, z1, 0), pos + vec3(x2, z2, 0), color);
        renderDebugLine(pos + vec3(0, z1, x1), pos + vec3(0, z2, x2), color);
    }
}

void DebugRenderer::flush(const Camera &cam) {
    debugRendererMesh.positions = positions;
    debugRendererMesh.normals = colors;

    debugRendererMesh.generate();

    positions.clear();
    colors.clear();

    debugShader.bind();
    debugShader.uniformMat4("view", cam.getView());
    debugShader.uniformMat4("projection", cam.getProjection());
    debugRendererMesh.renderMode = GL_LINES;
    debugRendererMesh.render();
}