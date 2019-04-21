#pragma once

#include <crucible/Math.hpp>
#include <crucible/Framebuffer.hpp>
#include <crucible/Camera.hpp>
#include <crucible/Frustum.hpp>

class DirectionalLight {
private:
    bool isFramebufferSetup = false;

    Camera getShadowCamera(float radius, const Camera &cam, float depth);

    Frustum getShadowFrustum(float radius, const Camera &cam, float depth);

    void setupFramebuffers();

public:
    Framebuffer shadowBuffer0;
    Framebuffer shadowBuffer1;
    Framebuffer shadowBuffer2;
    Framebuffer shadowBuffer3;

    vec3 m_direction;
    vec3 m_color;

    DirectionalLight(vec3 direction, vec3 color);

    

    void preRender(const Camera &cam);

    void render(const Camera &cam);
};