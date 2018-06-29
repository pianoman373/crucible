#include <crucible/Renderer.hpp>
#include <crucible/Framebuffer.hpp>
#include <glad/glad.h>
#include <crucible/Window.hpp>
#include <crucible/Camera.hpp>
#include <crucible/AABB.hpp>
#include <crucible/Frustum.hpp>
#include <crucible/Mesh.hpp>
#include <crucible/MeshFactory.hpp>
#include <crucible/Primitives.hpp>
#include <crucible/Math.hpp>
#include <crucible/Model.hpp>
#include <crucible/InternalShaders.hpp>
#include <crucible/Util.hpp>
#include <crucible/Input.hpp>
#include <crucible/IBL.hpp>
#include <crucible/DebugRenderer.hpp>
#include <crucible/Profiler.hpp>

#include <imgui.h>
#include <stack>
#include <string>

struct RenderCall {
	IRenderable *mesh;
	Material *material;
	Transform transform;
	AABB aabb;
};

struct RenderCallSprite {
	Texture tex;
	vec2 pos;
	vec2 dimensions;
	vec4 uv;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float radius;
};

// private variables
// -----------------
static const float cascadeDistances[4] = { 10.0f, 40.0f, 100.0f, 500.0f };
static const float cascadeDepths[4] = { 500.0f, 500.0f, 500.0f, 500.0f };
static DirectionalLight sun = { normalize(vec3(-0.4f, -0.7f, -1.0f)), vec3(1.4f, 1.3f, 1.0f) * 5.0f };

static std::vector<RenderCall> renderQueue;
static std::vector<PointLight> pointLights;
static std::stack<RenderCallSprite> renderQueueSprite;
static std::vector<RenderCall> renderQueueOutline;

static Framebuffer shadowBuffer0;
static Framebuffer shadowBuffer1;
static Framebuffer shadowBuffer2;
static Framebuffer shadowBuffer3;
static Framebuffer HDRbuffer;
static Framebuffer gBuffer;

static Shader skyboxShader;
static Shader spriteShader;
static Shader ShadowShader;
static Shader deferredShader;

static Mesh skyboxMesh;
static Mesh spriteMesh;

static bool shadows;
static int shadow_resolution;
static bool outlineEnabled = false;

static vec2i resolution;

// private functions
// -----------------

