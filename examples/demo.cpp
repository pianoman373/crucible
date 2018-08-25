#include <crucible/crucible.hpp>
#include <btBulletDynamicsCommon.h>

#include <crucible/GameObject.hpp>
#include <crucible/Scene.hpp>
#include <crucible/RigidBody.hpp>
#include <crucible/Bone.hpp>
#include <crucible/Stopwatch.hpp>
#include <imgui.h>

class CharacterController : public Component {
    Camera &cam;
    GameObject &gun;

public:
    CharacterController(Camera &cam, GameObject &gun): cam(cam), gun(gun) {

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

        //camera control
        float distance = 0.0f;

        static vec2 lastMousePos;
        Window::setMouseGrabbed(Input::isMouseButtonDown(1));

        if (Input::isMouseButtonDown(1)) {
            vec3 right = cam.getRight();

            //rotation
            vec2 offset = Input::getCursorPos() - lastMousePos;
            float xOffset = -offset.x / 10.0f;
            float yOffset = offset.y / 10.0f;

            rotX += xOffset;
            rotY += yOffset;
        }
        cam.direction = quaternion(vec3(0.0f, 1.0f, 0.0f), radians(rotX)) * quaternion(vec3(1.0f, 0.0f, 0.0f), radians(-rotY)) * vec3(0.0f, 0.0f, -1.0f);
        cam.position = getParent()->transform.position + (-cam.direction * distance);

        lastMousePos = Input::getCursorPos();

        vec3 gunOffset = (cam.getRight() * 0.15f) + (cam.getDirection() * 0.2f) - (cam.getUp() * 0.15f);

        // gun control
        gun.transform.position = this->getParent()->transform.position + gunOffset;
        gun.transform.rotation = quaternion(vec3(0.0f, 1.0f, 0.0f), radians(rotX + 180.0f)) * quaternion(vec3(1.0f, 0.0f, 0.0f), radians(rotY));
    }
};

