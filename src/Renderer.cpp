#include <crucible/Renderer.hpp>
#include <crucible/Framebuffer.hpp>
#include <glad/glad.h>
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

#include <imgui.h>
#include <stack>
#include <string>

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
static std::vector<PointLight> pointLights;

static Framebuffer shadowBuffer0;
static Framebuffer shadowBuffer1;
static Framebuffer shadowBuffer2;
static Framebuffer shadowBuffer3;
static Framebuffer HDRbuffer;
static Framebuffer gBuffer;


static bool shadows;
static int shadow_resolution;

static vec2i resolution;

// private functions
// -----------------

static mat4 shadowMatrix(float radius, const Camera &cam, float depth) {
	return orthographic(-radius, radius, -radius, radius, -depth, depth) * LookAt(cam.getPosition() - sun.direction, cam.getPosition(), vec3(0.0f, 1.0f, 0.0f));
}
// ------------------------------------------------------------------------
static Frustum shadowFrustum(float radius, const Camera &cam, float depth) {
	Frustum shadowFrustum;
	shadowFrustum.setupInternalsOrthographic(-radius, radius, -radius, radius, -depth, depth);
	Camera shadowCam;
	shadowCam.setPosition(cam.getPosition());
	shadowCam.setDirection(sun.direction);
	shadowFrustum.updateCamPosition(shadowCam);



	return shadowFrustum;
}
// ------------------------------------------------------------------------
static void renderShadow(Framebuffer &fbuffer, mat4 lightSpaceMatrix, Frustum f, bool doFrustumCulling) {
	glViewport(0, 0, fbuffer.getWidth(), fbuffer.getHeight());
	fbuffer.bind();
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

            for (int i = 0; i < skinningMatrices.size(); i++) {
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

// ------------------------------------------------------------------------
static void renderDebugGui() {
	float aspect = (float)resolution.x / (float)resolution.y;

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


namespace Renderer {
    vec3 ambient = vec3(0.01f);

    DebugRenderer debug;

    Cubemap environment;
    Cubemap irradiance;
    Cubemap specular;

    PostProcessor postProcessor;


    // public functions
    // ------------------------------------------------------------------------
    void init(bool doShadows, int shadowResolution, int resolutionX, int resolutionY) {
        resolution = vec2i(resolutionX, resolutionY);

        Resources::loadDefaultResources();

        debug.init();

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
        glEnable(GL_STENCIL_TEST);
        //    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

        resize(resolution.x, resolution.y);
        postProcessor.init();
    }

    void resize(int resolutionX, int resolutionY) {
        resolution = vec2i(resolutionX, resolutionY);

        HDRbuffer.destroy();
        HDRbuffer.setup(resolution.x, resolution.y);
        HDRbuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
        HDRbuffer.attachRBO();

        gBuffer.destroy();
        gBuffer.setup(resolution.x, resolution.y);
        gBuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT); //position
        gBuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT); //normal
        gBuffer.attachTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); //color + specular
        gBuffer.attachTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); //roughness + metallic + 2 extra channels
        gBuffer.attachRBO();

        postProcessor.resize();
    }

    void matchWindowResolution(float scale) {
        if (!(resolution == vec2i(Window::getWindowSize().x * scale, Window::getWindowSize().y * scale))) {
            resize(Window::getWindowSize().x * scale, Window::getWindowSize().y * scale);
        }
    }


    // ------------------------------------------------------------------------
    void renderSkybox(const mat4 &view, const mat4 &projection, const vec3 &cameraPos) {
        Resources::cubemapShader.bind();

        Resources::cubemapShader.uniformMat4("view", view);
        Resources::cubemapShader.uniformMat4("projection", projection);
        Resources::cubemapShader.uniformVec3("sun.direction", sun.direction);
        Resources::cubemapShader.uniformVec3("sun.color", sun.color);
        Resources::cubemapShader.uniformVec3("ambient", ambient);
        Resources::cubemapShader.uniformVec3("cameraPos", cameraPos);

        if (environment.getID() != 0) {
            environment.bind(0);
            Resources::cubemapShader.uniformBool("isTextured", true);
        }
        else {
            Resources::cubemapShader.uniformBool("isTextured", false);
        }

        Resources::cubemapMesh.render();
    }

    // ------------------------------------------------------------------------
    void renderPointLight(const vec3 &position, const vec3 &color, float radius) {
        PointLight p;
        p.position = position;
        p.color = color;
        p.radius = radius;

        pointLights.push_back(p);
    }

    // ------------------------------------------------------------------------
    void render(const IRenderable *mesh, const Material *material, const Transform *transform, const AABB *aabb, const Bone *bones) {
        RenderCall call;
        call.mesh = mesh;
        call.material = material;
        call.transform = transform;
        call.aabb = aabb;
        call.bones = bones;

        renderQueue.push_back(call);
    }

    // ------------------------------------------------------------------------
    void render(const Model *model, const Transform *transform, const AABB *aabb) {
        for (unsigned int i = 0; i < model->nodes.size(); i++) {
            const ModelNode *node = &model->nodes[i];
            render(&node->mesh, &model->materials[node->materialIndex], transform, aabb);
        }
    }

    void renderGbuffers(const Camera &cam, const Frustum &f, bool doFrustumCulling, Texture &gPosition,
                                  Texture &gNormal, Texture &gAlbedo, Texture &gRoughnessMetallic) {
        // render objects in scene into g-buffer
        // -------------------------------------
        gBuffer.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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

                for (int i = 0; i < skinningMatrices.size(); i++) {
                    mat4 trans = skinningMatrices.at(i);

                    s.uniformMat4("bones["+std::to_string(i)+"]", trans);
                }
            }
            else {
                s.uniformBool("doAnimation", false);
            }

            s.uniformMat4("model", call.transform->getMatrix());

            call.mesh->render();

            lastMaterial = call.material;
        }

        gPosition = gBuffer.getAttachment(0);
        gNormal = gBuffer.getAttachment(1);
        gAlbedo = gBuffer.getAttachment(2);
        gRoughnessMetallic = gBuffer.getAttachment(3);
    }

    Texture lightGbuffers(const Camera &cam, const Texture &gPosition, const Texture &gNormal,
                                    const Texture &gAlbedo, const Texture &gRoughnessMetallic) {

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
            //glDisable(GL_CULL_FACE);
            //glCullFace(GL_FRONT);
            renderShadow(shadowBuffer0, shadowMatrix0, shadowFrustum0, false);
            renderShadow(shadowBuffer1, shadowMatrix1, shadowFrustum1, false);
            renderShadow(shadowBuffer2, shadowMatrix2, shadowFrustum2, false);
            renderShadow(shadowBuffer3, shadowMatrix3, shadowFrustum3, false);
            //glCullFace(GL_BACK);
            //glEnable(GL_CULL_FACE);
        }

        endQuery();

        beginQuery(2);

        HDRbuffer.bind();
        glViewport(0, 0, resolution.x, resolution.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDepthFunc(GL_ALWAYS);
        glDepthMask(GL_TRUE);

        // render the g-buffers with the deferred shader
        // ---------------------------------------------
        Resources::deferredShader.bind();

        Resources::deferredShader.uniformInt("gPosition", 0);
        gPosition.bind(0);

        Resources::deferredShader.uniformInt("gNormal", 1);
        gNormal.bind(1);

        Resources::deferredShader.uniformInt("gAlbedo", 2);
        gAlbedo.bind(2);

        Resources::deferredShader.uniformInt("gRoughnessMetallic", 3);
        gRoughnessMetallic.bind(3);

        Resources::deferredShader.uniformVec3("sun.direction", vec3(vec4(sun.direction, 0.0f) * cam.getView()));
        Resources::deferredShader.uniformVec3("sun.color", sun.color);
        Resources::deferredShader.uniformMat4("view", cam.getView());

        if (shadows) {
            shadowBuffer0.getAttachment(0).bind(8);
            shadowBuffer1.getAttachment(0).bind(9);
            shadowBuffer2.getAttachment(0).bind(10);
            shadowBuffer3.getAttachment(0).bind(11);

            Resources::deferredShader.uniformInt("shadowTextures[0]", 8);
            Resources::deferredShader.uniformInt("shadowTextures[1]", 9);
            Resources::deferredShader.uniformInt("shadowTextures[2]", 10);
            Resources::deferredShader.uniformInt("shadowTextures[3]", 11);

            Resources::deferredShader.uniformMat4("lightSpaceMatrix[0]", shadowMatrix0);
            Resources::deferredShader.uniformMat4("lightSpaceMatrix[1]", shadowMatrix1);
            Resources::deferredShader.uniformMat4("lightSpaceMatrix[2]", shadowMatrix2);
            Resources::deferredShader.uniformMat4("lightSpaceMatrix[3]", shadowMatrix3);

            Resources::deferredShader.uniformFloat("cascadeDistances[0]", cascadeDistances[0]);
            Resources::deferredShader.uniformFloat("cascadeDistances[1]", cascadeDistances[1]);
            Resources::deferredShader.uniformFloat("cascadeDistances[2]", cascadeDistances[2]);
            Resources::deferredShader.uniformFloat("cascadeDistances[3]", cascadeDistances[3]);
        }
        Resources::framebufferMesh.render();


        // Render point light lighting to the buffer
        // ---------------------------------------------
        Resources::deferredPointShader.bind();

        Resources::deferredPointShader.uniformInt("gPosition", 0);
        gBuffer.getAttachment(0).bind(0);

        Resources::deferredPointShader.uniformInt("gNormal", 1);
        gBuffer.getAttachment(1).bind(1);

        Resources::deferredPointShader.uniformInt("gAlbedo", 2);
        gBuffer.getAttachment(2).bind(2);

        Resources::deferredPointShader.uniformInt("gRoughnessMetallic", 3);
        gBuffer.getAttachment(3).bind(3);

        Resources::deferredPointShader.uniformInt("irradiance", 4);
        irradiance.bind(4);
        Resources::deferredPointShader.uniformInt("prefilter", 5);
        specular.bind(5);
        Resources::deferredPointShader.uniformInt("brdf", 6);
        Resources::brdf.bind(6);

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
        gBuffer.getAttachment(0).bind(0);

        Resources::deferredAmbientShader.uniformInt("gNormal", 1);
        gBuffer.getAttachment(1).bind(1);

        Resources::deferredAmbientShader.uniformInt("gAlbedo", 2);
        gBuffer.getAttachment(2).bind(2);

        Resources::deferredAmbientShader.uniformInt("gRoughnessMetallic", 3);
        gBuffer.getAttachment(3).bind(3);

        Resources::deferredAmbientShader.uniformInt("irradiance", 4);
        irradiance.bind(4);
        Resources::deferredAmbientShader.uniformInt("prefilter", 5);
        specular.bind(5);
        Resources::deferredAmbientShader.uniformInt("brdf", 6);
        Resources::brdf.bind(6);

        if (irradiance.getID() != 0 && specular.getID() != 0) {
            Resources::deferredAmbientShader.uniformBool("doIBL", true);
        }
        else {
            Resources::deferredAmbientShader.uniformBool("doIBL", false);
        }

        Resources::deferredAmbientShader.uniformVec3("cameraPos", vec3());
        Resources::deferredAmbientShader.uniformVec3("ambient", ambient);
        Resources::deferredAmbientShader.uniformMat4("view", cam.getView());

        Resources::framebufferMesh.render();

        endQuery();

        glDisable(GL_BLEND);
        glDepthFunc(GL_LEQUAL);

        // render the skybox
        // -----------------
        renderSkybox(cam.getView(), cam.getProjection(), cam.getPosition());


        glDepthFunc(GL_LEQUAL);

        return HDRbuffer.getAttachment(0);
    }

    void flush(const Camera &cam) {
        Frustum f;
        flush(cam, f, false);
    }

    void flush(const Camera &cam, const Frustum &f, bool doFrustumCulling) {

        const Texture &t = flushToTexture(cam, f, doFrustumCulling);


        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Window::getWindowSize().x, Window::getWindowSize().y);
        Resources::passthroughShader.bind();
        t.bind();
        Resources::framebufferMesh.render();
    }

    const Texture &flushToTexture(const Camera &cam) {
        Frustum f;
        return flushToTexture(cam, f, false);
    }

    // ------------------------------------------------------------------------
    const Texture &flushToTexture(const Camera &cam, const Frustum &f, bool doFrustumCulling) {
        
        


        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        Texture gPosition;
        Texture gNormal;
        Texture gAlbedo;
        Texture gRoughnessMetallic;

        beginQuery(0);
        renderGbuffers(cam, f, doFrustumCulling, gPosition, gNormal, gAlbedo, gRoughnessMetallic);
        endQuery();


        
        Texture deferred = lightGbuffers(cam, gPosition, gNormal, gAlbedo, gRoughnessMetallic);

        beginQuery(3);
        Texture final = postProcessor.postRender(cam, deferred, gPosition, gNormal, gAlbedo, gRoughnessMetallic);
        endQuery();


        HDRbuffer.bind();
        Resources::passthroughShader.bind();
        final.bind();
        Resources::framebufferMesh.render();

        // copy depth and stencil buffer
        // -------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, HDRbuffer.fbo);
        glBlitFramebuffer(0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBlitFramebuffer(0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

        // render debug tools
        // ------------------
        debug.flush(cam);



        static bool lastKeydown = false;
        static bool debug = false;
        if (Input::isKeyDown(Input::KEY_F1) && !lastKeydown) {
            debug = !debug;
        }
        lastKeydown = Input::isKeyDown(Input::KEY_F1);

        if (debug) {
            renderDebugGui();
        }

        pointLights.clear();
        renderQueue.clear();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, Window::getWindowSize().x, Window::getWindowSize().y);

        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        return HDRbuffer.getAttachment(0);
    }
    // ------------------------------------------------------------------------
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

        // convert HDR equirectangular environment map to cubemap equivalent

        vec2i internalResolution = getResolution();
        resize(resolution, resolution);

        glViewport(0, 0, resolution, resolution); // don't forget to configure the viewport to the capture dimensions.
        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        for (unsigned int i = 0; i < 6; ++i)
        {
            //eq2cubeShader.uniformMat4("view", captureViews[i]);
            //cubemapMesh.render(); // renders a 1x1 cube

            //renderSkybox(captureViews[i], captureProjection);

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

            renderGbuffers(cam, Frustum(), false, gPosition, gNormal, gAlbedo, gRoughnessMetallic);
            Texture deferred = lightGbuffers(cam, gPosition, gNormal, gAlbedo, gRoughnessMetallic);

            glViewport(0, 0, resolution, resolution); // don't forget to configure the viewport to the capture dimensions.
            glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            Resources::passthroughShader.bind();
            deferred.bind();
            Resources::framebufferMesh.render();


        }
        resize(internalResolution.x, internalResolution.y);

        glDeleteFramebuffers(1, &captureFBO);
        glDeleteRenderbuffers(1, &captureRBO);

        Cubemap c;
        c.setID(envCubemap);

        return c;
    }

    // ------------------------------------------------------------------------
    void setSkyboxShader(const Shader &s) {
        //skyboxShader = s;
    }

    // ------------------------------------------------------------------------
    void setSun(const DirectionalLight &light) {
        sun = light;
    }

    vec2i getResolution() {
        return resolution;
    }
}
