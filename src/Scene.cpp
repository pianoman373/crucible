#include <crucible/Scene.hpp>
#include <crucible/Renderer.hpp>

// debug draw ---------------------------------
class CrucibleBulletDebugDraw: public btIDebugDraw{
    int m_debugMode;

public:
    virtual void drawLine(const btVector3& from,const btVector3& to,const btVector3& color);
    virtual void drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color);
    virtual void reportErrorWarning(const char* warningString);
    virtual void draw3dText(const btVector3& location,const char* textString);
    virtual void setDebugMode(int debugMode);
    virtual int  getDebugMode() const;
};

void CrucibleBulletDebugDraw::drawLine(const btVector3& from,const btVector3& to,const btVector3& color) {
    Renderer::debug.renderDebugLine({from.x(), from.y(), from.z()}, {to.x(), to.y(), to.z()}, {color.x(), color.y(), color.z()});
}

void CrucibleBulletDebugDraw::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color){}

void CrucibleBulletDebugDraw::reportErrorWarning(const char *warningString){
    std::cout << "[Bullet Warning]: " << warningString << std::endl;
}

void CrucibleBulletDebugDraw::draw3dText(const btVector3 &location, const char *textString){}

void CrucibleBulletDebugDraw::setDebugMode(int debugMode){}

int CrucibleBulletDebugDraw::getDebugMode() const {
    return DBG_DrawWireframe;
}
// debug draw ---------------------------------

Scene::~Scene() {
    for (int i = 0; i < objects.size(); i++) {
        delete objects[i];
    }
    objects.clear();

    delete debugDrawer;
}

GameObject &Scene::createObject(const Transform &transform, const std::string &name) {
    GameObject *obj = new GameObject(*this, transform, name);
    objects.push_back(obj);

    return *obj;
}

GameObject &Scene::createMeshObject(Mesh &mesh, Material &material, const Transform &transform,
                                           const std::string &name) {
    GameObject &obj = createObject(transform, name);
    obj.addComponent(new ModelComponent(mesh, material));

    return obj;
}

void Scene::render() {
    if (physicsEnabled) {
        //dynamicsWorld->debugDrawWorld();
    }

    for (int i = 0; i < objects.size(); i++) {
        objects[i]->render();
    }
}

void Scene::update(float delta) {
    dynamicsWorld->stepSimulation(delta, 10);

    for (int i = 0; i < objects.size(); i++) {
        objects[i]->update(delta);
    }
}

void Scene::setupPhysicsWorld() {
    ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
    collisionConfiguration = new btDefaultCollisionConfiguration();

    ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
    dispatcher = new btCollisionDispatcher(collisionConfiguration);

    ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
    overlappingPairCache = new btDbvtBroadphase();

    ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
    solver = new btSequentialImpulseConstraintSolver;

    dynamicsWorld = new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

    debugDrawer = new CrucibleBulletDebugDraw();
    dynamicsWorld->setDebugDrawer(debugDrawer);
    dynamicsWorld->getDebugDrawer()->setDebugMode(btIDebugDraw::DBG_DrawWireframe);

    dynamicsWorld->setGravity(btVector3(0, -9.8f, 0));

    physicsEnabled = true;
}

btDiscreteDynamicsWorld *Scene::getBulletWorld() {
    return dynamicsWorld;
}

bool Scene::isPhysicsEnabled() const {
    return physicsEnabled;
}

int Scene::numObjects() {
    return objects.size();
}

GameObject &Scene::getObject(int index) {
    return *objects[index];
}