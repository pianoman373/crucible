#include <crucible/Renderer.hpp>
#include <crucible/Framebuffer.hpp>
#include <crucible/Window.hpp>
#include <crucible/Camera.hpp>
#include <crucible/AABB.hpp>
#include <crucible/Frustum.hpp>
#include <crucible/Mesh.hpp>
#include <crucible/Primitives.hpp>
#include <crucible/Math.hpp>
#include <crucible/Model.hpp>
#include <crucible/Input.hpp>
#include <crucible/IBL.hpp>
#include <crucible/DebugRenderer.hpp>
#include <crucible/Resources.hpp>
#include <crucible/Resource.h>

#include <glad/glad.h>

#include <imgui.h>

struct RenderCall {
	const IRenderable *mesh;
	const Material *material;
	const Transform *transform;
	const AABB *aabb;
	const Bone *bones;
};

// private variables
// -----------------
static const float cascadeDistances[4] = { 20.0f, 60.0f, 150.0f, 400.0f };
static const float cascadeDepths[4] = { 400.0f, 400.0f, 400.0f, 400.0f };
static DirectionalLight sun = { normalize(vec3(-0.4f, -0.7f, -1.0f)), vec3(1.4f, 1.3f, 1.0f) * 5.0f };

static std::vector<RenderCall> renderQueue;
static std::vector<RenderCall> renderQueueForward;
static std::vector<PointLight> pointLights;

static Framebuffer shadowBuffer0;
static Framebuffer shadowBuffer1;
static Framebuffer shadowBuffer2;
static Framebuffer shadowBuffer3;
static Framebuffer HDRbuffer;
static Framebuffer HDRbuffer2;
static Framebuffer gBuffer;

static bool shadows;
static int shadow_resolution;

static vec2i resolution;

static vec3 clearColor;

// private functions
// -----------------
static mat4 shadowMatrix(float radius, const Camera &cam, float depth) {
	return orthographic(-radius, radius, -radius, radius, -depth, depth) * LookAt(cam.getPosition() - sun.direction, cam.getPosition(), vec3(0.0f, 1.0f, 0.0f));
}

static Frustum shadowFrustum(float radius, const Camera &cam, float depth) {
	Frustum shadowFrustum;
	shadowFrustum.setupInternalsOrthographic(-radius, radius, -radius, radius, -depth, depth);
	Camera shadowCam;
	shadowCam.setPosition(cam.getPosition());
	shadowCam.setDirection(sun.direction);
	shadowFrustum.updateCamPosition(shadowCam);

	return shadowFrustum;
}

static void renderShadow(Framebuffer &fbuffer, mat4 lightSpaceMatrix, Frustum f, bool doFrustumCulling) {
	glViewport(0, 0, fbuffer.getWidth(), fbuffer.getHeight());
	fbuffer.bind();
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_DEPTH_BUFFER_BIT);

	for (RenderCall c : renderQueue) {
		if (doFrustumCulling) {
			if (!f.isBoxInside(*c.aabb)) {
				continue;
			}
		}

		Resources::ShadowShader.bind();

        if (c.bones) {
			Resources::ShadowShader.uniformBool("doAnimation", true);

            std::vector<mat4> skinningMatrices = c.bones->getSkinningTransforms();

            for (size_t i = 0; i < skinningMatrices.size(); i++) {
                mat4 trans = skinningMatrices.at(i);

				Resources::ShadowShader.uniformMat4("bones["+std::to_string(i)+"]", trans);
            }
        }
        else {
			Resources::ShadowShader.uniformBool("doAnimation", false);
        }

		Resources::ShadowShader.uniformMat4("lightSpaceMatrix", lightSpaceMatrix);

		Resources::ShadowShader.uniformMat4("model", c.transform->getMatrix());

		c.mesh->render();
	}
}

static GLuint queries[4]; // The unique query id
static GLuint queryResults[4]; // Save the time, in nanoseconds

static void beginQuery(int id) {
    glGetQueryObjectuiv(queries[id], GL_QUERY_RESULT, &queryResults[id]);
    glBeginQuery(GL_TIME_ELAPSED, queries[id]);
}

static void endQuery() {
    glEndQuery(GL_TIME_ELAPSED);
}

namespace Renderer {

    DebugRenderer debug;

    Cubemap irradiance;
    Cubemap specular;

    std::vector<std::shared_ptr<PostProcessor>> postProcessingStack;

