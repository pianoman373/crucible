#pragma once

#include <crucible/Framebuffer.hpp>
#include <crucible/Camera.hpp>
#include <crucible/Shader.hpp>

class PostProcessor {
private:
    Shader tonemapShader;
    Shader fxaaShader;
    Shader gaussianBlurShader;
    Shader ssaoShader;
    Shader ssaoBlurShader;
    Shader ssrShader;

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
    void init();

    void resize();

    Texture postRender(const Camera &cam, const Texture &deferred, const Texture &gPosition, const Texture &gNormal, const Texture &gAlbedo, const Texture &gRoughnessMetallic);
};