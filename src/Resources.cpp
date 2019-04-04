#include <crucible/Resources.hpp>
#include <crucible/Shader.hpp>
#include <crucible/Texture.hpp>
#include <crucible/Path.hpp>
#include <crucible/Resource.h>
#include <crucible/Primitives.hpp>

#include <glad/glad.h>

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
static std::map<std::string, Shader> postProcessingShaderRegistry;
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


static void load() {
    Resources::standardShader.load(LOAD_RESOURCE(src_shaders_standard_vsh).data(), LOAD_RESOURCE(src_shaders_standard_fsh).data());
    Resources::eq2cubeShader.load(LOAD_RESOURCE(src_shaders_cubemap_vsh).data(), LOAD_RESOURCE(src_shaders_eq2cube_fsh).data());
    Resources::cubemapShader.load(LOAD_RESOURCE(src_shaders_cubemap_vsh).data(), LOAD_RESOURCE(src_shaders_cubemap_fsh).data());
    Resources::irradianceShader.load(LOAD_RESOURCE(src_shaders_cubemap_vsh).data(), LOAD_RESOURCE(src_shaders_irradiance_fsh).data());
    Resources::prefilterShader.load(LOAD_RESOURCE(src_shaders_cubemap_vsh).data(), LOAD_RESOURCE(src_shaders_prefilter_fsh).data());
    Resources::passthroughShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_passthrough_glsl).data());
    Resources::spriteShader.load(LOAD_RESOURCE(src_shaders_sprite_vsh).data(), LOAD_RESOURCE(src_shaders_sprite_fsh).data());
    Resources::textShader.load(LOAD_RESOURCE(src_shaders_text_vsh).data(), LOAD_RESOURCE(src_shaders_text_fsh).data());
    Resources::ShadowShader.load(LOAD_RESOURCE(src_shaders_shadow_vsh).data(), LOAD_RESOURCE(src_shaders_shadow_fsh).data());
    Resources::deferredShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_deferred_glsl).data());
    Resources::deferredAmbientShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_deferred_ambient_glsl).data());
    Resources::brdfShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_brdf_glsl).data());
    Resources::debugShader.load(LOAD_RESOURCE(src_shaders_debug_vsh).data(), LOAD_RESOURCE(src_shaders_debug_fsh).data());

    Resources::tonemapShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_tonemap_glsl).data());
    Resources::fxaaShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_fxaa_glsl).data());
    Resources::gaussianBlurShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_gaussianBlur_glsl).data());
    Resources::ssaoShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_ssao_glsl).data());
    Resources::ssaoBlurShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_ssaoBlur_glsl).data());
    Resources::ssrShader.loadPostProcessing(LOAD_RESOURCE(src_shaders_ssr_glsl).data());

    /*Resources::standardShader = Resources::getShader("src/shaders/standard.vsh", "src/shaders/standard.fsh");
    Resources::eq2cubeShader = Resources::getShader("src/shaders/cubemap.vsh", "src/shaders/eq2cube.fsh");
    Resources::cubemapShader = Resources::getShader("src/shaders/cubemap.vsh", "src/shaders/cubemap.fsh");
    Resources::irradianceShader = Resources::getShader("src/shaders/cubemap.vsh", "src/shaders/irradiance.fsh");
    Resources::prefilterShader = Resources::getShader("src/shaders/cubemap.vsh", "src/shaders/prefilter.fsh");
    Resources::passthroughShader = Resources::getPostProcessingShader("src/shaders/passthrough.glsl");
    Resources::spriteShader = Resources::getShader("src/shaders/sprite.vsh", "src/shaders/sprite.fsh");
    Resources::textShader = Resources::getShader("src/shaders/text.vsh", "src/shaders/text.fsh");
    Resources::ShadowShader = Resources::getShader("src/shaders/shadow.vsh", "src/shaders/shadow.fsh");
    Resources::deferredShader = Resources::getPostProcessingShader("src/shaders/deferred.glsl");
    Resources::deferredAmbientShader = Resources::getPostProcessingShader("src/shaders/deferred_ambient.glsl");
    Resources::brdfShader = Resources::getPostProcessingShader("src/shaders/brdf.glsl");
    Resources::debugShader = Resources::getShader("src/shaders/debug.vsh", "src/shaders/debug.fsh");

    Resources::tonemapShader = Resources::getPostProcessingShader("src/shaders/tonemap.glsl");
    Resources::fxaaShader = Resources::getPostProcessingShader("src/shaders/fxaa.glsl");
    Resources::gaussianBlurShader = Resources::getPostProcessingShader("src/shaders/gaussianBlur.glsl");
    Resources::ssaoShader = Resources::getPostProcessingShader("src/shaders/ssao.glsl");
    Resources::ssaoBlurShader = Resources::getPostProcessingShader("src/shaders/ssaoBlur.glsl");
    Resources::ssrShader = Resources::getPostProcessingShader("src/shaders/ssr.glsl");*/

    Resources::framebufferMesh = Primitives::framebuffer();
    Resources::cubemapMesh = Primitives::skybox();
    Resources::spriteMesh = Primitives::sprite();


    // create brdf texture
    unsigned int brdfLUTTexture;
    glGenTextures(1, &brdfLUTTexture);

    // pre-allocate enough memory for the LUT texture.
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned int captureFBO, captureRBO;
    glGenFramebuffers(1, &captureFBO);
    glGenRenderbuffers(1, &captureRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
    glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

    glViewport(0, 0, 512, 512);
    Resources::brdfShader.bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Resources::framebufferMesh.render();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &captureFBO);
    glDeleteRenderbuffers(1, &captureRBO);

    Resources::brdf.setID(brdfLUTTexture);
}


namespace Resources {
    Mesh cubemapMesh;
    Mesh framebufferMesh;
    Mesh spriteMesh;

    Shader standardShader;
    Shader eq2cubeShader;
    Shader cubemapShader;
    Shader irradianceShader;
    Shader prefilterShader;
    Shader brdfShader;
    Shader passthroughShader;
    Shader spriteShader;
    Shader textShader;
    Shader ShadowShader;
    Shader deferredShader;
    Shader deferredAmbientShader;
    Shader debugShader;

    Shader tonemapShader;
    Shader fxaaShader;
    Shader gaussianBlurShader;
    Shader ssaoShader;
    Shader ssaoBlurShader;
    Shader ssrShader;

    Texture brdf;

    void loadDefaultResources() {
        load();
    }

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

    Shader &getPostProcessingShader(const Path &path) {
        std::string key = path.toString();

        if (postProcessingShaderRegistry.find(key) == postProcessingShaderRegistry.end()) {
            std::cout << "loading post processing shader: " << path << std::endl;

            Shader shader;
            Path directory = path.getParent();

            std::ifstream stream(path);

            if (stream.is_open()) {
                std::string code = readShader(stream, directory);


                stream.close();

                shader.loadPostProcessing(code);
            }
            else {
                std::cerr << "error loading post processing shader: " << path << std::endl;
            }



            postProcessingShaderRegistry.insert(std::make_pair(key, shader));
        }
        return postProcessingShaderRegistry.at(key);
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