    // public functions
    // ------------------------------------------------------------------------
    void init(bool doShadows, int shadowResolution, int resolutionX, int resolutionY) {
        resolution = vec2i(resolutionX, resolutionY);

        Resources::loadDefaultResources();
        
        shadows = doShadows;
        shadow_resolution = shadowResolution;

        glGenQueries(4, queries);

        if (shadows) {
            shadowBuffer0.setup(shadow_resolution, shadow_resolution);
            shadowBuffer1.setup(shadow_resolution, shadow_resolution);
            shadowBuffer2.setup(shadow_resolution, shadow_resolution);
            shadowBuffer3.setup(shadow_resolution, shadow_resolution);

            shadowBuffer0.attachShadow(shadow_resolution, shadow_resolution);
            shadowBuffer1.attachShadow(shadow_resolution, shadow_resolution);
            shadowBuffer2.attachShadow(shadow_resolution, shadow_resolution);
            shadowBuffer3.attachShadow(shadow_resolution, shadow_resolution);
        }

        glEnable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
        glEnable(GL_DEPTH_TEST);

        resize(resolution.x, resolution.y);
    }

    void resize(int resolutionX, int resolutionY) {
        resolution = vec2i(resolutionX, resolutionY);

        HDRbuffer.destroy();
        HDRbuffer.setup(resolution.x, resolution.y);
        HDRbuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
        HDRbuffer.attachRBO();

        HDRbuffer2.destroy();
        HDRbuffer2.setup(resolution.x, resolution.y);
        HDRbuffer2.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
        HDRbuffer2.attachRBO();

        gBuffer.destroy();
        gBuffer.setup(resolution.x, resolution.y);
        gBuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT); //position
        gBuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT); //normal
        gBuffer.attachTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); //color + specular
        gBuffer.attachTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); //roughness + metallic + 2 extra channels
        gBuffer.attachRBO();

        for (size_t i = 0; i < postProcessingStack.size(); i++) {
            postProcessingStack[i]->resize();
        }
    }

    void matchWindowResolution(float scale) {
        if (!(resolution == vec2i(Window::getWindowSize().x * scale, Window::getWindowSize().y * scale))) {
            resize(Window::getWindowSize().x * scale, Window::getWindowSize().y * scale);
        }
    }

    void renderPointLight(const vec3 &position, const vec3 &color, float radius) {
        PointLight p;
        p.position = position;
        p.color = color;
        p.radius = radius;

        pointLights.push_back(p);
    }

    void render(const IRenderable *mesh, const Material *material, const Transform *transform, const AABB *aabb, const Bone *bones) {
        RenderCall call;
        call.mesh = mesh;
        call.material = material;
        call.transform = transform;
        call.aabb = aabb;
        call.bones = bones;

        if (material->deferred) {
            renderQueue.push_back(call);
        }
        else {
            renderQueueForward.push_back(call);
        }
    }

    void render(const Model *model, const Transform *transform, const AABB *aabb) {
        for (unsigned int i = 0; i < model->nodes.size(); i++) {
            const ModelNode *node = &model->nodes[i];
            render(&node->mesh, &model->materials[node->materialIndex], transform, aabb);
        }
    }

    void renderSkybox(const Material *material) {
        render(&Resources::cubemapMesh, material, nullptr, nullptr, nullptr);
    }

    void renderGbuffers(const Camera &cam, const Frustum &f, bool doFrustumCulling) {
        // render objects in scene into g-buffer
        // -------------------------------------
        gBuffer.bind();
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, resolution.x, resolution.y);

        const Material *lastMaterial = nullptr;

        for (RenderCall call : renderQueue) {
            if (doFrustumCulling) {
                if (!f.isBoxInside(*call.aabb)) {
                    continue;
                }
            }

            Shader s = call.material->getShader();

            if (call.material != lastMaterial) {
                s.bind();
                call.material->bindUniforms();

                s.uniformMat4("view", cam.getView());
                s.uniformMat4("projection", cam.getProjection());

            }

            if (call.bones) {
                s.uniformBool("doAnimation", true);

                std::vector<mat4> skinningMatrices = call.bones->getSkinningTransforms();

                for (size_t i = 0; i < skinningMatrices.size(); i++) {
                    mat4 trans = skinningMatrices.at(i);

                    s.uniformMat4("bones["+std::to_string(i)+"]", trans);
                }
            }
            else {
                s.uniformBool("doAnimation", false);
            }

            if (call.transform) {
                s.uniformMat4("model", call.transform->getMatrix());
            }
            else {
                s.uniformMat4("model", mat4());
            }
            

            call.mesh->render();

            lastMaterial = call.material;
        }
    }

    void renderForwardPass(const Camera &cam, const Frustum &f, bool doFrustumCulling) {
        const Material *lastMaterial = nullptr;

        for (RenderCall call : renderQueueForward) {
            if (doFrustumCulling) {
                if (!f.isBoxInside(*call.aabb)) {
                    continue;
                }
            }

            Shader s = call.material->getShader();

            if (call.material != lastMaterial) {
                s.bind();
                call.material->bindUniforms();

                s.uniformMat4("view", cam.getView());
                s.uniformMat4("projection", cam.getProjection());
            }

            if (call.bones) {
                s.uniformBool("doAnimation", true);

                std::vector<mat4> skinningMatrices = call.bones->getSkinningTransforms();

                for (size_t i = 0; i < skinningMatrices.size(); i++) {
                    mat4 trans = skinningMatrices.at(i);

                    s.uniformMat4("bones["+std::to_string(i)+"]", trans);
                }
            }
            else {
                s.uniformBool("doAnimation", false);
            }

            if (call.transform) {
                s.uniformMat4("model", call.transform->getMatrix());
            }
            else {
                s.uniformMat4("model", mat4());
            }

            call.mesh->render();

            lastMaterial = call.material;
        }
    }

    void lightGbuffers(const Camera &cam) {
        mat4 shadowMatrix0 = shadowMatrix(cascadeDistances[0], cam, cascadeDepths[0]);
        mat4 shadowMatrix1 = shadowMatrix(cascadeDistances[1], cam, cascadeDepths[1]);
        mat4 shadowMatrix2 = shadowMatrix(cascadeDistances[2], cam, cascadeDepths[2]);
        mat4 shadowMatrix3 = shadowMatrix(cascadeDistances[3], cam, cascadeDepths[3]);

        Frustum shadowFrustum0 = shadowFrustum(cascadeDistances[0], cam, cascadeDepths[0]);
        Frustum shadowFrustum1 = shadowFrustum(cascadeDistances[1], cam, cascadeDepths[1]);
        Frustum shadowFrustum2 = shadowFrustum(cascadeDistances[2], cam, cascadeDepths[2]);
        Frustum shadowFrustum3 = shadowFrustum(cascadeDistances[3], cam, cascadeDepths[3]);

        beginQuery(1);

        // render scene multiple times to shadow buffers
        if (shadows) {
            renderShadow(shadowBuffer0, shadowMatrix0, shadowFrustum0, false);
            renderShadow(shadowBuffer1, shadowMatrix1, shadowFrustum1, false);
            renderShadow(shadowBuffer2, shadowMatrix2, shadowFrustum2, false);
            renderShadow(shadowBuffer3, shadowMatrix3, shadowFrustum3, false);
        }

        endQuery();
        beginQuery(2);

        HDRbuffer.bind();
        glViewport(0, 0, resolution.x, resolution.y);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_TRUE);

        gBuffer.getAttachment(0).bind(0);
        gBuffer.getAttachment(1).bind(1);
        gBuffer.getAttachment(2).bind(2);
        gBuffer.getAttachment(3).bind(3);

        mat4 inverseView = inverse(cam.getView());

        // light the g-buffers with the deferred shader
        // ---------------------------------------------
        Resources::deferredShader.bind();

        Resources::deferredShader.uniformInt("gPosition", 0);
        Resources::deferredShader.uniformInt("gNormal", 1);
        Resources::deferredShader.uniformInt("gAlbedo", 2);
        Resources::deferredShader.uniformInt("gRoughnessMetallic", 3);

        Resources::framebufferMesh.render();

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        // render directional lighting to the buffer
        // ---------------------------------------------
        Resources::deferredDirectionalShader.bind();

        Resources::deferredDirectionalShader.uniformInt("gPosition", 0);
        Resources::deferredDirectionalShader.uniformInt("gNormal", 1);
        Resources::deferredDirectionalShader.uniformInt("gAlbedo", 2);
        Resources::deferredDirectionalShader.uniformInt("gRoughnessMetallic", 3);

        Resources::deferredDirectionalShader.uniformVec3("sun.direction", vec3(vec4(sun.direction, 0.0f) * cam.getView()));
        Resources::deferredDirectionalShader.uniformVec3("sun.color", sun.color);
        Resources::deferredDirectionalShader.uniformMat4("view", cam.getView());

        if (shadows) {
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

            Resources::deferredDirectionalShader.uniformMat4("lightSpaceMatrix[0]", biasMatrix * shadowMatrix0 * inverseView);
            Resources::deferredDirectionalShader.uniformMat4("lightSpaceMatrix[1]", biasMatrix * shadowMatrix1 * inverseView);
            Resources::deferredDirectionalShader.uniformMat4("lightSpaceMatrix[2]", biasMatrix * shadowMatrix2 * inverseView);
            Resources::deferredDirectionalShader.uniformMat4("lightSpaceMatrix[3]", biasMatrix * shadowMatrix3 * inverseView);

            Resources::deferredDirectionalShader.uniformFloat("cascadeDistances[0]", cascadeDistances[0]);
            Resources::deferredDirectionalShader.uniformFloat("cascadeDistances[1]", cascadeDistances[1]);
            Resources::deferredDirectionalShader.uniformFloat("cascadeDistances[2]", cascadeDistances[2]);
            Resources::deferredDirectionalShader.uniformFloat("cascadeDistances[3]", cascadeDistances[3]);
        }
        Resources::framebufferMesh.render();

        // Render point light lighting to the buffer
        // ---------------------------------------------
        Resources::deferredPointShader.bind();

        Resources::deferredPointShader.uniformInt("gPosition", 0);
        Resources::deferredPointShader.uniformInt("gNormal", 1);
        Resources::deferredPointShader.uniformInt("gAlbedo", 2);
        Resources::deferredPointShader.uniformInt("gRoughnessMetallic", 3);

        Resources::deferredPointShader.uniformInt("pointLightCount", (int) pointLights.size());
        for (unsigned int i = 0; i < pointLights.size(); i++) {
            Resources::deferredPointShader.uniformVec3("pointLights[" + std::to_string(i) + "].position",
                                                  vec3(vec4(pointLights[i].position, 1.0f) * cam.getView()));
            Resources::deferredPointShader.uniformVec3("pointLights[" + std::to_string(i) + "].color", pointLights[i].color);

            Resources::deferredPointShader.uniformFloat("pointLights[" + std::to_string(i) + "].radius", pointLights[i].radius);
        }

        Resources::framebufferMesh.render();

        // Render ambient lighting to the buffer
        // ---------------------------------------------
        Resources::deferredAmbientShader.bind();

        Resources::deferredAmbientShader.uniformInt("gPosition", 0);
        Resources::deferredAmbientShader.uniformInt("gNormal", 1);
        Resources::deferredAmbientShader.uniformInt("gAlbedo", 2);
        Resources::deferredAmbientShader.uniformInt("gRoughnessMetallic", 3);
        

        Resources::deferredAmbientShader.uniformInt("irradiance", 4);
        irradiance.bind(4);
        Resources::deferredAmbientShader.uniformInt("prefilter", 5);
        specular.bind(5);
        Resources::deferredAmbientShader.uniformInt("brdf", 6);
        Resources::brdf.bind(6);

        Resources::deferredAmbientShader.uniformMat4("inverseView", inverseView);

        Resources::framebufferMesh.render();

        endQuery();
    }

    void flush(const Camera &cam) {
        Frustum f;
        flush(cam, f, false);
    }

    void flush(const Camera &cam, const Frustum &f, bool doFrustumCulling) {
        const Texture &t = flushToTexture(cam, f, doFrustumCulling);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Window::getWindowSize().x, Window::getWindowSize().y);
        Resources::gammaCorrectShader.bind();
        t.bind();
        Resources::framebufferMesh.render();
    }

    const Texture &flushToTexture(const Camera &cam) {
        Frustum f;
        return flushToTexture(cam, f, false);
    }

    const Texture &flushToTexture(const Camera &cam, const Frustum &f, bool doFrustumCulling) {
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        beginQuery(0);
        renderGbuffers(cam, f, doFrustumCulling);
        endQuery();
        
        lightGbuffers(cam);

        // copy depth and stencil buffer
        // -------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, HDRbuffer.fbo);
        glBlitFramebuffer(0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBlitFramebuffer(0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        renderForwardPass(cam, f, doFrustumCulling);
        
        // post processing
        beginQuery(3);
        Framebuffer &source = HDRbuffer;
        Framebuffer &destination = HDRbuffer2;

        if (postProcessingStack.size() > 0) {
            for (size_t i = 0; i < postProcessingStack.size(); i++) {
                std::shared_ptr<PostProcessor> step = postProcessingStack[i];

                step->postProcess(cam, source, destination);

                Framebuffer &temp = destination;
                destination = source;
                source = temp;
            }
        }
        else {
            destination = HDRbuffer;
        }
        endQuery();

        debug.flush(cam);

        static bool lastKeydown = false;
        static bool debug = false;
        if (Input::isKeyDown(Input::KEY_F1) && !lastKeydown) {
            debug = !debug;
        }
        lastKeydown = Input::isKeyDown(Input::KEY_F1);

        if (debug) {
            bool p_open = false;
            if (ImGui::Begin("Example: Fixed Overlay", &p_open, ImVec2(0, 0), 0.3f,
                            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                            ImGuiWindowFlags_NoSavedSettings)) {
                            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                            ImGui::GetIO().Framerate);

                            ImGui::Text("geometry: %.2f ms", queryResults[0]*0.000001f);
                            ImGui::Text("shadow pass: %.2f ms", queryResults[1]*0.000001f);
                            ImGui::Text("deferred lighting: %.2f ms", queryResults[2]*0.000001f);
                            ImGui::Text("post processing: %.2f ms", queryResults[3]*0.000001f);
                ImGui::End();
            }

            ImGui::ShowDemoWindow();
        }

        pointLights.clear();
        renderQueue.clear();
        renderQueueForward.clear();

        return destination.getAttachment(0);
    }

    Cubemap renderToProbe(const vec3 &position) {
        static const int resolution = 512;

        static vec3 forwards[] = {
                vec3(1.0f,  0.0f,  0.0f),
                vec3(-1.0f,  0.0f,  0.0f),
                vec3(0.0f,  1.0f,  0.0f),
                vec3(0.0f, -1.0f,  0.0f),
                vec3(0.0f,  0.0f,  1.0f),
                vec3(0.0f,  0.0f, -1.0f)
        };

        static vec3 ups[] = {
                vec3(0.0f,  -1.0f,  0.0f),
                vec3(0.0f,  -1.0f,  0.0f),
                vec3(0.0f,  0.0f,  1.0f),
                vec3(0.0f, 0.0f,  -1.0f),
                vec3(0.0f,  -1.0f, 0.0f),
                vec3(0.0f,  -1.0f, 0.0f)
        };

        unsigned int captureFBO, captureRBO;
        glGenFramebuffers(1, &captureFBO);
        glGenRenderbuffers(1, &captureRBO);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

        unsigned int envCubemap;

        glGenTextures(1, &envCubemap);
        glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
        for (unsigned int i = 0; i < 6; ++i)
        {
            // note that we store each face with 16 bit floating point values
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                         resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        vec2i internalResolution = getResolution();
        resize(resolution, resolution);

        glViewport(0, 0, resolution, resolution);
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
            Texture gPosition;
            Texture gNormal;
            Texture gAlbedo;
            Texture gRoughnessMetallic;

            Camera cam;
            cam.dimensions = {(float)resolution, (float)resolution};
            cam.position = position;

            cam.direction = forwards[i];
            cam.up = ups[i];
            cam.fov = 90.0f;

            renderGbuffers(cam, Frustum(), false);
            lightGbuffers(cam);

            // copy depth and stencil buffer
            // -------------------------------
            glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, HDRbuffer.fbo);
            glBlitFramebuffer(0, 0, resolution, resolution, 0, 0, resolution, resolution, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
            glBlitFramebuffer(0, 0, resolution, resolution, 0, 0, resolution, resolution, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

            glDepthFunc(GL_LEQUAL);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            renderForwardPass(cam, Frustum(), false);
            glDisable(GL_BLEND);

            glViewport(0, 0, resolution, resolution);
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
            glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Resources::passthroughShader.bind();
            HDRbuffer.getAttachment(0).bind();
            Resources::framebufferMesh.render();
        }

        resize(internalResolution.x, internalResolution.y);

        glDeleteFramebuffers(1, &captureFBO);
        glDeleteRenderbuffers(1, &captureRBO);

        Cubemap c;
        c.setID(envCubemap);

        return c;
    }

    void setSun(const DirectionalLight &light) {
        sun = light;
    }

    void setClearColor(vec3 color) {
        clearColor = color;
    }

    vec2i getResolution() {
        return resolution;
    }

    Framebuffer &getGBuffer() {
        return gBuffer;
    }
}
