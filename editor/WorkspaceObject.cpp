#include "WorkspaceObject.hpp"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <crucible/Renderer.hpp>
#include <crucible/Path.hpp>
#include <json.hpp>
#include <fstream>

void WorkspaceObject::import(std::string filename) {
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

        aiString albedoFile;
        Texture albedo;
        aMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &albedoFile);
        std::string albedoPath = workingDirectory + std::string(albedoFile.C_Str());
        std::replace(albedoPath.begin(), albedoPath.end(), '\\', '/');
        albedo = Renderer::getTexture(albedoPath);

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
        metallic = Renderer::getTexture(metallicPath);

        aiString roughnessFile;
        Texture roughness;
        aMaterial->GetTexture(aiTextureType_SHININESS, 0, &roughnessFile);
        std::string roughnessPath = workingDirectory + std::string(roughnessFile.C_Str());
        std::replace(roughnessPath.begin(), roughnessPath.end(), '\\', '/');
        roughness = Renderer::getTexture(roughnessPath);

        material.setPBRUniforms(albedo, roughness, metallic, normal);

        aiString materialName;
        aMaterial->Get(AI_MATKEY_NAME, materialName);
        materials.push_back({material, materialName.C_Str()});
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

        meshes.push_back({mesh, aMesh->mName.C_Str(), aMesh->mMaterialIndex});
    }
}

void WorkspaceObject::open(std::string filename) {
    clear();

    Path::format(filename);

    std::string workingDirectory = Path::getWorkingDirectory(filename);

    std::cout << "opening: " << filename << std::endl;

    using json = nlohmann::json;

    json j;
    std::ifstream o(filename);
    o >> j;


    json jMaterials = j["materials"];

    for (int i = 0; i < jMaterials.size(); i++) {
        json jMaterial = jMaterials[i];

        Material mat;

        mat.setShader(Renderer::standardShader);
        mat.setUniformBool("albedoTextured", jMaterial["albedoTextured"]);
        if (jMaterial["albedoTextured"]) {
            Texture tex = Renderer::getTexture(Path::getFullPath(workingDirectory, jMaterial["albedoTex"]));
            std::cout << tex.getFilepath() << std::endl;
            mat.setUniformTexture("albedoTex", tex, 0);
        }
        else {
            mat.setUniformVec3("albedoColor", vec3(jMaterial["albedoColor"][0], jMaterial["albedoColor"][1], jMaterial["albedoColor"][2]));
        }

        mat.setUniformBool("roughnessTextured", jMaterial["roughnessTextured"]);
        mat.setUniformBool("invertRoughness", jMaterial["invertRoughness"]);
        mat.setUniformBool("roughnessMetallicAlpha", jMaterial["roughnessMetallicAlpha"]);
        if (jMaterial["roughnessTextured"]) {
            Texture tex = Renderer::getTexture(Path::getFullPath(workingDirectory, jMaterial["roughnessTex"]));
            mat.setUniformTexture("roughnessTex", tex, 1);
        }
        else {
            mat.setUniformFloat("roughnessColor", jMaterial["roughnessColor"]);
        }

        mat.setUniformBool("metallicTextured", jMaterial["metallicTextured"]);
        if (jMaterial["metallicTextured"]) {
            Texture tex = Renderer::getTexture(Path::getFullPath(workingDirectory, jMaterial["metallicTex"]));
            mat.setUniformTexture("metallicTex", tex, 2);
        }
        else {
            mat.setUniformFloat("metallicColor", jMaterial["metallicColor"]);
        }

        mat.setUniformBool("normalTextured", jMaterial["normalTextured"]);
        if (jMaterial["normalTextured"]) {
            Texture tex = Renderer::getTexture(Path::getFullPath(workingDirectory, jMaterial["normalTex"]));
            mat.setUniformTexture("normalTex", tex, 3);
        }

        materials.push_back({mat, jMaterial["name"]});
    }


    json jMeshes = j["meshes"];

    for (int i = 0; i < jMeshes.size(); i++) {
        json jMesh = jMeshes[i];

        Mesh m;
        m.fromstring(jMesh["data"]);
        m.generate();

        meshes.push_back({m, jMesh["name"], jMesh["materialIndex"]});
    }

}

void WorkspaceObject::render(int selection) {
    for (int i = 0; i < meshes.size(); i++) {
        if (i == selection) {
            Renderer::enableOutline();
        }

        Renderer::render(&meshes[i].mesh, &materials[meshes[i].materialIndex].material, Transform(vec3(), vec3(), vec3(1.0f)), AABB());

        if (i == selection) {
            Renderer::disableOutline();
        }

    }
}

void WorkspaceObject::save(std::string path) {
    std::string originalPath = path;
    Path::format(path);

    std::cout << Path::getRelativePath(path, "/Users/Joseph/Desktop/joseph-data-backup/git/crucible/resources/test/hi.png") << std::endl;

    using json = nlohmann::json;

    json j;

    for (int i = 0; i < meshes.size(); i++) {
        WorkspaceMesh wmesh = meshes[i];
        json jMesh;
        jMesh["name"] = wmesh.name;

        jMesh["data"] = wmesh.mesh.tostring();

        jMesh["materialIndex"] = wmesh.materialIndex;

        j["meshes"][i] = jMesh;
    }

    for (int i = 0; i < materials.size(); i++) {
        WorkspaceMaterial mat = materials[i];

        json jMaterial;

        jMaterial["name"] = mat.name;

        bool albedoTextured = *mat.material.getUniformBool("albedoTextured");
        bool roughnessTextured = *mat.material.getUniformBool("roughnessTextured");
        bool metallicTextured = *mat.material.getUniformBool("metallicTextured");
        bool normalTextured = *mat.material.getUniformBool("normalTextured");

        jMaterial["albedoTextured"] = albedoTextured;
        if (albedoTextured) {
            jMaterial["albedoTex"] = Path::getRelativePath(path, mat.material.getUniformTexture("albedoTex")->getFilepath());
        }
        else {
            vec3 albedoColor = *mat.material.getUniformVec3("albedoColor");
            jMaterial["albedoColor"] = {albedoColor.x, albedoColor.y, albedoColor.z};
        }

        jMaterial["roughnessTextured"] = roughnessTextured;
        jMaterial["invertRoughness"] = *mat.material.getUniformBool("invertRoughness");
        jMaterial["roughnessMetallicAlpha"] = *mat.material.getUniformBool("roughnessMetallicAlpha");
        if (roughnessTextured) {
            jMaterial["roughnessTex"] = Path::getRelativePath(path, mat.material.getUniformTexture("roughnessTex")->getFilepath());
        }
        else {
            jMaterial["roughnessColor"] = *mat.material.getUniformFloat("roughnessColor");
        }

        jMaterial["metallicTextured"] = metallicTextured;
        if (metallicTextured) {
            jMaterial["metallicTex"] = Path::getRelativePath(path, mat.material.getUniformTexture("metallicTex")->getFilepath());
        }
        else {
            jMaterial["metallicColor"] = *mat.material.getUniformFloat("metallicColor");
        }

        jMaterial["normalTextured"] = normalTextured;
        if (normalTextured) {
            jMaterial["normalTex"] = Path::getRelativePath(path, mat.material.getUniformTexture("normalTex")->getFilepath());
        }

        j["materials"][i] = jMaterial;
    }

    std::ofstream o(originalPath);
    o << std::setw(4) << j << std::endl;
}

void WorkspaceObject::clear() {
    meshes.clear();
    materials.clear();
}