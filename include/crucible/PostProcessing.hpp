#pragma once

#include <crucible/Framebuffer.hpp>
#include <crucible/Camera.hpp>
#include <crucible/Shader.hpp>

class PostProcessor {
public:
    virtual void postProcess(const Camera &cam, const Framebuffer &source, const Framebuffer &destination);

    virtual void resize();
};

class FxaaPostProcessor: public PostProcessor {
public:
    void postProcess(const Camera &cam, const Framebuffer &source, const Framebuffer &destination);
};

class TonemapPostProcessor: public PostProcessor {
public:
    void postProcess(const Camera &cam, const Framebuffer &source, const Framebuffer &destination);
};

class SsaoPostProcessor: public PostProcessor {
private:
    Framebuffer ssaoBuffer;

    std::vector<vec3> ssaoKernel;
    Texture noiseTex;

public:
    float ssaoRadius = 4.0f;
    int ssaoKernelSize = 8;

    SsaoPostProcessor();

    virtual void postProcess(const Camera &cam, const Framebuffer &source, const Framebuffer &destination);

    virtual void resize();
};


class BloomPostProcessor: public PostProcessor {
private:
    Framebuffer bloomBuffer0;
    Framebuffer bloomBuffer1;
    Framebuffer bloomBuffer2;
    Framebuffer bloomBuffer3;
    Framebuffer bloomBuffer4;
    Framebuffer bloomBuffer5;

public:
    float bloomStrength = 0.05f;

    BloomPostProcessor();

    virtual void postProcess(const Camera &cam, const Framebuffer &source, const Framebuffer &destination);

    virtual void resize();
};