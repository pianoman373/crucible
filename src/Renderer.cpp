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
#include <random>

struct RenderCall {
	Mesh *mesh;
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

// private variables
// -----------------
static const float cascadeDistances[4] = { 10.0f, 40.0f, 100.0f, 500.0f };
static const float cascadeDepths[4] = { 500.0f, 500.0f, 500.0f, 500.0f };
static DirectionalLight sun = { normalize(vec3(-0.4f, -0.7f, -1.0f)), vec3(1.4f, 1.3f, 1.0f) * 5.0f };

static std::vector<RenderCall> renderQueue;
static std::stack<RenderCallSprite> renderQueueSprite;
static std::vector<RenderCall> renderQueueOutline;

static std::map<unsigned int, Texture> m_Textures;

static Framebuffer shadowBuffer0;
static Framebuffer shadowBuffer1;
static Framebuffer shadowBuffer2;
static Framebuffer shadowBuffer3;
static Framebuffer HDRbuffer;
static Framebuffer HDRbuffer2;
static Framebuffer gBuffer;
static Framebuffer ssaoBuffer;
static Framebuffer bloomBuffer0;
static Framebuffer bloomBuffer1;
static Framebuffer bloomBuffer2;
static Framebuffer bloomBuffer3;
static Framebuffer bloomBuffer4;
static Framebuffer bloomBuffer5;

static Shader skyboxShader;
static Shader spriteShader;
static Shader ShadowShader;
static Shader deferredShader;
static Shader tonemapShader;
static Shader fxaaShader;
static Shader gaussianBlurShader;
static Shader ssaoShader;

static Mesh skyboxMesh;
static Mesh crosshairMesh;
static Mesh spriteMesh;

static bool shadows;
static int shadow_resolution;
static bool outlineEnabled = false;

static vec2i resolution;

static std::vector<vec3> ssaoKernel;
static Texture noiseTex;

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
static void renderShadow(Framebuffer &fbuffer, mat4 lightSpaceMatrix, Frustum f) {
	glViewport(0, 0, fbuffer.getWidth(), fbuffer.getHeight());
	fbuffer.bind();
	glClear(GL_DEPTH_BUFFER_BIT);


	for (RenderCall c : renderQueue) {

		if (f.isBoxInside(c.aabb) || true) {

			ShadowShader.bind();
			ShadowShader.uniformMat4("lightSpaceMatrix", lightSpaceMatrix);

			ShadowShader.uniformMat4("model", c.transform.getMatrix());

			c.mesh->render();
		}
	}
}
// ------------------------------------------------------------------------
static void renderDebugGui() {
    float aspect = (float)resolution.x / (float)resolution.y;

    ImGui::Begin("frame buffers");
	ImGui::Image(ImTextureID((long long) ssaoBuffer.getAttachment(0).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
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
	ImGui::Image(ImTextureID((long long) HDRbuffer2.getAttachment(0).getID()), ImVec2(256, 256 / aspect), ImVec2(0, 1),
				 ImVec2(1, 0), ImColor(255, 255, 255, 255), ImColor(255, 255, 255, 128));
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
// ------------------------------------------------------------------------
static void doBloom() {
	glViewport(0, 0, bloomBuffer0.getWidth(), bloomBuffer0.getHeight());
	bloomBuffer1.bind();
	Renderer::passthroughShader.bind();
	HDRbuffer.getAttachment(1).bind();
	Renderer::framebufferMesh.render();

	bloomBuffer0.bind();
	gaussianBlurShader.bind();
	gaussianBlurShader.uniformBool("horizontal", true);
	bloomBuffer1.getAttachment(0).bind();
	Renderer::framebufferMesh.render();

	bloomBuffer1.bind();
	gaussianBlurShader.bind();
	gaussianBlurShader.uniformBool("horizontal", false);
	bloomBuffer0.getAttachment(0).bind();
	Renderer::framebufferMesh.render();

	// -------------------------------------------------------------------------

	glViewport(0, 0, bloomBuffer3.getWidth(), bloomBuffer3.getHeight());
	bloomBuffer3.bind();
	Renderer::passthroughShader.bind();
	bloomBuffer1.getAttachment(0).bind();
	Renderer::framebufferMesh.render();

	bloomBuffer2.bind();
	gaussianBlurShader.bind();
	gaussianBlurShader.uniformBool("horizontal", true);
	bloomBuffer3.getAttachment(0).bind();
	Renderer::framebufferMesh.render();

	bloomBuffer3.bind();
	gaussianBlurShader.bind();
	gaussianBlurShader.uniformBool("horizontal", false);
	bloomBuffer2.getAttachment(0).bind();
	Renderer::framebufferMesh.render();

	// -------------------------------------------------------------------------

	glViewport(0, 0, bloomBuffer5.getWidth(), bloomBuffer5.getHeight());
	bloomBuffer5.bind();
	Renderer::passthroughShader.bind();
	bloomBuffer3.getAttachment(0).bind();
	Renderer::framebufferMesh.render();

	bloomBuffer4.bind();
	gaussianBlurShader.bind();
	gaussianBlurShader.uniformBool("horizontal", true);
	bloomBuffer5.getAttachment(0).bind();
	Renderer::framebufferMesh.render();

	bloomBuffer5.bind();
	gaussianBlurShader.bind();
	gaussianBlurShader.uniformBool("horizontal", false);
	bloomBuffer4.getAttachment(0).bind();
	Renderer::framebufferMesh.render();
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
	unsigned int brdf;

	std::vector<PointLight> pointLights;
	std::vector<DirectionalLight> directionalLights;

	Texture getTexture(std::string path) {
		unsigned int id = SID(path);

		// if texture already exists, return that handle
		if (m_Textures.find(id) != m_Textures.end())
			return m_Textures[id];


		Texture texture;
		texture.load(path.c_str());

		std::cout << "loading texture: " << path << std::endl;
		m_Textures[id] = texture;

		return texture;
	}

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

		tonemapShader.loadPostProcessing(InternalShaders::tonemap_glsl);
		fxaaShader.loadPostProcessing(InternalShaders::fxaa_glsl);
		brdfShader.loadPostProcessing(InternalShaders::brdf_glsl);
		deferredShader.loadPostProcessing(InternalShaders::deferred_glsl);
		gaussianBlurShader.loadPostProcessing(InternalShaders::gaussianBlur_glsl);
		ssaoShader.loadPostProcessing(InternalShaders::ssao_glsl);

		skyboxShader = cubemapShader;

		Primitives::skybox(skyboxMesh);
		Primitives::sprite(spriteMesh);
		Primitives::framebuffer(framebufferMesh);
		Primitives::skybox(cubemapMesh);

		glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		//    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		// HDR framebuffer
		HDRbuffer.setup(resolution.x, resolution.y);
		HDRbuffer.attachTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
		HDRbuffer.attachTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT); // bright pixels
		HDRbuffer.attachRBO();

		// HDR framebuffer 2
		HDRbuffer2.setup(resolution.x, resolution.y);
		HDRbuffer2.attachTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
		//HDRbuffer2.attachRBO();

		gBuffer.setup(resolution.x, resolution.y);
		gBuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT); //position
		gBuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT); //normal
		gBuffer.attachTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); //color + specular
		gBuffer.attachTexture(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE); //roughness + metallic + 2 extra channels
		gBuffer.attachRBO();

