#pragma once

#include <crucible/Framebuffer.hpp>
#include <crucible/Camera.hpp>
#include <crucible/Shader.hpp>

class PostProcessor {
private:
    Framebuffer HDRbuffer0;
    Framebuffer HDRbuffer1;
    Framebuffer bloomBuffer0;
    Framebuffer bloomBuffer1;
    Framebuffer bloomBuffer2;
    Framebuffer bloomBuffer3;
    Framebuffer bloomBuffer4;
    Framebuffer bloomBuffer5;
    Framebuffer ssaoBuffer;
    Framebuffer ssaoBufferBlur;
    Framebuffer ssrBlurBuffer0;
    Framebuffer ssrBlurBuffer1;

    std::vector<vec3> ssaoKernel;
    Texture noiseTex;

    void doBloom(const Texture &deferred);

public:
    bool fxaa = true;
    bool vignette = true;
    bool tonemap = true;
    bool bloom = true;
    bool SSR = true;
    float bloomStrength = 0.05f;
    bool ssao = true;
    float ssaoRadius = 10.0f;
    int ssaoKernelSize = 8;
    float fogInner = 100.0f;
    float fogOuter = 150.0f;

    void init();

    void resize();

    Texture postRender(const Camera &cam, const Texture &deferred, const Texture &gPosition, const Texture &gNormal, const Texture &gAlbedo, const Texture &gRoughnessMetallic);
};