int main() {
	Window::create({ 1280, 720 }, "Crucible Demo", false);
	Camera cam(vec3(0.0f));

	Renderer::init(true, 2048, 1280, 720);

	Cubemap cubemap;
	cubemap.loadEquirectangular("resources/canyon.hdr");

    Renderer::environment = cubemap;
    IBL::generateIBLmaps(vec3(), Renderer::irradiance, Renderer::specular);


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
	shaderBall.importFile("resources/shaderball.fbx", false);

	Model environment;
	environment.importFile("resources/environment.fbx");

	Model gunModel;
	gunModel.openFile("resources/Rifle/rifle.crmodel");

	Model character;
	character.importFile("resources/Character Running.fbx");
	character.materials[0].setPBRUniforms(Resources::getTexture("resources/Character Texture.png"), 0.7f, 0.0f);

	Material wood;
	wood.loadFile("resources/wood.crmaterial");

    Material plastic;
    plastic.loadFile("resources/plastic.crmaterial");

    Material rustediron;
    rustediron.loadFile("resources/rustediron.crmaterial");

    Material gold;
    gold.loadFile("resources/gold.crmaterial");

    Material ceramic;
    ceramic.loadFile("resources/ceramic.crmaterial");

    Material checker;
    checker.loadFile("resources/checkerboard.crmaterial");

    Material mirror;
    mirror.setPBRUniforms(vec3(1.0f), 0.0f, 1.0f);

	bool keyDown = false;

	std::vector<vec3> lightPositions;

    Scene scene;
    scene.setupPhysicsWorld();

    scene.createMeshObject(shaderBall.nodes[0].mesh, plastic, Transform(vec3(0.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 0").addRigidBody(0.0f)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);
    scene.createMeshObject(shaderBall.nodes[0].mesh, wood, Transform(vec3(4.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 1").addRigidBody(0.0f)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);
    scene.createMeshObject(shaderBall.nodes[0].mesh, rustediron, Transform(vec3(-4.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 2").addRigidBody(0.0f)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);
    scene.createMeshObject(shaderBall.nodes[0].mesh, gold, Transform(vec3(8.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 3").addRigidBody(0.0f)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);
    scene.createMeshObject(shaderBall.nodes[0].mesh, checker, Transform(vec3(-8.0f, 1.9f, -10.0f), quaternion(), vec3(0.25f)), "shaderball 4").addRigidBody(0.0f)->addSphereCollider(vec3(), 1.5f)->addCylinderCollider(vec3(0.f, -1.7f, 0.0f), 1.5f, 0.2f);

    scene.createMeshObject(environment.nodes[0].mesh, checker, Transform(vec3(), quaternion(), vec3(0.5f)), "environment").addRigidBody(0.0f)->addMeshCollider(vec3(), environment.nodes[0].mesh, vec3(0.5f));

    GameObject &gun = scene.createMeshObject(gunModel.nodes[0].mesh, gunModel.materials[0], Transform(vec3(), quaternion(), vec3(0.05f)), "gun");


    GameObject &torusObject0 = scene.createMeshObject(torus0, gold, Transform(vec3(0.0f, 20.0f, -10.0f)), "torus 0");
    GameObject &torusObject1 = scene.createMeshObject(torus1, gold, Transform(vec3(0.0f, 20.0f, -10.0f)), "torus 1");
    GameObject &torusObject2 = scene.createMeshObject(torus2, gold, Transform(vec3(0.0f, 20.0f, -10.0f)), "torus 2");

    GameObject &player = scene.createObject(Transform(vec3(0.0f, 10.0f, 0.0f)), "player");
    RigidBody *rb = player.addRigidBody(1.0f);
    rb->addSphereCollider(vec3(), 0.5f);
    rb->addSphereCollider(vec3(0.0f, -1.0f, 0.0f), 0.5f);
    rb->setAngularFactor(0.0f);
    player.addComponent(new CharacterController(cam, gun));

    vec2i lastResolution = Window::getWindowSize();

    bool first = true;

    Bone root("resources/Character Running.fbx", "Torso");

    std::vector<Transform> transforms;

    int amount = 50;
    float size = 20.0f;
    for (int x = 0; x < amount; x++) {
        for (int z = 0; z < amount; z++) {
            ///Renderer::render(&cube, &checker, , quaternion(), vec3(0.1f)));
            transforms.push_back(Transform(vec3(((float)x / (float)amount) * size, 3.0f, ((float)z / (float)amount) * size), quaternion(), vec3(0.1f)));
        }
    }

    Transform characterTransform = Transform(vec3(5.0f, 0.0f, 0.0f), quaternion(), vec3(0.3f));


    while (Window::isOpen()) {
        root.children[0].children[0].rotation = quaternion(vec3(0.0f, 1.0f, 0.0f), radians(180.0f)) * quaternion(normalize(vec3(5.2f, 1.0f, 0.1f)), sin(Window::getTime())*0.5f);
        root.children[0].children[2].rotation = quaternion(vec3(0.0f, 1.0f, 0.0f), radians(180.0f)) * quaternion(normalize(vec3(5.2f, 1.0f, 0.1f)), radians(180.0f) + cos(Window::getTime()));
        root.children[0].children[2].children[0].rotation = quaternion(normalize(vec3(1.0f, 0.0f, 0.0f)), cos(Window::getTime())+1.0f);

        Window::begin();
        cam.dimensions = {(float)Window::getWindowSize().x, (float)Window::getWindowSize().y};

        if (!(lastResolution == Window::getWindowSize())) {
            Renderer::resize(Window::getWindowSize().x, Window::getWindowSize().y);
        }

        lastResolution = Window::getWindowSize();

        if (keyDown) {
			if (!Input::isMouseButtonDown(0)) {
				keyDown = false;
			}
        }
        else {
			if (Input::isMouseButtonDown(0)) {
				keyDown = true;

                GameObject &obj = scene.createMeshObject(cube, checker, Transform(cam.getPosition(), quaternion(), vec3(1.00f)), "physics object 1");
                obj.addRigidBody(1.0f);
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

        scene.update(ImGui::GetIO().DeltaTime);
        scene.render();

        if (first) {
            IBL::generateIBLmaps(vec3(0.0f,  2.0f, 0.0f), Renderer::irradiance, Renderer::specular);
            first = false;
        }

        Renderer::render(character.nodes[0].mesh, character.materials[0], characterTransform, root);

        for (int i = 0; i < transforms.size(); i++) {
            Renderer::render(cube, checker, transforms[i]);
        }

        root.debugDraw();

        Renderer::flush(cam);

        Window::end();
    }
}