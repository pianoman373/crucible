#include <crucible/Model.hpp>
#include <crucible/Renderer.hpp>
#include <crucible/Resources.hpp>
#include <crucible/Path.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <algorithm>
#include <fstream>

void Model::addSubmesh(const Mesh &mesh, const Material &material, const std::string &name) {
    materials.push_back(material);

    ModelNode node;
    node.mesh = mesh;
    node.materialIndex = materials.size();
    node.name = name;

    nodes.push_back(node);
}

void Model::importFile(std::string filename, bool loadTextures) {
    clear();

    Assimp::Importer importer;

    std::replace(filename.begin(), filename.end(), '\\', '/');
    std::string workingDirectory = filename.substr(0, filename.find_last_of("/")) + "/";
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices);


    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    }

    for (int i = 0; i < scene->mNumMaterials; i++) {
        aiMaterial *aMaterial = scene->mMaterials[i];

        Material material;

        material.setShader(Renderer::standardShader);
        material.setDefaultPBRUniforms();

        if (aMaterial->GetTextureCount(aiTextureType_DIFFUSE)) {
            aiString albedoFile;
            Texture albedo;
            aMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &albedoFile);
            std::string albedoPath = workingDirectory + std::string(albedoFile.C_Str());
            std::replace(albedoPath.begin(), albedoPath.end(), '\\', '/');
            albedo = Resources::getTexture(albedoPath);

            material.setUniformBool("albedoTextured", true);
            material.setUniformTexture("albedoTex", albedo, 0);
        }

        if (aMaterial->GetTextureCount(aiTextureType_NORMALS)) {
            aiString normalFile;
            Texture normal;
            aMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalFile);
            std::string normalPath = workingDirectory + std::string(normalFile.C_Str());
            std::replace(normalPath.begin(), normalPath.end(), '\\', '/');
            normal.load(normalPath.c_str());

            material.setUniformBool("normalTextured", true);
            material.setUniformTexture("normalTex", normal, 1);
        }

        if (aMaterial->GetTextureCount(aiTextureType_SPECULAR)) {
            aiString metallicFile;
            Texture metallic;
            aMaterial->GetTexture(aiTextureType_SPECULAR, 0, &metallicFile);
            std::string metallicPath = workingDirectory + std::string(metallicFile.C_Str());
            std::replace(metallicPath.begin(), metallicPath.end(), '\\', '/');
            metallic = Resources::getTexture(metallicPath);

            material.setUniformBool("metallicTextured", true);
            material.setUniformTexture("metallicTex", metallic, 2);
        }

        if (aMaterial->GetTextureCount(aiTextureType_SHININESS)) {
            aiString roughnessFile;
            Texture roughness;
            aMaterial->GetTexture(aiTextureType_SHININESS, 0, &roughnessFile);
            std::string roughnessPath = workingDirectory + std::string(roughnessFile.C_Str());
            std::replace(roughnessPath.begin(), roughnessPath.end(), '\\', '/');
            roughness = Resources::getTexture(roughnessPath);

            material.setUniformBool("roughnessTextured", true);
            material.setUniformTexture("roughnessTex", roughness, 3);
        }


        aiString materialName;
        aMaterial->Get(AI_MATKEY_NAME, materialName);
        materials.push_back(material);
    }

    for (int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh *aMesh = scene->mMeshes[i];

        Mesh mesh = Mesh();

        mesh.positions.resize(aMesh->mNumVertices);
        mesh.normals.resize(aMesh->mNumVertices);
        mesh.indices.resize(aMesh->mNumFaces * 3);
        mesh.tangents.resize(aMesh->mNumVertices);

        if (aMesh->mNumUVComponents > 0) {
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

        ModelNode node;
        node.mesh = mesh;
        node.mesh.generate();
        node.materialIndex = aMesh->mMaterialIndex;
        node.name = aMesh->mName.C_Str();

        nodes.push_back(node);
    }
}

void Model::openFile(std::string filename) {
    Path::format(filename);
    std::string workingDirectory  = Path::getWorkingDirectory(filename);

    json j;
    std::ifstream o(filename);
    o >> j;


    fromJson(j, workingDirectory);
}

void Model::fromJson(const json &j, const std::string &workingDirectory) {

    json jMaterials = j["materials"];

    for (int i = 0; i < jMaterials.size(); i++) {
        json jMaterial = jMaterials[i];

        Material mat;

        mat.setShader(Renderer::standardShader);
        mat.setDefaultPBRUniforms();
        mat.fromJson(jMaterial, workingDirectory);

        materials.push_back(mat);
    }


    json jMeshes = j["meshes"];

    for (int i = 0; i < jMeshes.size(); i++) {
        json jMesh = jMeshes[i];


        ModelNode node;
        node.mesh.fromJson(jMesh["data"]);
        node.mesh.generate();

        node.materialIndex = jMesh["materialIndex"];
        node.name = jMesh["name"];

        nodes.push_back(node);
    }
}

json Model::toJson(const std::string &workingDirectory) const {
   json j;

    for (int i = 0; i < nodes.size(); i++) {
        ModelNode node = nodes[i];
        json jMesh;
        jMesh["name"] = node.name;
        jMesh["data"] = node.mesh.toJson();
        jMesh["materialIndex"] = node.materialIndex;

        j["meshes"][i] = jMesh;
    }

    for (int i = 0; i < materials.size(); i++) {
        Material mat = materials[i];

        j["materials"][i]  = mat.toJson(workingDirectory);
    }

    return j;
}

void Model::clear() {
    nodes.clear();
    materials.clear();
}