		ssaoBuffer.setup(resolution.x, resolution.y);
		ssaoBuffer.attachTexture(GL_RGB16F, GL_RGB, GL_FLOAT);

		//bloom framebuffers
		bloomBuffer0.setup(resolution.x/2, resolution.y/2);
		bloomBuffer0.attachTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
		bloomBuffer1.setup(resolution.x/2, resolution.y/2);
		bloomBuffer1.attachTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);

		bloomBuffer2.setup(resolution.x/8, resolution.y/8);
		bloomBuffer2.attachTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
		bloomBuffer3.setup(resolution.x/8, resolution.y/8);
		bloomBuffer3.attachTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);

		bloomBuffer4.setup(resolution.x/32, resolution.y/32);
		bloomBuffer4.attachTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);
		bloomBuffer5.setup(resolution.x/32, resolution.y/32);
		bloomBuffer5.attachTexture(GL_RGBA16F, GL_RGBA, GL_FLOAT);


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
	}

	// ------------------------------------------------------------------------
	void renderSkybox(mat4 view, mat4 projection) {
		glDepthMask(GL_TRUE);
		skyboxShader.bind();

		skyboxShader.uniformMat4("view", view);
		skyboxShader.uniformMat4("projection", projection);
		skyboxShader.uniformVec3("sun.direction", sun.direction);
		skyboxShader.uniformVec3("sun.color", sun.color);
		skyboxShader.uniformVec3("ambient", ambient);
		deferredShader.uniformFloat("bloomStrength", settings.bloomStrength);

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
	void render(Mesh *mesh, Material *material, Transform transform, AABB aabb) {
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
		for (unsigned int i = 0; i < model->meshes.size(); i++) {
			render(&model->meshes[i], &model->materials[i], transform, aabb);
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

	// ------------------------------------------------------------------------
	void flush(Camera cam) {
	    Profiler::begin("flush time");


	    Profiler::begin("shadow pass");
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
			renderShadow(shadowBuffer0, lightSpaceMatrix0, shadowFrustum0);
			renderShadow(shadowBuffer1, lightSpaceMatrix1, shadowFrustum1);
			renderShadow(shadowBuffer2, lightSpaceMatrix2, shadowFrustum2);
			renderShadow(shadowBuffer3, lightSpaceMatrix3, shadowFrustum3);
		}

		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);
		Profiler::end();


		Profiler::begin("object rendering");
		// render objects in scene into g-buffer
		// -------------------------------------
		gBuffer.bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glViewport(0, 0, resolution.x, resolution.y);

		Material *lastMaterial = nullptr;


		glStencilMask(0x00);

		for (RenderCall call : renderQueue) {

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

		if (settings.ssao) {
            // render the g-buffers for SSAO
            // ---------------------------------------------
            ssaoBuffer.bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ssaoShader.bind();

            ssaoShader.uniformInt("gPosition", 0);
            gBuffer.getAttachment(0).bind(0);

            ssaoShader.uniformInt("gNormal", 1);
            gBuffer.getAttachment(1).bind(1);

            ssaoShader.uniformInt("texNoise", 2);
            noiseTex.bind(2);

            ssaoShader.uniformMat4("projection", cam.getProjection());
            ssaoShader.uniformFloat("radius", settings.ssaoRadius);
            ssaoShader.uniformInt("kernelSize", settings.ssaoKernelSize);

            for (int i = 0; i < settings.ssaoKernelSize; i++) {
                ssaoShader.uniformVec3(std::string("samples[") + std::to_string(i) + std::string("]"), ssaoKernel[i]);
            }
            framebufferMesh.render();
		}


		Profiler::end();


		Profiler::begin("deferred pass");
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

        deferredShader.uniformInt("ssaoTex", 4);
        ssaoBuffer.getAttachment(0).bind(4);

		deferredShader.uniformInt("irradiance", 5);
		irradiance.bind(5);
		deferredShader.uniformInt("prefilter", 6);
		specular.bind(6);
		deferredShader.uniformInt("brdf", 7);
		glActiveTexture(GL_TEXTURE7);
		glBindTexture(GL_TEXTURE_2D, brdf);

        if (irradiance.getID() != 0 && specular.getID() != 0 && brdf != 0) {
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
		deferredShader.uniformBool("ssaoEnabled", settings.ssao);
		deferredShader.uniformFloat("bloomStrength", settings.bloomStrength);

		deferredShader.uniformInt("pointLightCount", (int) pointLights.size());
		for (unsigned int i = 0; i < pointLights.size(); i++) {
			deferredShader.uniformVec3("pointLights[" + std::to_string(i) + "].position",
									   vec3(vec4(pointLights[i].position, 1.0f) * cam.getView()));
			deferredShader.uniformVec3("pointLights[" + std::to_string(i) + "].color", pointLights[i].color);
		}

		deferredShader.uniformInt("directionalLightCount", (int) directionalLights.size());
		for (unsigned int i = 0; i < directionalLights.size(); i++) {
			deferredShader.uniformVec3("directionalLights[" + std::to_string(i) + "].direction",
									   vec3(vec4(directionalLights[i].direction, 0.0f) * cam.getView()));
			deferredShader.uniformVec3("directionalLights[" + std::to_string(i) + "].color",
									   directionalLights[i].color);
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

		Profiler::end();

		// render the skybox
		// -----------------
		renderSkybox(cam.getView(), cam.getProjection());




		Profiler::begin("post processing");
		/// bloom
		// ---------------

		if (settings.bloom) {
			doBloom();

			glViewport(0, 0, resolution.x, resolution.y);
		}



		// post processing
		// ---------------
		if (settings.fxaa) {
			HDRbuffer2.bind();
		} else {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, Window::getWindowSize().x, Window::getWindowSize().y);
		}


		bloomBuffer1.getAttachment(0).bind(0);
		bloomBuffer3.getAttachment(0).bind(1);
		bloomBuffer5.getAttachment(0).bind(2);
		HDRbuffer.getAttachment(0).bind(3);

		tonemapShader.bind();
		tonemapShader.uniformBool("vignette", settings.vignette);
		tonemapShader.uniformBool("tonemap", settings.tonemap);
		tonemapShader.uniformBool("bloom", settings.bloom);
		tonemapShader.uniformInt("texture0", 0);
		tonemapShader.uniformInt("texture1", 1);
		tonemapShader.uniformInt("texture2", 2);
		tonemapShader.uniformInt("texture3", 3);

		framebufferMesh.render();


		// fxaa
		// -------------------------------
		if (settings.fxaa) {
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glViewport(0, 0, Window::getWindowSize().x, Window::getWindowSize().y);
			fxaaShader.bind();
			HDRbuffer2.getAttachment(0).bind();
			framebufferMesh.render();
		}


		// copy depth and stencil buffer
		// -------------------------------
		glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer.fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBlitFramebuffer(0, 0, resolution.x, resolution.y, 0, 0, resolution.x, resolution.y, GL_STENCIL_BUFFER_BIT, GL_NEAREST);

		Profiler::end();


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

		renderQueue.clear();
		renderQueueOutline.clear();

		Profiler::end();
	}

	// ------------------------------------------------------------------------
	void setSkyboxShader(Shader s) {
		skyboxShader = s;
	}

	// ------------------------------------------------------------------------
	void setSun(DirectionalLight light) {
		sun = light;
	}

	// ------------------------------------------------------------------------
	void generateIBLmaps() {
		IBL::generateIBLmaps();
	}
}