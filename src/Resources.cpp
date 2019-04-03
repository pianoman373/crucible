#include <crucible/Resources.hpp>
#include <crucible/Shader.hpp>
#include <crucible/Texture.hpp>
#include <crucible/Path.hpp>

#include <map>
#include <string>

#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>

#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>


#include <fstream>
#include <sstream>

static Assimp::Importer importer;

static std::map<std::string, Texture> textureRegistry;
static std::map<std::string, Shader> shaderRegistry;
static std::map<std::string, AssimpFile> assimpFileRegistry;
static std::map<std::string, Material> materialRegistry;

static std::string readShader(std::ifstream &file, std::string directory) {
    std::string source, line;
    while (std::getline(file, line))
    {
        std::string prefix = "#include \"";
        if(line.substr(0, prefix.size()) == prefix) {
            //::cout << "found include" << std::endl;

            if (line.substr(line.size() - 1) == "\"") {
                //std::cout << line.substr(prefix.size(), (line.size() - 1) - prefix.size()) << std::endl;

                std::string includePath = directory + "/" + line.substr(prefix.size(), (line.size() - 1) - prefix.size());
                std::ifstream includeFile(includePath);
                if (includeFile.is_open())
                {
                    source += readShader(includeFile, directory);
                }
                includeFile.close();
            }
        }
        else {
            source += line + "\n";
        }
    }
    return source;
}

namespace Resources {
    Texture &getTexture(const Path &path, bool pixelated) {
        if (textureRegistry.find(path) == textureRegistry.end()) {
            std::cout << "loading texture: " << path << std::endl;

            Texture texture;
            stbi_set_flip_vertically_on_load(false);
            int width, height, components;
            unsigned char* image = stbi_load(path.toString().c_str(), &width, &height, &components, STBI_rgb_alpha);


            if (image) {
                texture.load(image, width, height, pixelated, false, path);

                stbi_image_free(image);
            }
            else {
                std::cerr << "error loading texture: " << path << std::endl;
            }



            textureRegistry.insert(std::make_pair(path, texture));
        }
        return textureRegistry.at(path);
    }

    Shader &getShader(const Path &vertexShader, const Path &fragmentShader) {
        std::string key = vertexShader.toString()+fragmentShader.toString();

        if (shaderRegistry.find(key) == shaderRegistry.end()) {
            std::cout << "loading shader: " << vertexShader << ", " << fragmentShader << std::endl;

            Shader shader;
            Path directory = vertexShader.getParent();

            std::ifstream vertexStream(vertexShader);
            std::ifstream fragmentStream(fragmentShader);

            if (vertexStream.is_open() && fragmentStream.is_open()) {
                std::string vertexCode = readShader(vertexStream, directory);
                std::string fragmentCode = readShader(fragmentStream, directory);


                vertexStream.close();
                fragmentStream.close();

                shader.load(vertexCode, fragmentCode);
            }
            else {
                std::cerr << "error loading shader: " << vertexShader << ", " << fragmentShader << std::endl;
            }



            shaderRegistry.insert(std::make_pair(key, shader));
        }
        return shaderRegistry.at(key);
    }

    Shader &getShader(const Path &vertexShader, const Path &fragmentShader, const Path &geometryShader) {
        std::string key = vertexShader.toString()+fragmentShader.toString()+geometryShader.toString();

        if (shaderRegistry.find(key) == shaderRegistry.end()) {
            std::cout << "loading shader: " << vertexShader << ", " << fragmentShader << ", " << geometryShader << std::endl;

            Shader shader;
            Path directory = vertexShader.getParent();

            std::ifstream vertexStream(vertexShader);
            std::ifstream fragmentStream(fragmentShader);
            std::ifstream geometryStream(geometryShader);

            if (vertexStream.is_open() && fragmentStream.is_open() && geometryStream.is_open()) {
                std::string vertexCode = readShader(vertexStream, directory);
                std::string fragmentCode = readShader(fragmentStream, directory);
                std::string geometryCode = readShader(geometryStream, directory);


                vertexStream.close();
                fragmentStream.close();
                geometryStream.close();

                shader.load(vertexCode, fragmentCode, geometryCode);
            }
            else {
                std::cerr << "error loading shader: " << vertexShader << ", " << fragmentShader << ", " << geometryShader << std::endl;
            }

            shaderRegistry.insert(std::make_pair(key, shader));
        }
        return shaderRegistry.at(key);
    }

    AssimpFile &getAssimpFile(const Path &path) {
        if (assimpFileRegistry.find(path) == assimpFileRegistry.end()) {
            std::cout << "loading Assimp file: " << path << std::endl;

            const aiScene* scene = importer.ReadFile(path.toString(), aiProcess_Triangulate | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices);

            if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
            {
                std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            }

            assimpFileRegistry.insert(std::make_pair(path, AssimpFile(scene)));
        }

        return assimpFileRegistry.at(path);
    }

    Material &getMaterial(const Path &path) {
        if (materialRegistry.find(path) == materialRegistry.end()) {
            std::cout << "loading material: " << path << std::endl;
            Material mat;

            Path wd = path.getParent();


            std::ifstream o(path);
            if (o.is_open()) {
                json j;

                o >> j;

                mat.fromJson(j, wd);
            }
            else {
                std::cerr << "error loading material: " << path <<  std::endl;
            }


            materialRegistry.insert(std::make_pair(path, mat));
        }

        return materialRegistry.at(path);
    }
}