static mat4 shadowMatrix(float radius, Camera &cam, float depth) {
	return orthographic(-radius, radius, -radius, radius, -depth, depth) * LookAt(cam.getPosition() - sun.direction, cam.getPosition(), vec3(0.0f, 1.0f, 0.0f));
}
// ------------------------------------------------------------------------
static Frustum shadowFrustum(float radius, Camera &cam, float depth) {
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
			if (!f.isBoxInside(c.aabb)) {
				continue;
			}
		}


		ShadowShader.bind();
		ShadowShader.uniformMat4("lightSpaceMatrix", lightSpaceMatrix);

		ShadowShader.uniformMat4("model", c.transform.getMatrix());

		c.mesh->render();
	}
}
// ------------------------------------------------------------------------
static void renderDebugGui() {
    float aspect = (float)resolution.x / (float)resolution.y;

    ImGui::Begin("frame buffers");
//	ImGui::Image(ImTextureID((long long) ssaoBuffer.getAttachment(0).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
//				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    ImGui::SameLine(300);
	ImGui::Image(ImTextureID((long long) gBuffer.getAttachment(0).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
	ImGui::Image(ImTextureID((long long) gBuffer.getAttachment(1).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    ImGui::SameLine(300);
	ImGui::Image(ImTextureID((long long) gBuffer.getAttachment(2).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));


	ImGui::Image(ImTextureID((long long) gBuffer.getAttachment(3).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    ImGui::SameLine(300);
	ImGui::Image(ImTextureID((long long) HDRbuffer.getAttachment(0).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
	ImGui::Image(ImTextureID((long long) HDRbuffer.getAttachment(1).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
    ImGui::SameLine(300);
//	ImGui::Image(ImTextureID((long long) Renderer::postProcessor.HDRbuffer2.getAttachment(0).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
//				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
	ImGui::End();

	bool p_open = false;
	if (ImGui::Begin("Example: Fixed Overlay", &p_open, ImVec2(0, 0), 0.3f,
					 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
					 ImGuiWindowFlags_NoSavedSettings)) {
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
					ImGui::GetIO().Framerate);
		ImGui::End();
	}

	ImGui::SetNextWindowPos(ImVec2(Window::getWindowSize().x - 300, 0));
    if (ImGui::Begin("Profiler", &p_open, ImVec2(300, 0), 0.3f,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::Text("Profiler");
        ImGui::Separator();

        auto times = Profiler::getValues();

        for (auto &i : times) {
            ImGui::Text("%s = %.1fms", i.first.c_str(), i.second);
        }

        ImGui::End();
    }
}

// public functions
// ---------------

namespace Renderer {
	RendererSettings settings;

	vec3 ambient = vec3(0.01f);

	DebugRenderer debug;

	Mesh cubemapMesh;
	Mesh framebufferMesh;

	Shader standardShader;
	Shader eq2cubeShader;
	Shader cubemapShader;
	Shader irradianceShader;
	Shader prefilterShader;
	Shader brdfShader;
	Shader outlineShader;
	Shader passthroughShader;

	Cubemap environment;
	Cubemap irradiance;
	Cubemap specular;

	Texture brdf;

	PostProcessor postProcessor;

	// ------------------------------------------------------------------------
	void init(bool doShadows, int shadowResolution, int resolutionX, int resolutionY) {
		resolution = vec2i(resolutionX, resolutionY);

		debug.init();

		shadows = doShadows;
		shadow_resolution = shadowResolution;


		if (shadows) {
			shadowBuffer0.setupShadow(shadow_resolution, shadow_resolution);
			shadowBuffer1.setupShadow(shadow_resolution, shadow_resolution);
			shadowBuffer2.setupShadow(shadow_resolution, shadow_resolution);
			shadowBuffer3.setupShadow(shadow_resolution, shadow_resolution);
			ShadowShader.load(InternalShaders::shadow_vsh, InternalShaders::shadow_fsh);
		}

		spriteShader.load(InternalShaders::sprite_vsh, InternalShaders::sprite_fsh);
		standardShader.load(InternalShaders::standard_vsh, InternalShaders::standard_fsh);
		eq2cubeShader.load(InternalShaders::cubemap_vsh, InternalShaders::eq2cube_fsh);
		cubemapShader.load(InternalShaders::cubemap_vsh, InternalShaders::cubemap_fsh);
		irradianceShader.load(InternalShaders::cubemap_vsh, InternalShaders::irradiance_fsh);
		prefilterShader.load(InternalShaders::cubemap_vsh, InternalShaders::prefilter_fsh);
		outlineShader.load(InternalShaders::outline_vsh, InternalShaders::outline_fsh);
		passthroughShader.loadPostProcessing(InternalShaders::passthrough_glsl);


		brdfShader.loadPostProcessing(InternalShaders::brdf_glsl);
		deferredShader.loadPostProcessing(InternalShaders::deferred_glsl);

		skyboxShader = cubemapShader;

		Primitives::skybox(skyboxMesh);
		Primitives::sprite(spriteMesh);
		Primitives::framebuffer(framebufferMesh);
		Primitives::skybox(cubemapMesh);

		glEnable(GL_CULL_FACE);
		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		//    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		// HDR framebuffer
		HDRbuffer.setup(resolution.x, resolution.y);
		HDRbuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);
		HDRbuffer.attachRBO();

		gBuffer.setup(resolution.x, resolution.y);
		gBuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT); //position
		gBuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT); //normal
		gBuffer.attachTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); //color + specular
		gBuffer.attachTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); //roughness + metallic + 2 extra channels
		gBuffer.attachRBO();

		postProcessor.init();


        // create brdf texture
        unsigned int brdfLUTTexture;
        glGenTextures(1, &brdfLUTTexture);

        // pre-allocate enough memory for the LUT texture.
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        unsigned int captureFBO, captureRBO;
        glGenFramebuffers(1, &captureFBO);
        glGenRenderbuffers(1, &captureRBO);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

        glViewport(0, 0, 512, 512);
        Renderer::brdfShader.bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Renderer::framebufferMesh.render();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDeleteFramebuffers(1, &captureFBO);
        glDeleteRenderbuffers(1, &captureRBO);

        brdf.setID(brdfLUTTexture);
	}

	// ------------------------------------------------------------------------
	void renderSkybox(mat4 view, mat4 projection, vec3 cameraPos) {
		glDepthMask(GL_TRUE);
		skyboxShader.bind();

		skyboxShader.uniformMat4("view", view);
		skyboxShader.uniformMat4("projection", projection);
		skyboxShader.uniformVec3("sun.direction", sun.direction);
		skyboxShader.uniformVec3("sun.color", sun.color);
		skyboxShader.uniformVec3("ambient", ambient);
        skyboxShader.uniformVec3("cameraPos", cameraPos);

		if (environment.getID() != 0) {
			environment.bind(0);
			skyboxShader.uniformBool("isTextured", true);
		}
		else {
			skyboxShader.uniformBool("isTextured", false);
		}




		cubemapMesh.render();
		glDepthMask(GL_TRUE);
	}

    // ------------------------------------------------------------------------
    void renderPointLight(vec3 position, vec3 color, float radius) {
        PointLight p;
        p.position = position;
        p.color = color;
        p.radius = radius;

        pointLights.push_back(p);
	}

	// ------------------------------------------------------------------------
	void render(IRenderable *mesh, Material *material, Transform transform, AABB aabb) {
		RenderCall call;
		call.mesh = mesh;
		call.material = material;
		call.transform = transform;
		call.aabb = aabb;

		if (outlineEnabled)
			renderQueueOutline.push_back(call);
		else
			renderQueue.push_back(call);
	}

	// ------------------------------------------------------------------------
	void render(Model *model, Transform transform, AABB aabb) {
		for (unsigned int i = 0; i < model->nodes.size(); i++) {
			ModelNode *node = &model->nodes[i];
			render(&node->mesh, &model->materials[node->materialIndex], transform, aabb);
		}
	}

	// ------------------------------------------------------------------------
	void renderSprite(Texture tex, vec2 pos, vec2 dimensions, vec4 uv) {
		RenderCallSprite call;
		call.tex = tex;
		call.pos = pos;
		call.dimensions = dimensions;
		call.uv = uv;

		renderQueueSprite.push(call);
	}

	// ------------------------------------------------------------------------
	void enableOutline() {
		outlineEnabled = true;
	}

	// ------------------------------------------------------------------------
	void disableOutline() {
		outlineEnabled = false;
	}

	void flush(Camera cam) {
		Frustum f;
		flush(cam, f, false);
	}

	// ------------------------------------------------------------------------
	void flush(Camera cam, Frustum f, bool doFrustumCulling) {
		glStencilMask(0x00);

		mat4 lightSpaceMatrix0 = shadowMatrix(cascadeDistances[0], cam, cascadeDepths[0]);
		mat4 lightSpaceMatrix1 = shadowMatrix(cascadeDistances[1], cam, cascadeDepths[1]);
		mat4 lightSpaceMatrix2 = shadowMatrix(cascadeDistances[2], cam, cascadeDepths[2]);
		mat4 lightSpaceMatrix3 = shadowMatrix(cascadeDistances[3], cam, cascadeDepths[3]);

		Frustum shadowFrustum0 = shadowFrustum(cascadeDistances[0], cam, cascadeDepths[0]);
		Frustum shadowFrustum1 = shadowFrustum(cascadeDistances[1], cam, cascadeDepths[1]);
		Frustum shadowFrustum2 = shadowFrustum(cascadeDistances[2], cam, cascadeDepths[2]);
		Frustum shadowFrustum3 = shadowFrustum(cascadeDistances[3], cam, cascadeDepths[3]);

		// render scene multiple times to shadow buffers
		if (shadows) {
			glDisable(GL_CULL_FACE);
			renderShadow(shadowBuffer0, lightSpaceMatrix0, shadowFrustum0, doFrustumCulling);
			renderShadow(shadowBuffer1, lightSpaceMatrix1, shadowFrustum1, doFrustumCulling);
			renderShadow(shadowBuffer2, lightSpaceMatrix2, shadowFrustum2, doFrustumCulling);
			renderShadow(shadowBuffer3, lightSpaceMatrix3, shadowFrustum3, doFrustumCulling);
			glEnable(GL_CULL_FACE);
		}

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);

		// render objects in scene into g-buffer
		// -------------------------------------
		gBuffer.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glViewport(0, 0, resolution.x, resolution.y);

		Material *lastMaterial = nullptr;


		glStencilMask(0x00);

		for (RenderCall call : renderQueue) {
			if (doFrustumCulling) {
				if (!f.isBoxInside(call.aabb)) {
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

			s.uniformMat4("model", call.transform.getMatrix());

			call.mesh->render();

			lastMaterial = call.material;
		}

		glStencilMask(0xFF);
		for (RenderCall call : renderQueueOutline) {

			Shader s = call.material->getShader();

			if (call.material != lastMaterial) {
				s.bind();
				call.material->bindUniforms();

				s.uniformMat4("view", cam.getView());
				s.uniformMat4("projection", cam.getProjection());

			}

			s.uniformMat4("model", call.transform.getMatrix());

			call.mesh->render();

			lastMaterial = call.material;
		}


		HDRbuffer.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		HDRbuffer.bind();
		// render the g-buffers with the deferred shader
		// ---------------------------------------------
		deferredShader.bind();

		deferredShader.uniformInt("gPosition", 0);
		gBuffer.getAttachment(0).bind(0);

		deferredShader.uniformInt("gNormal", 1);
		gBuffer.getAttachment(1).bind(1);

		deferredShader.uniformInt("gAlbedo", 2);
		gBuffer.getAttachment(2).bind(2);

		deferredShader.uniformInt("gRoughnessMetallic", 3);
		gBuffer.getAttachment(3).bind(3);

		deferredShader.uniformInt("irradiance", 4);
		irradiance.bind(4);
		deferredShader.uniformInt("prefilter", 5);
		specular.bind(5);
		deferredShader.uniformInt("brdf", 6);
		brdf.bind(6);

        if (irradiance.getID() != 0 && specular.getID() != 0 && brdf.getID() != 0) {
            deferredShader.uniformBool("doIBL", true);
        }
        else {
            deferredShader.uniformBool("doIBL", false);
        }

		deferredShader.uniformVec3("cameraPos", vec3());
		deferredShader.uniformVec3("sun.direction", vec3(vec4(sun.direction, 0.0f) * cam.getView()));
		deferredShader.uniformVec3("sun.color", sun.color);
		deferredShader.uniformVec3("ambient", ambient);
		deferredShader.uniformMat4("view", cam.getView());

		deferredShader.uniformInt("pointLightCount", (int) pointLights.size());
		for (unsigned int i = 0; i < pointLights.size(); i++) {
			deferredShader.uniformVec3("pointLights[" + std::to_string(i) + "].position",
									   vec3(vec4(pointLights[i].position, 1.0f) * cam.getView()));
			deferredShader.uniformVec3("pointLights[" + std::to_string(i) + "].color", pointLights[i].color);

            deferredShader.uniformFloat("pointLights[" + std::to_string(i) + "].radius", pointLights[i].radius);
		}

		if (shadows) {
			shadowBuffer0.bindTexture(8);
			shadowBuffer1.bindTexture(9);
			shadowBuffer2.bindTexture(10);
			shadowBuffer3.bindTexture(11);

			deferredShader.uniformInt("shadowTextures[0]", 8);
			deferredShader.uniformInt("shadowTextures[1]", 9);
			deferredShader.uniformInt("shadowTextures[2]", 10);
			deferredShader.uniformInt("shadowTextures[3]", 11);

			deferredShader.uniformMat4("lightSpaceMatrix[0]", lightSpaceMatrix0);
			deferredShader.uniformMat4("lightSpaceMatrix[1]", lightSpaceMatrix1);
			deferredShader.uniformMat4("lightSpaceMatrix[2]", lightSpaceMatrix2);
			deferredShader.uniformMat4("lightSpaceMatrix[3]", lightSpaceMatrix3);

			deferredShader.uniformFloat("cascadeDistances[0]", cascadeDistances[0]);
			deferredShader.uniformFloat("cascadeDistances[1]", cascadeDistances[1]);
			deferredShader.uniformFloat("cascadeDistances[2]", cascadeDistances[2]);
			deferredShader.uniformFloat("cascadeDistances[3]", cascadeDistances[3]);
		}
		framebufferMesh.render();

		// render the skybox
		// -----------------
		renderSkybox(cam.getView(), cam.getProjection(), cam.getPosition());

		Texture final = postProcessor.postRender(cam, HDRbuffer.getAttachment(0), gBuffer.getAttachment(0), gBuffer.getAttachment(1), gBuffer.getAttachment(2), gBuffer.getAttachment(3));

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, Window::getWindowSize().x, Window::getWindowSize().y);
		passthroughShader.bind();
		final.bind();
		framebufferMesh.render();

		// render any sprites
		// ------------------
		glDepthMask(GL_FALSE);
		for (unsigned int i = 0; i < renderQueueSprite.size(); i++) {
			RenderCallSprite call = renderQueueSprite.top();
			renderQueueSprite.pop();

			spriteShader.bind();

			call.tex.bind(0);

			spriteShader.uniformVec4("uvOffsets", call.uv);

			mat4 model;
			model = translate(model, vec3(call.pos.x, call.pos.y, 0.0f));
			model = scale(model, vec3(call.dimensions.x, call.dimensions.y, 0.0f));


			spriteShader.uniformMat4("model", model);
			spriteShader.uniformMat4("view", cam.getView());
			spriteShader.uniformMat4("projection", cam.getProjection());
			spriteShader.uniformVec3("cameraPos", cam.getPosition());

			spriteMesh.render();
		}
		glDepthMask(GL_TRUE);

		// copy depth and stencil buffer
		// -------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBlitFramebuffer(0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, GL_STENCIL_BUFFER_BIT, GL_NEAREST);


		// render object outlines
		// ----------------------
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilMask(0x00);

		for (RenderCall call : renderQueueOutline) {

			Shader s = outlineShader;

			s.bind();
			call.material->bindUniforms();

			s.uniformMat4("view", cam.getView());
			s.uniformMat4("projection", cam.getProjection());
			s.uniformMat4("model", call.transform.getMatrix());
			s.uniformVec3("color", vec3(1.0f, 0.5f, 0.0f));
			s.uniformFloat("thickness", 0.005);

			call.mesh->render();
		}
		glStencilFunc(GL_ALWAYS, 1, 0xFF);


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
		renderQueueOutline.clear();
	}

	// ------------------------------------------------------------------------
	void setSkyboxShader(Shader s) {
		skyboxShader = s;
	}

	// ------------------------------------------------------------------------
	void setSun(DirectionalLight light) {
		sun = light;
	}

	vec2i getResolution() {
		return resolution;
	}
}