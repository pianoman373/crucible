#include <crucible/PostProcessor.hpp>
#include <crucible/Renderer.hpp>
#include <crucible/Window.hpp>
#include <crucible/Resources.hpp>
#include <crucible/Resource.h>

#include <glad/glad.h>

#include <random>


static float gaussianDistribution (float x, float sigma)
{
    float d = x;
    float n = 1.0f / (sqrt(2.0f * PI) * sigma);

    return exp(-d*d/(2.0f * sigma * sigma)) * n;
};

static void uniformGaussians(const Shader &s, std::string name, int radius) {
    for (int i = 0; i < radius; i++) {
        s.uniformFloat(name + "[" + std::to_string(i) + "]", gaussianDistribution(i, 1.0f));
    }

    s.uniformInt(name + "_length", radius);
}

void PostProcessor::doBloom(const Texture &deferred) {
    int blurRadius = 8;

    glViewport(0, 0, bloomBuffer0.getWidth(), bloomBuffer0.getHeight());
    bloomBuffer1.bind();
    Resources::passthroughShader.bind();
    deferred.bind();
    Resources::framebufferMesh.render();

    bloomBuffer0.bind();
    Resources::gaussianBlurShader.bind();
    Resources::gaussianBlurShader.uniformBool("horizontal", true);
    uniformGaussians(Resources::gaussianBlurShader, "weights", blurRadius);
    bloomBuffer1.getAttachment(0).bind();
    Resources::framebufferMesh.render();

    bloomBuffer1.bind();
    Resources::gaussianBlurShader.bind();
    Resources::gaussianBlurShader.uniformBool("horizontal", false);
    uniformGaussians(Resources::gaussianBlurShader, "weights", blurRadius);
    bloomBuffer0.getAttachment(0).bind();
    Resources::framebufferMesh.render();

    // -------------------------------------------------------------------------

    glViewport(0, 0, bloomBuffer3.getWidth(), bloomBuffer3.getHeight());
    bloomBuffer3.bind();
    Resources::passthroughShader.bind();
    bloomBuffer1.getAttachment(0).bind();
    Resources::framebufferMesh.render();

    bloomBuffer2.bind();
    Resources::gaussianBlurShader.bind();
    Resources::gaussianBlurShader.uniformBool("horizontal", true);
    uniformGaussians(Resources::gaussianBlurShader, "weights", blurRadius);
    bloomBuffer3.getAttachment(0).bind();
    Resources::framebufferMesh.render();

    bloomBuffer3.bind();
    Resources::gaussianBlurShader.bind();
    Resources::gaussianBlurShader.uniformBool("horizontal", false);
    uniformGaussians(Resources::gaussianBlurShader, "weights", blurRadius);
    bloomBuffer2.getAttachment(0).bind();
    Resources::framebufferMesh.render();

    // -------------------------------------------------------------------------

    glViewport(0, 0, bloomBuffer5.getWidth(), bloomBuffer5.getHeight());
    bloomBuffer5.bind();
    Resources::passthroughShader.bind();
    bloomBuffer3.getAttachment(0).bind();
    Resources::framebufferMesh.render();

    bloomBuffer4.bind();
    Resources::gaussianBlurShader.bind();
    Resources::gaussianBlurShader.uniformBool("horizontal", true);
    uniformGaussians(Resources::gaussianBlurShader, "weights", blurRadius);
    bloomBuffer5.getAttachment(0).bind();
    Resources::framebufferMesh.render();

    bloomBuffer5.bind();
    Resources::gaussianBlurShader.bind();
    Resources::gaussianBlurShader.uniformBool("horizontal", false);
    uniformGaussians(Resources::gaussianBlurShader, "weights", blurRadius);
    bloomBuffer4.getAttachment(0).bind();
    Resources::framebufferMesh.render();
}



