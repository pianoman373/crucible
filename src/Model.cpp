#include <crucible/Model.hpp>
#include <crucible/Renderer.hpp>
#include <crucible/Resources.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <algorithm>

void Model::addSubmesh(Mesh mesh, Material material, std::string name) {
    meshes.push_back(mesh);
    materials.push_back(material);
    names.push_back(name);
}

void Model::loadFile(std::string filename, bool loadTextures) {
    Assimp::Importer importer;
    std::replace(filename.begin(), filename.end(), '\\', '/');
	std::string workingDirectory = filename.substr(0, filename.find_last_of("/")) + "/";
    const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_GenSmoothNormals);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
    }

    for (int i = 0; i < scene->mNumMeshes; i++) {
        aiMesh *aMesh = scene->mMeshes[i];
		aiMaterial *aMaterial = scene->mMaterials[aMesh->mMaterialIndex];

		Material material;

		material.setShader(Renderer::standardShader);

		if (loadTextures) {
            aiString albedoFile;
            Texture albedo;
            aMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &albedoFile);
            std::string albedoPath = workingDirectory + std::string(albedoFile.C_Str());
            std::replace(albedoPath.begin(), albedoPath.end(), '\\', '/');
            albedo = Resources::getTexture(albedoPath);

            aiString normalFile;
            Texture normal;
            aMaterial->GetTexture(aiTextureType_NORMALS, 0, &normalFile);
            std::string normalPath = workingDirectory + std::string(normalFile.C_Str());
            std::replace(normalPath.begin(), normalPath.end(), '\\', '/');
            normal.load(normalPath.c_str());

            aiString metallicFile;
            Texture metallic;
            aMaterial->GetTexture(aiTextureType_SPECULAR, 0, &metallicFile);
            std::string metallicPath = workingDirectory + std::string(metallicFile.C_Str());
            std::replace(metallicPath.begin(), metallicPath.end(), '\\', '/');
            metallic = Resources::getTexture(metallicPath);

            aiString roughnessFile;
            Texture roughness;
            aMaterial->GetTexture(aiTextureType_SHININESS, 0, &roughnessFile);
            std::string roughnessPath = workingDirectory + std::string(roughnessFile.C_Str());
            std::replace(roughnessPath.begin(), roughnessPath.end(), '\\', '/');
            roughness = Resources::getTexture(roughnessPath);


            //std::cout << "metallic: " << metallicFile.C_Str() << std::endl;
            //std::cout << "roughness: " << roughnessFile.C_Str() << std::endl;


            material.setPBRUniforms(albedo, roughness, metallic, normal);
        }
        else {
            material.setPBRUniforms(vec3(0.8f), 0.5f, 0.0f);
		}

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

            if (aMesh->mTextureCoords[0]) {
                mesh.uvs[i] = vec2(aMesh->mTextureCoords[0][i].x, aMesh->mTextureCoords[0][i].y);
            }
        }

        for (unsigned int f = 0; f < aMesh->mNumFaces; ++f) {
            for (unsigned int i = 0; i < 3; ++i) {
                mesh.indices[f * 3 + i] = aMesh->mFaces[f].mIndices[i];
            }
        }

        mesh.generate();

        addSubmesh(mesh, material, std::string(aMesh->mName.C_Str()));
    }
}

void Model::clear() {
    meshes.clear();
    materials.clear();
}
