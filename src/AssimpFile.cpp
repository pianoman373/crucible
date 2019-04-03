#include <crucible/AssimpFile.hpp>
#include <crucible/Path.hpp>
#include <crucible/Resources.hpp>

#include <assimp/scene.h>
#include <assimp/postprocess.h>


void AssimpFile::processNode(Bone &b, aiNode *node) {
    for (int i = 0; i < node->mNumChildren; i++) {
        aiNode *child = node->mChildren[i];

        aiVector3D position;
        aiQuaternion rotation;

        child->mTransformation.DecomposeNoScaling(rotation, position);

        Bone &childBone = b.addChild(Bone(child->mName.C_Str(), vec3(position.x, position.y, position.z), quaternion(rotation.w, rotation.x, rotation.y, rotation.z)));

        processNode(childBone, child);
    }
}

AssimpFile::AssimpFile() {

}

AssimpFile::AssimpFile(const aiScene *scene) {
    this->scene = scene;
}

AssimpFile::~AssimpFile() {

}

Bone AssimpFile::getSkeleton() {
    Bone bone;

    if (scene) {
        aiNode *rootNode = scene->mRootNode->FindNode("root");

        aiVector3D position;
        aiQuaternion rotation;

        //std::cout << rootNode->mName.C_Str() << std::endl;

        rootNode->mTransformation.DecomposeNoScaling(rotation, position);

        bone.name = rootNode->mName.C_Str();
        bone.position = vec3(position.x, position.y, position.z);
        bone.startingPosition = bone.position;
        bone.rotation = quaternion(rotation.w, rotation.x, rotation.y, rotation.z);
        bone.startingRotation = bone.rotation;

        processNode(bone, rootNode);
    }

    return bone;
}

Mesh AssimpFile::getMesh(int index) {
    Mesh mesh;

    if (scene) {
        if (index >= 0 && index < scene->mNumMeshes) {
            aiMesh *aMesh = scene->mMeshes[index];

            mesh.positions.resize(aMesh->mNumVertices);
            mesh.normals.resize(aMesh->mNumVertices);
            mesh.indices.resize(aMesh->mNumFaces * 3);
            mesh.tangents.resize(aMesh->mNumVertices);

            if (aMesh->mNumUVComponents[0] > 0) {
                mesh.uvs.resize(aMesh->mNumVertices);
            }

            for (unsigned int i = 0; i < aMesh->mNumVertices; ++i) {
                mesh.positions[i] = vec3(aMesh->mVertices[i].x, aMesh->mVertices[i].y, aMesh->mVertices[i].z);
                mesh.normals[i] = vec3(aMesh->mNormals[i].x, aMesh->mNormals[i].y, aMesh->mNormals[i].z);
                mesh.tangents[i] = vec3(aMesh->mTangents[i].x, aMesh->mTangents[i].y, aMesh->mTangents[i].z);

                if (isnan(mesh.tangents[i].x) || isnan(mesh.tangents[i].y) ||  isnan(mesh.tangents[i].z)) {
                    mesh.tangents[i] = {0.0f, 0.0f, 0.0f};
                }

                if (aMesh->mTextureCoords[0]) {
                    mesh.uvs[i] = vec2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y);
                }
            }

            if (aMesh->mNumBones > 0) {
                mesh.boneIDs.resize(aMesh->mNumVertices);
                mesh.boneWeights.resize(aMesh->mNumVertices);

                std::vector<std::vector<int>> tempBoneIDs;
                tempBoneIDs.resize(aMesh->mNumVertices);

                std::vector<std::vector<float>> tempBoneWeights;
                tempBoneWeights.resize(aMesh->mNumVertices);



                for (int i = 0; i < aMesh->mNumBones; i++) {
                    aiBone *bone = aMesh->mBones[i];

                    std::cout << bone->mName.C_Str() << std::endl;

                    for (int j = 0; j < bone->mNumWeights; j++) {
                        aiVertexWeight weight = bone->mWeights[j];

                        tempBoneIDs[weight.mVertexId].push_back(i);
                        tempBoneWeights[weight.mVertexId].push_back(weight.mWeight);
                    }
                }

                for (int i = 0; i < tempBoneIDs.size(); i++) {
                    std::vector<int> idsAtVertex = tempBoneIDs[i];
                    std::vector<float> weightsAtVertex = tempBoneWeights[i];

                    int size = idsAtVertex.size();

                    vec4i boneID;
                    vec4 boneWeight;

                    if (size > 0) {
                        boneID.x = idsAtVertex[0];
                        boneWeight.x = weightsAtVertex[0];
                    }
                    if (size > 1) {
                        boneID.y = idsAtVertex[1];
                        boneWeight.y = weightsAtVertex[1];
                    }
                    if (size > 2) {
                        boneID.z = idsAtVertex[2];
                        boneWeight.z = weightsAtVertex[2];
                    }
                    if (size > 3) {
                        boneID.w = idsAtVertex[3];
                        boneWeight.w = weightsAtVertex[3];
                    }

                    mesh.boneIDs[i] = boneID;
                    mesh.boneWeights[i] = boneWeight;
                }
            }

            for (unsigned int f = 0; f < aMesh->mNumFaces; ++f) {
                for (unsigned int i = 0; i < 3; ++i) {
                    mesh.indices[f * 3 + i] = aMesh->mFaces[f].mIndices[i];
                }
            }

            mesh.generate();
        }
    }

    return mesh;
}

int AssimpFile::numMeshes() {
    return scene->mNumMeshes;
}


Animation AssimpFile::getAnimation() {
    Animation ret;

    if (scene) {
        aiAnimation* anim = scene->mAnimations[1];




        float ticks = (float)anim->mTicksPerSecond;
        ret.length = (float)anim->mDuration / ticks;



        for (int i = 1; i < anim->mNumChannels; i++) {
            aiNodeAnim* nodeAnim = anim->mChannels[i];



            auto &keyframes = ret.keyframes[nodeAnim->mNodeName.C_Str()];

//        std::cout << anim->mChannels[i]->mNodeName.C_Str() << std::endl;
//
//        std::cout << nodeAnim->mNumPositionKeys << std::endl;
//        std::cout << nodeAnim->mNumRotationKeys << std::endl;
//        std::cout << nodeAnim->mNumScalingKeys << std::endl;

            for (int j = 0; j < nodeAnim->mNumPositionKeys; j++) {
                Transform t;

                aiVectorKey posKey = nodeAnim->mPositionKeys[j];
                aiQuatKey rotKey = nodeAnim->mRotationKeys[j];
                aiVectorKey scaleKey = nodeAnim->mScalingKeys[j];

                t.position = vec3(posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
                t.rotation = quaternion(rotKey.mValue.w, rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z);
                t.scale = vec3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z);

                keyframes.push_back({(float)posKey.mTime / ticks, t});

            }
        }
    }

    return ret;
}