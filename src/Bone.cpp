#include <crucible/Bone.hpp>
#include <crucible/Renderer.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Bone::Bone() {

}

Bone::Bone(std::string name, vec3 position, quaternion rotation) {
    this->name = name;
    this->position = position;
    this->rotation = rotation;
    this->startingPosition = position;
    this->startingRotation = rotation;
}

static void processNode(Bone &b, aiNode *node) {
    for (int i = 0; i < node->mNumChildren; i++) {
        aiNode *child = node->mChildren[i];

        aiVector3D position;
        aiQuaternion rotation;

        child->mTransformation.DecomposeNoScaling(rotation, position);

        std::cout << child->mName.C_Str() << std::endl;

        Bone &childBone = b.addChild(Bone(child->mName.C_Str(), vec3(position.x, position.y, position.z), quaternion(rotation.w, rotation.x, rotation.y, rotation.z)));

        processNode(childBone, child);
    }
}

Bone::Bone(std::string filename, std::string root) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices);

    aiNode *rootNode = scene->mRootNode->FindNode(root.c_str());

    aiVector3D position;
    aiQuaternion rotation;

    rootNode->mTransformation.DecomposeNoScaling(rotation, position);

    this->name = rootNode->mName.C_Str();
    this->position = vec3(position.x, position.y, position.z);
    this->startingPosition = this->position;
    this->rotation = quaternion(rotation.w, rotation.x, rotation.y, rotation.z);
    this->startingRotation = this->rotation;

    processNode(*this, rootNode);


}

Bone &Bone::addChild(Bone b) {
    children.push_back(b);

    return children.back();
}

mat4 Bone::getLocalTransform() {
    mat4 mat;
    mat = translate(mat, position);
    mat = mat * toMatrix(rotation);

    return mat;
};

mat4 Bone::getStartingTransform() {
    mat4 mat;
    mat = translate(mat, startingPosition);
    mat = mat * toMatrix(startingRotation);

    return mat;
};

std::vector<mat4> Bone::getStartingTransforms() {
    std::vector<mat4> transforms;

    mat4 local = getStartingTransform();

    transforms.push_back(local);

    for (int i = 0; i < children.size(); i++) {
        std::vector<mat4> childTransforms = children[i].getStartingTransforms();

        for (int j = 0; j < childTransforms.size(); j++) {
            transforms.push_back(local * childTransforms[j]);
        }
    }

    return transforms;
}


std::vector<mat4> Bone::getSkeletonTransforms() {
    std::vector<mat4> transforms;

    mat4 local = getLocalTransform();

    transforms.push_back(local);

    for (int i = 0; i < children.size(); i++) {
        std::vector<mat4> childTransforms = children[i].getSkeletonTransforms();

        for (int j = 0; j < childTransforms.size(); j++) {
            transforms.push_back(local * childTransforms[j]);
        }
    }

    return transforms;
}

std::vector<mat4> Bone::getSkinningTransforms() {
    std::vector<mat4> transforms = getSkeletonTransforms();
    std::vector<mat4> startingTransforms = getStartingTransforms();

    for (int i = 0; i < transforms.size(); i++) {
        transforms[i] =  transforms[i] * inverse(startingTransforms[i]);
    }

    return transforms;
}

static void renderMat4(mat4 mat) {
    Renderer::debug.renderDebugLine(vec3(mat * vec4(0.0f, 0.0f, 0.0f, 1.0f)), vec3(mat * vec4(0.1f, 0.0f, 0.0f, 1.0f)), vec3(1.0f, 0.0f, 0.0f)); //x
    Renderer::debug.renderDebugLine(vec3(mat * vec4(0.0f, 0.0f, 0.0f, 1.0f)), vec3(mat * vec4(0.0f, 0.1f, 0.0f, 1.0f)), vec3(0.0f, 1.0f, 0.0f)); //y
    Renderer::debug.renderDebugLine(vec3(mat * vec4(0.0f, 0.0f, 0.0f, 1.0f)), vec3(mat * vec4(0.0f, 0.0f, 0.1f, 1.0f)), vec3(0.0f, 0.0f, 1.0f)); //z
}

void Bone::debugDraw(mat4 parentTransform) {
    //Renderer::debug.renderDebugSphere(this->position, 0.1f, vec3(1.0f, 0.0f, 0.0f));

    //Renderer::debug.renderDebugSphere(vec3(getLocalTransform() * vec4(0.0f, 0.0f, 0.0f, 1.0f)), 0.1f, vec3(1.0f, 0.0f, 0.0f));

    mat4 local = parentTransform * getLocalTransform();

    renderMat4(local);

    for (int i = 0; i < children.size(); i++) {
        Renderer::debug.renderDebugLine(vec3(local * vec4(0.0f, 0.0f, 0.0f, 1.0f)), vec3(local * vec4(children[i].position, 1.0f)), vec3(1.0f, 1.0f, 1.0f));


        children[i].debugDraw(local);
    }
}