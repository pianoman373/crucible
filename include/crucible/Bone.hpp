#pragma once

#include <string>
#include <vector>

#include <crucible/Math.hpp>

class Bone {
public:
    std::vector<Bone> children;

    vec3 startingPosition;
    quaternion startingRotation;

    vec3 position;
    quaternion rotation;

    std::string name;

    Bone();

    Bone(std::string name, vec3 position, quaternion rotation);

    Bone(std::string filename, std::string root);

    Bone &addChild(Bone b);

    mat4 getLocalTransform();

    mat4 getStartingTransform();

    std::vector<mat4> getStartingTransforms();

    std::vector<mat4> getSkeletonTransforms();

    std::vector<mat4> getSkinningTransforms();

    void debugDraw(mat4 parentTransform=mat4());
};
