#pragma once

#include <crucible/Bone.hpp>
#include <crucible/Model.hpp>
#include <crucible/Animation.hpp>
#include <crucible/Scene.hpp>

class Path;
class aiScene;
class aiNode;

class AssimpFile {
private:
    const aiScene* scene = nullptr;

    
    std::vector<Material> materials;
    std::vector<Mesh> meshes;
    std::vector<int> meshMaterialIndices;

    void processNode(Bone &b, aiNode *node);

public:
    

    AssimpFile();

    ~AssimpFile();

    void load(const aiScene *scene, Path workingDirectory);

    Bone getSkeleton();

    Mesh &getMesh(unsigned int index=0);

    unsigned int numMeshes();

    Animation getAnimation();

    void addToScene(Scene &scene);
};