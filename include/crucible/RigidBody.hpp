#pragma once

#include <crucible/Scene.hpp>

#include <vector>

class RigidBody {
private:
    btDefaultMotionState* motionState;
    btCompoundShape* colShape;

    btRigidBody* body;
    float mass;

public:
    RigidBody(Scene *scene, Transform transform, float mass);

    ~RigidBody();

    vec3 getPosition();

    quaternion getRotation();

    void setVelocity(vec3 velocity);

    vec3 getVelocity();

    void setAngularFactor(float factor);

    RigidBody *addBoxCollider(vec3 origin, vec3 halfExtents);

    RigidBody *addSphereCollider(vec3 origin, float radius);

    RigidBody *addCylinderCollider(vec3 origin, float radius, float height);

    RigidBody *addMeshCollider(vec3 origin, Mesh *mesh, vec3 scale);
};