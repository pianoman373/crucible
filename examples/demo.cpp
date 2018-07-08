#include <crucible/crucible.hpp>
#include <btBulletDynamicsCommon.h>

class MyDebugDraw: public btIDebugDraw{
    int m_debugMode;

public:
    virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color);
    virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
    virtual void reportErrorWarning(const char* warningString);
    virtual void draw3dText(const btVector3& location,const char* textString);
    virtual void setDebugMode(int debugMode);
    virtual int  getDebugMode() const;
};

void MyDebugDraw::drawLine(const btVector3& from,const btVector3& to,const btVector3& color) {
    Renderer::debug.renderDebugLine({from.x(), from.y(), from.z()}, {to.x(), to.y(), to.z()}, {color.x(), color.y(), color.z()});
}

void MyDebugDraw::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color){}

void MyDebugDraw::reportErrorWarning(const char *warningString){
    std::cout << "[Bullet Warning]: " << warningString << std::endl;
}

void MyDebugDraw::draw3dText(const btVector3 &location, const char *textString){}

void MyDebugDraw::setDebugMode(int debugMode){}

int MyDebugDraw::getDebugMode() const {
    return DBG_DrawWireframe;
}

int main() {
	Window::create({ 1280, 720 }, "Crucible Demo", false);
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

	Mesh floor;
	Primitives::cube(floor, 0.1f, 100.0f, 1.0f, 100.0f);

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

	bool keyDown = false;

	std::vector<vec3> lightPositions;

    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    btDefaultCollisionConfiguration* collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    btCollisionDispatcher* dispatcher = new btCollisionDispatcher(collisionConfiguration);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    btBroadphaseInterface* overlappingPairCache = new btDbvtBroadphase();

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    btSequentialImpulseConstraintSolver* solver = new btSequentialImpulseConstraintSolver;

    btDiscreteDynamicsWorld* dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

    MyDebugDraw debugDrawer;
    dynamicsWorld->setDebugDrawer(&debugDrawer);
    dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

    dynamicsWorld->setGravity(btVector3(0, -10, 0));

    ///-----initialization_end-----
    //keep track of the shapes, we release memory at exit.
    //make sure to re-use collision shapes among rigid bodies whenever possible!
    btAlignedObjectArray<btCollisionShape*> collisionShapes;

    ///create a few basic rigid bodies

    //the ground is a cube of side 100 at position y = -56.
    //the sphere will hit it at y = -6, with center at -5
    {
        btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

        collisionShapes.push_back(groundShape);

        btTransform groundTransform;
        groundTransform.setIdentity();
        groundTransform.setOrigin(btVector3(0, -50, 0));

        btScalar mass(0.0f);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            groundShape->calculateLocalInertia(mass, localInertia);

        //using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        //add the body to the dynamics world
        dynamicsWorld->addRigidBody(body);
    }

    for (int i = 0; i < 5; i++) {
        //create a dynamic rigidbody

        //btCollisionShape* colShape = new btBoxShape(btVector3(0.5f,0.5f,0.5f));
        btCollisionShape* colShape = new btSphereShape(btScalar(1.));
        collisionShapes.push_back(colShape);

        /// Create Dynamic Objects
        btTransform startTransform;
        startTransform.setIdentity();

        btScalar mass(1.f);

        //rigidbody is dynamic if and only if mass is non zero, otherwise static
        bool isDynamic = (mass != 0.f);

        btVector3 localInertia(0, 0, 0);
        if (isDynamic)
            colShape->calculateLocalInertia(mass, localInertia);

        startTransform.setOrigin(btVector3(0, 10 + (i*2), 0));

        //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
        btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
        btRigidBody* body = new btRigidBody(rbInfo);
        body->setLinearVelocity(btVector3(0.1f, 0, 0));


        dynamicsWorld->addRigidBody(body);
    }

    dynamicsWorld->setGravity ( btVector3 (0 , -9.8f ,0) ) ;

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
        quaternion q1 = normalize(q0 * quaternion(vec3(0.0f, 1.0f, 0.0f), Window::getTime()*2.0f));
        quaternion q2 = normalize(q1 * quaternion(vec3(1.0f, 0.0f, 0.0f), Window::getTime()*3.0f));

        Renderer::render(&torus0, &gold, Transform(vec3(0.0f, 20.0f, -10.0f), q2, vec3(1.0f)), AABB());
        Renderer::render(&torus1, &gold, Transform(vec3(0.0f, 20.0f, -10.0f), q1, vec3(1.0f)), AABB());
        Renderer::render(&torus2, &gold, Transform(vec3(0.0f, 20.0f, -10.0f), q0, vec3(1.0f)), AABB());

        Renderer::render(&shaderBall.nodes[0].mesh, &plastic, Transform(vec3(0.0f, 3.7f, -10.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.nodes[0].mesh, &wood, Transform(vec3(7.0f, 3.7f, -10.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.nodes[0].mesh, &rustediron, Transform(vec3(-7.0f, 3.7f, -10.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.nodes[0].mesh, &gold, Transform(vec3(14.0f, 3.7f, -10.0f), quaternion(), vec3(0.5f)), AABB());
        Renderer::render(&shaderBall.nodes[0].mesh, &checker, Transform(vec3(-14.0f, 3.7f, -10.0f), quaternion(), vec3(0.5f)), AABB());

        Renderer::render(&environment.nodes[0].mesh, &checker, Transform(), AABB()); //ground

        dynamicsWorld->stepSimulation(1.f / 60.f, 10);

        //print positions of all objects
        for (int j = 0; j < dynamicsWorld->getNumCollisionObjects(); j++)
        {
            btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[j];
            btRigidBody* body = btRigidBody::upcast(obj);
            btTransform trans;
            if (body && body->getMotionState())
            {
                body->getMotionState()->getWorldTransform(trans);
            }
            else
            {
                trans = obj->getWorldTransform();
            }
            vec3 pos = vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
            quaternion quat = quaternion(trans.getRotation().w(), trans.getRotation().x(), trans.getRotation().y(), trans.getRotation().z());

            Renderer::render(&cube, &checker, Transform(pos, quat, vec3(1.0f)), AABB());
        }

        dynamicsWorld->debugDrawWorld();

        //Renderer::render(&cube, &checker, Transform(vec3(), quaternion(), vec3(1.0f)), AABB());

        Renderer::flush(cam);

        Window::end();
    }

    //remove the rigidbodies from the dynamics world and delete them
    for (int i = dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
    {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if (body && body->getMotionState())
        {
            delete body->getMotionState();
        }
        dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }

    //delete collision shapes
    for (int j = 0; j < collisionShapes.size(); j++)
    {
        btCollisionShape* shape = collisionShapes[j];
        collisionShapes[j] = 0;
        delete shape;
    }

    //delete dynamics world
    delete dynamicsWorld;

    //delete solver
    delete solver;

    //delete broadphase
    delete overlappingPairCache;

    //delete dispatcher
    delete dispatcher;

    delete collisionConfiguration;

    //next line is optional: it will be cleared by the destructor when the array goes out of scope
    collisionShapes.clear();
}
