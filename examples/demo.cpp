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
    ground.setPBRUniforms(vec3(0.1f), 0.01f, 0.0f);

	Material ball;
	ball.setPBRUniforms(vec3(1.0f, 1.0f, 1.0f), 0.0f, 0.0f);

	Material glowy;
	glowy.setPBRUniforms(vec3(2.0f, 1.5f, 1.0f), 0.0f, 0.0f);
	glowy.setUniformFloat("emission", 1000.0f);


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

    std::cout << cube.toJson() << std::endl;


	Model shaderBall;
	shaderBall.importFile("resources/shaderball.fbx", false);

	Model rifle;
	rifle.importFile("../crucible-resources-stuff/sponza/sponza.fbx");


	Material wood;
	wood.loadFile("resources/wood.crmaterial");

    std::cout << wood.toJson("resources/wood.crmaterial") << std::endl;
    wood.saveFile("../crucible-resources-stuff/test.crmaterial");

    Material plastic;
    plastic.loadFile("resources/plastic.crmaterial");

    Material rustediron;
    rustediron.loadFile("resources/rustediron.crmaterial");

    Material gold;
    gold.loadFile("resources/gold.crmaterial");

    Material ceramic;
    ceramic.loadFile("resources/ceramic.crmaterial");

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

        Renderer::render(&shaderBall.nodes[0].mesh, &plastic, Transform(vec3(0.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.nodes[0].mesh, &wood, Transform(vec3(7.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.nodes[0].mesh, &rustediron, Transform(vec3(-7.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.nodes[0].mesh, &gold, Transform(vec3(14.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.nodes[0].mesh, &ceramic, Transform(vec3(-14.0f, 0.0f, 0.0f), quaternion(), vec3(0.5f)), AABB());

        Renderer::render(&cube, &gold, Transform(vec3(0.0f, 10.0f, 0.0f) + (q2 * vec3(0.0f, 0.0f, 5.0f)), quaternion(), vec3(1.0f)), AABB());

        Renderer::render(&cube, &gold, Transform(vec3(0.0f, -4.3f, 0.0f), quaternion(), vec3(100.0f, 1.0f, 100.0f)), AABB()); //ground

        Renderer::render(&cube, &ceramic, Transform(vec3(0.0f, 0.0f, -50.0f), quaternion(), vec3(100.0f, 100.0f, 1.0f)), AABB());

        //Renderer::render(&rifle, Transform(vec3(), quaternion(), vec3(1.0f)), AABB());

        //Renderer::render(&sphere, &ball, Transform(vec3(0.0f, 1.5f, 0.0f), vec3(), vec3(1.0f)), AABB());

        Renderer::flush(cam);

        Window::end();
    }
}
