#include <crucible/crucible.hpp>

int main() {
	Window::create({ 1280, 720 }, "test", false);
	Camera cam(vec3(0.0f, 5.0f, 10.0f));

	Renderer::init(true, 2048, 1280, 720);

	Cubemap cubemap;
	cubemap.loadEquirectangular("resources/canyon.hdr");

    Renderer::environment = cubemap;
    IBL::generateIBLmaps(Renderer::irradiance, Renderer::specular);

	Material ground;
    ground.setShader(Renderer::standardShader);
    ground.setPBRUniforms(vec3(0.1f), 0.01f, 0.0f);

	Material ball;
	ball.setShader(Renderer::standardShader);
	ball.setPBRUniforms(vec3(1.0f, 1.0f, 1.0f), 0.0f, 0.0f);

	Material glowy;
	glowy.setShader(Renderer::standardShader);
	glowy.setPBRUniforms(vec3(2.0f, 1.5f, 1.0f) * 0.5f, 0.0f, 0.0f);
	glowy.setUniformFloat("emission", 1.0f);


	Renderer::setSun({ vec3(1.05f, -1.2f, -1.3f), vec3(10.0f, 10.0f, 10.0f) });

	Mesh cube;
	Primitives::cube(cube);

	Mesh sphere;
	Primitives::sphere(sphere, 32, 32);

	Mesh torus0;
	Primitives::torus(torus0, 2.0f, 0.5f, 64, 64);

    Mesh torus1;
    Primitives::torus(torus1, 3.0f, 0.5f, 64, 64);

    Mesh torus2;
    Primitives::torus(torus2, 4.0f, 0.5f, 64, 64);


	Model shaderBall;
	shaderBall.loadFile("resources/shaderball.fbx", false);

	Model sponza;
	sponza.loadFile("../crucible-resources-stuff/sponza/sponza.fbx", true);


	Material wood;
    wood.setShader(Renderer::standardShader);
	wood.setPBRUniforms(
	        Resources::getTexture("resources/wood/albedo.png"),
            Resources::getTexture("resources/wood/roughness.png"),
            Resources::getTexture("resources/wood/metallic.png"),
            Resources::getTexture("resources/wood/normal.png")
	        );

    Material plastic;
    plastic.setShader(Renderer::standardShader);
    plastic.setPBRUniforms(
            Resources::getTexture("resources/plastic/albedo.png"),
            Resources::getTexture("resources/plastic/roughness.png"),
            Resources::getTexture("resources/plastic/metallic.png"),
            Resources::getTexture("resources/plastic/normal.png")
    );

    Material rustediron;
    rustediron.setShader(Renderer::standardShader);
    rustediron.setPBRUniforms(
            Resources::getTexture("resources/rustediron/albedo.png"),
            Resources::getTexture("resources/rustediron/roughness.png"),
            Resources::getTexture("resources/rustediron/metallic.png"),
            Resources::getTexture("resources/rustediron/normal.png")
    );

    Material gold;
    gold.setShader(Renderer::standardShader);
    gold.setPBRUniforms(
            vec3(1.0f, 0.8f, 0.3f),
            0.3f,
            1.0f
    );

    Material ceramic;
    ceramic.setShader(Renderer::standardShader);
    ceramic.setPBRUniforms(
            vec3(0.05f, 0.05f, 1.0f),
            0.1f,
            0.0f
    );

	bool keyDown = false;

	std::vector<vec3> lightPositions;

    while (Window::isOpen()) {
        Window::begin();
        cam.dimensions = {(float)Window::getWindowSize().x, (float)Window::getWindowSize().y};
        Util::updateSpaceCamera(cam);

        if (keyDown) {
			if (!Input::isMouseButtonDown(0)) {
				keyDown = false;
			}
        }
        else {
			if (Input::isMouseButtonDown(0)) {
				keyDown = true;

				lightPositions.push_back(cam.getPosition());
			}
        }

        for (int i = 0; i < lightPositions.size(); i++) {
        	Renderer::renderPointLight(lightPositions[i], vec3(2.0f, 1.5f, 1.0f), 50.0f);
        	Renderer::render(&sphere, &glowy, Transform(lightPositions[i], quaternion(), vec3(0.3f)), AABB());
        }

        quaternion q0 = quaternion(vec3(1.0f, 0.0f, 0.0f), Window::getTime());
        quaternion q1 = normalize(q0 * quaternion(vec3(0.0f, 1.0f, 0.0f), Window::getTime())  );
        quaternion q2 = normalize(q1 * quaternion(vec3(1.0f, 0.0f, 0.0f), Window::getTime()) );

        Renderer::render(&torus0, &gold, Transform(vec3(0.0f, 10.0f, 0.0f), q2, vec3(1.0f)), AABB());
        Renderer::render(&torus1, &gold, Transform(vec3(0.0f, 10.0f, 0.0f), q1, vec3(1.0f)), AABB());
        Renderer::render(&torus2, &gold, Transform(vec3(0.0f, 10.0f, 0.0f), q0, vec3(1.0f)), AABB());

        Renderer::render(&shaderBall.meshes[0], &plastic, Transform(vec3(0.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.meshes[0], &wood, Transform(vec3(7.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.meshes[0], &rustediron, Transform(vec3(-7.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.meshes[0], &gold, Transform(vec3(14.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.meshes[0], &ceramic, Transform(vec3(-14.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());

        Renderer::render(&cube, &gold, Transform(vec3(0.0f, 10.0f, 0.0f) + (q2 * vec3(0.0f, 0.0f, 5.0f)), quaternion(), vec3(1.0f)), AABB());

        //Renderer::render(&sponza, Transform(vec3(-14.0f, 0.0f, 0.0f), vec3(), vec3(1.0f)), AABB());

        Renderer::render(&cube, &gold, Transform(vec3(0.0f, -4.3f, 0.0f), quaternion(), vec3(100.0f, 1.0f, 100.0f)), AABB()); //ground

        Renderer::render(&cube, &ceramic, Transform(vec3(0.0f, 0.0f, -50.0f), quaternion(), vec3(100.0f, 100.0f, 1.0f)), AABB());

        //Renderer::render(&sphere, &ball, Transform(vec3(0.0f, 1.5f, 0.0f), vec3(), vec3(1.0f)), AABB());

        Renderer::flush(cam);

        Window::end();
    }
}