void PostProcessor::init() {
    //setup SSAO
    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between 0.0 - 1.0
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 256; ++i)
    {
        vec3 sample(
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator) * 2.0 - 1.0,
                randomFloats(generator)
        );
        sample  = normalize(sample);
        sample = sample * randomFloats(generator);
        float scale = (float)i / 256.0;

        scale   = lerp(0.1f, 1.0f, scale * scale);
        sample = sample * scale;
        ssaoKernel.push_back(sample);
    }

    std::vector<vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
        ssaoNoise.push_back(noise);
    }
    unsigned int noiseTexture;
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    noiseTex.setID(noiseTexture);

    resize();
}

void PostProcessor::resize() {
    vec2i resolution = Renderer::getResolution();

    // HDR framebuffer 0
    HDRbuffer0.destroy();
    HDRbuffer0.setup(resolution.x, resolution.y);
    HDRbuffer0.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);

    // HDR framebuffer 1
    HDRbuffer1.destroy();
    HDRbuffer1.setup(resolution.x, resolution.y);
    HDRbuffer1.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);

    //bloom framebuffers
    bloomBuffer0.destroy();
    bloomBuffer0.setup(resolution.x/2, resolution.y/2);
    bloomBuffer0.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
    bloomBuffer1.destroy();
    bloomBuffer1.setup(resolution.x/2, resolution.y/2);
    bloomBuffer1.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);

    bloomBuffer2.destroy();
    bloomBuffer2.setup(resolution.x/8, resolution.y/8);
    bloomBuffer2.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
    bloomBuffer3.destroy();
    bloomBuffer3.setup(resolution.x/8, resolution.y/8);
    bloomBuffer3.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);

    bloomBuffer4.destroy();
    bloomBuffer4.setup(resolution.x/16, resolution.y/16);
    bloomBuffer4.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
    bloomBuffer5.destroy();
    bloomBuffer5.setup(resolution.x/16, resolution.y/16);
    bloomBuffer5.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);

    ssrBlurBuffer0.destroy();
    ssrBlurBuffer0.setup(resolution.x/8, resolution.y/8);
    ssrBlurBuffer0.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);

    ssrBlurBuffer1.destroy();
    ssrBlurBuffer1.setup(resolution.x/8, resolution.y/8);
    ssrBlurBuffer1.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);

    ssaoBuffer.destroy();
    ssaoBuffer.setup(resolution.x, resolution.y);
    ssaoBuffer.attachTexture(GL_RED, GL_RED, GL_UNSIGNED_BYTE);

    ssaoBufferBlur.destroy();
    ssaoBufferBlur.setup(resolution.x, resolution.y);
    ssaoBufferBlur.attachTexture(GL_RED, GL_RED, GL_UNSIGNED_BYTE);
}

