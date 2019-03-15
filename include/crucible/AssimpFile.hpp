#pragma once

#include <crucible/Bone.hpp>
#include <crucible/Model.hpp>
#include <assimp/Importer.hpp>
#include <crucible/Animation.hpp>

class Path;
class aiScene;
class aiNode;

class AssimpFile {
private:
    const aiScene* scene = nullptr;
    Assimp::Importer importer;

    Path workingDirectory;


    void processNode(Bone &b, aiNode *node);

public:

    AssimpFile(const Path &path);

    ~AssimpFile();

    Bone getSkeleton(std::string rootNode);

    Bone getSkeletonNew();

    Model getModel(bool loadTextures=false);

    Animation getAnimation();
};