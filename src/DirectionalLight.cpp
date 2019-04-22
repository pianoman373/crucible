#include <crucible/DirectionalLight.hpp>
#include <crucible/Frustum.hpp>
#include <crucible/Resources.hpp>
#include <crucible/Camera.hpp>
#include <crucible/Renderer.hpp>

#include <glad/glad.h>

static const float cascadeDistances[4] = { 20.0f, 60.0f, 150.0f, 400.0f };
static const float cascadeDepths[4] = { 400.0f, 400.0f, 400.0f, 400.0f };

Camera DirectionalLight::getShadowCamera(float radius, const Camera &cam, float depth) {
    Camera ret;
    ret.position = cam.getPosition();
    ret.direction = m_direction;
    ret.up = vec3(0.0f, 1.0f, 0.0f);
    ret.dimensions = vec2(radius*2.0f, radius*2.0f);

    ret.orthographic = true;
    
    return ret;
}

Frustum DirectionalLight::getShadowFrustum(float radius, const Camera &cam, float depth) {
	Frustum shadowFrustum;
	shadowFrustum.setupInternalsOrthographic(-radius, radius, -radius, radius, -depth, depth);
	Camera shadowCam;
	shadowCam.setPosition(cam.getPosition());
	shadowCam.setDirection(m_direction);
	shadowFrustum.updateCamPosition(shadowCam);

	return shadowFrustum;
}

void DirectionalLight::setupFramebuffers() {
    int shadow_resolution = 2048;

    shadowBuffer0.setup(shadow_resolution, shadow_resolution);
    shadowBuffer1.setup(shadow_resolution, shadow_resolution);
    shadowBuffer2.setup(shadow_resolution, shadow_resolution);
    shadowBuffer3.setup(shadow_resolution, shadow_resolution);

    shadowBuffer0.attachShadow(shadow_resolution, shadow_resolution);
    shadowBuffer1.attachShadow(shadow_resolution, shadow_resolution);
    shadowBuffer2.attachShadow(shadow_resolution, shadow_resolution);
    shadowBuffer3.attachShadow(shadow_resolution, shadow_resolution);
}

DirectionalLight::DirectionalLight(vec3 direction, vec3 color): m_direction(direction), m_color(color) {

}

void DirectionalLight::preRender(const Camera &cam) {
    if (!isFramebufferSetup) {
        setupFramebuffers();
        isFramebufferSetup = true;
    }
    
    Camera shadowCamera0 = getShadowCamera(cascadeDistances[0], cam, cascadeDepths[0]);
    Camera shadowCamera1 = getShadowCamera(cascadeDistances[1], cam, cascadeDepths[1]);
    Camera shadowCamera2 = getShadowCamera(cascadeDistances[2], cam, cascadeDepths[2]);
    Camera shadowCamera3 = getShadowCamera(cascadeDistances[3], cam, cascadeDepths[3]);

    Frustum shadowFrustum0 = getShadowFrustum(cascadeDistances[0], cam, cascadeDepths[0]);
    Frustum shadowFrustum1 = getShadowFrustum(cascadeDistances[1], cam, cascadeDepths[1]);
    Frustum shadowFrustum2 = getShadowFrustum(cascadeDistances[2], cam, cascadeDepths[2]);
    Frustum shadowFrustum3 = getShadowFrustum(cascadeDistances[3], cam, cascadeDepths[3]);

    Renderer::renderToDepth(shadowBuffer0, shadowCamera0, shadowFrustum0, false);
    Renderer::renderToDepth(shadowBuffer1, shadowCamera1, shadowFrustum1, false);
    Renderer::renderToDepth(shadowBuffer2, shadowCamera2, shadowFrustum2, false);
    Renderer::renderToDepth(shadowBuffer3, shadowCamera3, shadowFrustum3, false);
}

void DirectionalLight::render(const Camera &cam) {
    Camera shadowCamera0 = getShadowCamera(cascadeDistances[0], cam, cascadeDepths[0]);
    Camera shadowCamera1 = getShadowCamera(cascadeDistances[1], cam, cascadeDepths[1]);
    Camera shadowCamera2 = getShadowCamera(cascadeDistances[2], cam, cascadeDepths[2]);
    Camera shadowCamera3 = getShadowCamera(cascadeDistances[3], cam, cascadeDepths[3]);

    Resources::deferredDirectionalShader.bind();

    mat4 inverseView = inverse(cam.getView());

    Renderer::getGBuffer().getAttachment(0).bind(0);
    Renderer::getGBuffer().getAttachment(1).bind(1);
    Renderer::getGBuffer().getAttachment(2).bind(2);
    Renderer::getGBuffer().getAttachment(3).bind(3);

    Resources::deferredDirectionalShader.uniformInt("gPosition", 0);
    Resources::deferredDirectionalShader.uniformInt("gNormal", 1);
    Resources::deferredDirectionalShader.uniformInt("gAlbedo", 2);
    Resources::deferredDirectionalShader.uniformInt("gRoughnessMetallic", 3);

    Resources::deferredDirectionalShader.uniformVec3("sun.direction", vec3(vec4(m_direction, 0.0f) * cam.getView()));
    Resources::deferredDirectionalShader.uniformVec3("sun.color", m_color);
    Resources::deferredDirectionalShader.uniformMat4("view", cam.getView());

    shadowBuffer0.getAttachment(0).bind(8);
    shadowBuffer1.getAttachment(0).bind(9);
    shadowBuffer2.getAttachment(0).bind(10);
    shadowBuffer3.getAttachment(0).bind(11);

    mat4 biasMatrix = mat4(
        0.5f, 0.0f, 0.0f, 0.0f,
        0.0f, 0.5f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f
    );

    Resources::deferredDirectionalShader.uniformInt("shadowTextures[0]", 8);
    Resources::deferredDirectionalShader.uniformInt("shadowTextures[1]", 9);
    Resources::deferredDirectionalShader.uniformInt("shadowTextures[2]", 10);
    Resources::deferredDirectionalShader.uniformInt("shadowTextures[3]", 11);

    Resources::deferredDirectionalShader.uniformMat4("lightSpaceMatrix[0]", biasMatrix * shadowCamera0.getProjection() * shadowCamera0.getView() * inverseView);
    Resources::deferredDirectionalShader.uniformMat4("lightSpaceMatrix[1]", biasMatrix * shadowCamera1.getProjection() * shadowCamera1.getView() * inverseView);
    Resources::deferredDirectionalShader.uniformMat4("lightSpaceMatrix[2]", biasMatrix * shadowCamera2.getProjection() * shadowCamera2.getView() * inverseView);
    Resources::deferredDirectionalShader.uniformMat4("lightSpaceMatrix[3]", biasMatrix * shadowCamera3.getProjection() * shadowCamera3.getView() * inverseView);

    Resources::deferredDirectionalShader.uniformFloat("cascadeDistances[0]", cascadeDistances[0]);
    Resources::deferredDirectionalShader.uniformFloat("cascadeDistances[1]", cascadeDistances[1]);
    Resources::deferredDirectionalShader.uniformFloat("cascadeDistances[2]", cascadeDistances[2]);
    Resources::deferredDirectionalShader.uniformFloat("cascadeDistances[3]", cascadeDistances[3]);

    Resources::framebufferMesh.render();
}