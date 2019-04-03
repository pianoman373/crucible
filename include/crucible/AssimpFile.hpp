#pragma once

#include <crucible/Bone.hpp>
#include <crucible/Model.hpp>
#include <crucible/Animation.hpp>

class Path;
class aiScene;
class aiNode;

class AssimpFile {
private:
    const aiScene* scene = nullptr;

    void processNode(Bone &b, aiNode *node);

public:
    AssimpFile();

    AssimpFile(const aiScene *scene);

    ~AssimpFile();

    Bone getSkeleton();

    Mesh getMesh(int index=0);

    int numMeshes();

    Animation getAnimation();
};