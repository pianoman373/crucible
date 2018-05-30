#include <crucible/crucible.hpp>

int main(int argc, char *argv[]) {
	Window::create({ 1280, 720 }, "test", false);
	Camera cam(vec3(1.0f, 1.0f, 6.0f));

	Renderer::init(true, 2048, 1280, 720);
	Renderer::settings.tonemap = true;
	Renderer::settings.vignette = true;
	Renderer::settings.bloom = true;
	Renderer::settings.ssao = true;
	Renderer::settings.fxaa = false;
	Shader s;
	s.loadFile("resources/skybox.vsh", "resources/skybox.fsh");
	//Renderer::setSkyboxShader(s);

	Cubemap cubemap;
	cubemap.loadEquirectangular("resources/canyon.hdr");

	Renderer::environment = cubemap;
	Renderer::generateIBLmaps();

	cubemap.setID(0);
  	Renderer::environment = cubemap;


	Material ground;
	ground.setShader(Renderer::standardShader);
	ground.setPBRUniforms(vec3(0.1f), 0.5f, 0.0f);
    ground.setUniformFloat("emission", 0.0f);

	Material ball;
	ball.setShader(Renderer::standardShader);
	ball.setPBRUniforms(vec3(1.0f, 1.0f, 1.0f), 0.0f, 0.0f);
	ball.setUniformBool("aoTextured", false);

	Renderer::setSun({ vec3(1.05f, -7.8f, -1.3f), vec3(10.0f, 10.0f, 10.0f) });

    //Renderer::pointLights.push_back({vec3(0.0f, 8.0f, 0.0f), vec3(200.0f, 200.0f, 200.0f)});

    Model model;
    model.loadFile("resources/shaderball.fbx");

	Mesh cube;
	Primitives::cube(cube);

	Mesh sphere;
	Primitives::sphere(sphere, 32, 32);

    while (Window::isOpen()) {
        Window::begin();
        cam.dimensions = {(float)Window::getWindowSize().x, (float)Window::getWindowSize().y};
        Util::updateSpaceCamera(cam);



        Renderer::render(&model, Transform(vec3(0.0f, 5.0f, 0.0f), vec3(), vec3(1.0f)), AABB());
				Renderer::render(&cube, &ground, Transform(vec3(), vec3(), vec3(20.0f, 1.0f, 20.0f)), AABB()); //ground

				Renderer::render(&cube, &ground, Transform(vec3(0.0f, -20.0f, 0.0f), vec3(), vec3(2000.0f, 1.0f, 2000.0f)), AABB());

				Renderer::render(&sphere, &model.materials[0], Transform(vec3(0.0f, 1.5f, 0.0f), vec3(), vec3(1.0f)), AABB());

				//walls
				Renderer::render(&cube, &ground, Transform(vec3(0.0f, 1.0f, 9.5f), vec3(), vec3(20.0f, 1.0f, 1.0f)), AABB());
				Renderer::render(&cube, &ground, Transform(vec3(0.0f, 1.0f, -9.5f), vec3(), vec3(20.0f, 1.0f, 1.0f)), AABB());

				Renderer::render(&cube, &ground, Transform(vec3(9.5f, 1.0f, 0.0f), vec3(), vec3(1.0f, 1.0f, 18.0f)), AABB());
				Renderer::render(&cube, &ground, Transform(vec3(-9.5f, 1.0f, 0.0f), vec3(), vec3(1.0f, 1.0f, 18.0f)), AABB());

				Renderer::debug.renderDebugAABB(vec3(-1.0f, -1.0f, -1.0f), vec3(1.0f, 1.0f, 1.0f), vec3(1.0f, 0.0f, 0.0f));

        Renderer::flush(cam);

        Window::end();
    }
}