Texture PostProcessor::postRender(const Camera &cam, const Texture &deferred, const Texture &gPosition, const Texture &gNormal, const Texture &gAlbedo, const Texture &gRoughnessMetallic) {
    vec2i resolution = Renderer::getResolution();

    if (ssao) {
        // render the g-buffers for SSAO
        // ---------------------------------------------
        ssaoBuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Resources::ssaoShader.bind();

        Resources::ssaoShader.uniformInt("gPosition", 0);
        gPosition.bind(0);

        Resources::ssaoShader.uniformInt("gNormal", 1);
        gNormal.bind(1);

        Resources::ssaoShader.uniformInt("texNoise", 2);
        noiseTex.bind(2);

        Resources::ssaoShader.uniformMat4("projection", cam.getProjection());
        Resources::ssaoShader.uniformFloat("radius", ssaoRadius);
        Resources::ssaoShader.uniformInt("kernelSize", ssaoKernelSize);
        Resources::ssaoShader.uniformVec3("noiseScale",  vec3(resolution.x/4.0f, resolution.y/4.0f, 0.0f));

        for (int i = 0; i < ssaoKernelSize; i++) {
            Resources::ssaoShader.uniformVec3(std::string("samples[") + std::to_string(i) + std::string("]"), ssaoKernel[i]);
        }
        Resources::framebufferMesh.render();

        ssaoBufferBlur.bind();
        Resources::ssaoBlurShader.bind();
        ssaoBuffer.getAttachment(0).bind();
        Resources::framebufferMesh.render();
    }

    // SSR
    if (SSR){
        glViewport(0, 0, ssrBlurBuffer0.getWidth(), ssrBlurBuffer0.getHeight());
        ssrBlurBuffer0.bind();
        Resources::passthroughShader.bind();
        deferred.bind();
        Resources::framebufferMesh.render();

        ssrBlurBuffer1.bind();
        Resources::gaussianBlurShader.bind();
        Resources::gaussianBlurShader.uniformBool("horizontal", true);
        ssrBlurBuffer0.getAttachment(0).bind();
        Resources::framebufferMesh.render();

        ssrBlurBuffer0.bind();
        Resources::gaussianBlurShader.bind();
        Resources::gaussianBlurShader.uniformBool("horizontal", false);
        ssrBlurBuffer1.getAttachment(0).bind();
        Resources::framebufferMesh.render();

        glViewport(0, 0, resolution.x, resolution.y);

        HDRbuffer0.bind();
        Resources::ssrShader.bind();

        Resources::ssrShader.uniformMat4("view", cam.getView());
        Resources::ssrShader.uniformMat4("projection", cam.getProjection());


        Resources::ssrShader.uniformInt("gPosition", 0);
        gPosition.bind(0);

        Resources::ssrShader.uniformInt("gNormal", 1);
        gNormal.bind(1);

        Resources::ssrShader.uniformInt("gAlbedo", 2);
        gAlbedo.bind(2);

        Resources::ssrShader.uniformInt("gRoughnessMetallic", 3);
        gRoughnessMetallic.bind(3);

        Resources::ssrShader.uniformInt("deferred", 4);
        deferred.bind(4);

        Resources::ssrShader.uniformInt("deferredBlur", 5);
        ssrBlurBuffer0.getAttachment(0).bind(5);

        Resources::ssrShader.uniformInt("prefilter", 6);
        Renderer::specular.bind(6);
        Resources::ssrShader.uniformInt("brdf", 7);
        Resources::brdf.bind(7);

        Resources::framebufferMesh.render();
    }

    // bloom
    // ---------------

    if (bloom) {
        doBloom(deferred);

        glViewport(0, 0, resolution.x, resolution.y);
    }

    // post processing
    // ---------------
    HDRbuffer1.bind();

    bloomBuffer1.getAttachment(0).bind(0);
    bloomBuffer3.getAttachment(0).bind(1);
    bloomBuffer5.getAttachment(0).bind(2);

    if (SSR) {
        HDRbuffer0.getAttachment(0).bind(3);
    }
    else {
        deferred.bind(3);
    }

    ssaoBufferBlur.getAttachment(0).bind(4);
    gPosition.bind(5);

    Resources::tonemapShader.bind();
    Resources::tonemapShader.uniformBool("vignette", vignette);
    Resources::tonemapShader.uniformBool("tonemap", tonemap);
    Resources::tonemapShader.uniformBool("bloom", bloom);
    Resources::tonemapShader.uniformBool("ssao", ssao);
    Resources::tonemapShader.uniformFloat("bloomStrength", bloomStrength);
    Resources::tonemapShader.uniformFloat("fogInner", fogInner);
    Resources::tonemapShader.uniformFloat("fogOuter", fogOuter);
    Resources::tonemapShader.uniformInt("bloom0", 0);
    Resources::tonemapShader.uniformInt("bloom1", 1);
    Resources::tonemapShader.uniformInt("bloom2", 2);
    Resources::tonemapShader.uniformInt("deferred", 3);
    Resources::tonemapShader.uniformInt("ssaoTexture", 4);
    Resources::tonemapShader.uniformInt("gPosition", 5);
    Resources::tonemapShader.uniformVec3("camPosition", cam.position);

    Resources::framebufferMesh.render();


    // fxaa
    // -------------------------------
    if (fxaa) {
        HDRbuffer0.bind();
        glViewport(0, 0, resolution.x, resolution.y);
        Resources::fxaaShader.bind();
        HDRbuffer1.getAttachment(0).bind();
        Resources::framebufferMesh.render();

        return HDRbuffer0.getAttachment(0);
    }

    return HDRbuffer1.getAttachment(0);
}