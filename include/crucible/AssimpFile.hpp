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

    std::vector<Mesh> meshes;

    void processNode(Bone &b, aiNode *node);

public:
    AssimpFile();

    ~AssimpFile();

    void load(const aiScene *scene);

    Bone getSkeleton();

    Mesh getMesh(unsigned int index=0);

    unsigned int numMeshes();

    Animation getAnimation();
};