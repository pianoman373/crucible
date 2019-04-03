#include <crucible/crucible.hpp>

class CharacterController : public Component {
    Camera &cam;

public:
    CharacterController(Camera &cam): cam(cam) {

    }

    void update(float delta) {
        static float rotX;
        static float rotY;

        vec3 movement;

        if (Input::isKeyDown(Input::KEY_W)) {
            vec3 vel = normalize(cam.getDirection());
            vel.y = 0.0f;
            vel = normalize(vel) * 5.0f;

            movement = movement + vel;
        }
        if (Input::isKeyDown(Input::KEY_S)) {
            vec3 vel = -normalize(cam.getDirection());
            vel.y = 0.0f;
            vel = normalize(vel) * 5.0f;

            movement = movement + vel;
        }
        if (Input::isKeyDown(Input::KEY_D)) {
            vec3 vel = normalize(cam.getRight());
            vel.y = 0.0f;
            vel = normalize(vel) * 5.0f;

            movement = movement + vel;
        }
        if (Input::isKeyDown(Input::KEY_A)) {
            vec3 vel = -normalize(cam.getRight());
            vel.y = 0.0f;
            vel = normalize(vel) * 5.0f;

            movement = movement + vel;
        }

        movement.y = getParent()->getRigidBody()->getVelocity().y;


        if (Input::isKeyDown(Input::KEY_SPACE)) {
            movement.y = 5.0f;
        }

        getParent()->getRigidBody()->setVelocity(movement);

        static vec2 lastMousePos;
        Window::setMouseGrabbed(Input::isMouseButtonDown(1));

        if (Input::isMouseButtonDown(1)) {
            //rotation
            vec2 offset = Input::getCursorPos() - lastMousePos;
            float xOffset = -offset.x / 10.0f;
            float yOffset = -offset.y / 10.0f;

            rotX += xOffset;
            rotY += yOffset;
        }
        cam.direction = quaternion(vec3(0.0f, 1.0f, 0.0f), radians(rotX)) * quaternion(vec3(1.0f, 0.0f, 0.0f), radians(-rotY)) * vec3(0.0f, 0.0f, -1.0f);
        cam.position = getParent()->transform.position;

        lastMousePos = Input::getCursorPos();
    }
};

int main() {
	Window::create({ 1280, 720 }, "Crucible Demo", false, false);
    Renderer::init(true, 2048, 1280, 720);

	Camera cam;
	int thing;


	Cubemap cubemap;
	cubemap.loadEquirectangular("resources/canyon.hdr");
    Renderer::environment = cubemap;
    IBL::generateIBLmaps(vec3(), Renderer::irradiance, Renderer::specular);
	Renderer::setSun({ vec3(1.05f, -1.2f, -1.3f), vec3(10.0f, 10.0f, 10.0f) });

	Mesh cube = Primitives::cube();
	Mesh torus0 = Primitives::torus(2.0f, 0.5f, 64, 64);
    Mesh torus1 = Primitives::torus(3.0f, 0.5f, 64, 64);
    Mesh torus2 = Primitives::torus(4.0f, 0.5f, 64, 64);

	Mesh shaderBall = Resources::getAssimpFile("resources/shaderball.fbx").getMesh(0);
	Mesh environment =  Resources::getAssimpFile("resources/environment.fbx").getMesh(0);

	Material &wood = Resources::getMaterial("resources/wood.crmaterial");
    Material &plastic = Resources::getMaterial("resources/plastic.crmaterial");
    Material &rustediron = Resources::getMaterial("resources/rustediron.crmaterial");
    Material &gold = Resources::getMaterial("resources/gold.crmaterial");
    Material &ceramic = Resources::getMaterial("resources/ceramic.crmaterial");
    Material &checker = Resources::getMaterial("resources/checkerboard.crmaterial");

    Scene scene;
    scene.setupPhysicsWorld();

    scene.createMeshObject(shaderBall, plastic, Transform(vec3(0.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 0").addRigidBody(0.0f, scene)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);
    scene.createMeshObject(shaderBall, wood, Transform(vec3(4.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 1").addRigidBody(0.0f, scene)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);
    scene.createMeshObject(shaderBall, rustediron, Transform(vec3(-4.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 2").addRigidBody(0.0f, scene)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);
    scene.createMeshObject(shaderBall, gold, Transform(vec3(8.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 3").addRigidBody(0.0f, scene)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);
    scene.createMeshObject(shaderBall, checker, Transform(vec3(-8.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 4").addRigidBody(0.0f, scene)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);

    scene.createMeshObject(environment, checker, Transform(vec3(), quaternion(), vec3(0.5f)), "environment").addRigidBody(0.0f, scene)->addMeshCollider(vec3(), environment, vec3(0.5f));

    GameObject &torusObject0 = scene.createMeshObject(torus0, gold, Transform(vec3(0.0f, 20.0f, -10.0f)), "torus 0");
    GameObject &torusObject1 = scene.createMeshObject(torus1, gold, Transform(vec3(0.0f, 20.0f, -10.0f)), "torus 1");
    GameObject &torusObject2 = scene.createMeshObject(torus2, gold, Transform(vec3(0.0f, 20.0f, -10.0f)), "torus 2");

    GameObject &player = scene.createObject(Transform(vec3(0.0f, 10.0f, 0.0f)), "player");
    RigidBody *rb = player.addRigidBody(1.0f, scene);
    rb->addSphereCollider(vec3(), 0.5f);
    rb->addSphereCollider(vec3(0.0f, -1.0f, 0.0f), 0.5f);
    rb->setAngularFactor(0.0f);
    player.addComponent(new CharacterController(cam));


    bool first = true;
    bool keyDown = false;

    while (Window::isOpen()) {
        Window::begin();
        cam.matchWindowResolution();
        Renderer::matchWindowResolution();

        if (keyDown) {
			if (!Input::isMouseButtonDown(0)) {
				keyDown = false;
			}
        }
        else {
			if (Input::isMouseButtonDown(0)) {
				keyDown = true;

                GameObject &obj = scene.createMeshObject(cube, checker, Transform(cam.getPosition() + cam.getDirection(), quaternion(), vec3(1.00f)), "physics object 1");
                obj.addRigidBody(1.0f, scene);
                obj.getRigidBody()->addBoxCollider(vec3(), vec3(0.5f));
                obj.getRigidBody()->setVelocity(cam.getDirection() * 20.0f);
			}
        }

        quaternion q0 = quaternion(vec3(1.0f, 0.0f, 0.0f), Window::getTime());
        quaternion q1 = normalize(q0 * quaternion(vec3(0.0f, 1.0f, 0.0f), Window::getTime()*2.0f));
        quaternion q2 = normalize(q1 * quaternion(vec3(1.0f, 0.0f, 0.0f), Window::getTime()*3.0f));

        torusObject0.transform.rotation = q2;
        torusObject1.transform.rotation = q1;
        torusObject2.transform.rotation = q0;

        scene.update(Window::deltaTime());
        scene.render();

        if (first) {
            IBL::generateIBLmaps(vec3(0.0f,  2.0f, 0.0f), Renderer::irradiance, Renderer::specular);
            first = false;
        }

        Renderer::flush(cam);
        Window::end();
    }
}