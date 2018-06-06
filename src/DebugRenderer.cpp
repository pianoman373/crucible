#include <crucible/DebugRenderer.hpp>
#include <crucible/Renderer.hpp>
#include <crucible/InternalShaders.hpp>

#include <glad/glad.h>

void DebugRenderer::init() {
    debugShader.load(InternalShaders::debug_vsh, InternalShaders::debug_fsh);
}

void DebugRenderer::renderDebugLine(vec3 v1, vec3 v2, vec3 color) {
    debugRendererFactory.vertex(v1.x, v1.y, v1.z, color.x, color.y, color.z, 0.0f, 0.0f);
    debugRendererFactory.vertex(v2.x, v2.y, v2.z, color.x, color.y, color.z, 0.0f, 0.0f);
}
// ------------------------------------------------------------------------
void DebugRenderer::renderDebugAABB(vec3 v1, vec3 v2, vec3 color) {
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
void DebugRenderer::renderDebugAABB(AABB aabb, vec3 color) {
    renderDebugAABB(aabb.min, aabb.max, color);
}

void DebugRenderer::flush(Camera cam) {
    debugRendererFactory.toMesh(debugRendererMesh);
    debugShader.bind();
    debugShader.uniformMat4("view", cam.getView());
    debugShader.uniformMat4("projection", cam.getProjection());
    debugRendererMesh.renderMode = GL_LINES;
    debugRendererMesh.render();
    debugRendererFactory.clear();
}