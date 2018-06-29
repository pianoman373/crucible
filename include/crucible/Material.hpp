#pragma once

#include <map>
#include <string>

#include <crucible/Math.hpp>
#include <crucible/Shader.hpp>
#include <crucible/Texture.hpp>

#include <json.hpp>
using nlohmann::json;

struct UniformTexture {
    Texture tex;
    unsigned int unit;
};

/**
 * Stores a list of uniforms to pass on to a shader.
 */
class Material {
private:
    Shader shader;



    std::map<std::string, UniformTexture> textures;

    std::map<std::string, vec3> vec3Uniforms;

    std::map<std::string, float> floatUniforms;

    std::map<std::string, bool> boolUniforms;

public:
	std::string name;

    Material();

    void fromJson(json j, std::string workingDirectory);

    json toJson(std::string workingDirectory);

    void loadFile(std::string file);

    void saveFile(std::string file);

	void setDefaultPBRUniforms();

    //all combinations of textures vs floats etc
    void setPBRUniforms(vec3 albedo, float roughness, float metallic);

    void setPBRUniforms(Texture albedo, float roughness, float metallic);

    void setPBRUniforms(Texture albedo, Texture roughness, float metallic);

    void setPBRUniforms(Texture albedo, float roughness, Texture metallic);

    void setPBRUniforms(Texture albedo, Texture roughness, Texture metallic);

    void setPBRUniforms(vec3 albedo, Texture roughness, float metallic);

    void setPBRUniforms(vec3 albedo, float roughness, Texture metallic);

    void setPBRUniforms(vec3 albedo, Texture roughness, Texture metallic);

	//with normal maps
	void setPBRUniforms(vec3 albedo, float roughness, float metallic, Texture normal);

	void setPBRUniforms(Texture albedo, float roughness, float metallic, Texture normal);

	void setPBRUniforms(Texture albedo, Texture roughness, float metallic, Texture normal);

	void setPBRUniforms(Texture albedo, float roughness, Texture metallic, Texture normal);

	void setPBRUniforms(Texture albedo, Texture roughness, Texture metallic, Texture normal);

	void setPBRUniforms(vec3 albedo, Texture roughness, float metallic, Texture normal);

	void setPBRUniforms(vec3 albedo, float roughness, Texture metallic, Texture normal);

	void setPBRUniforms(vec3 albedo, Texture roughness, Texture metallic, Texture normal);



    void setUniformTexture(std::string name, Texture value, unsigned int unit = 0);

    void setUniformVec3(std::string name, vec3 value);

    void setUniformFloat(std::string name, float value);

    void setUniformBool(std::string name, bool value);

    vec3 *getUniformVec3(std::string name);

    float *getUniformFloat(std::string name);

    bool *getUniformBool(std::string name);

    Texture *getUniformTexture(std::string name);

    void setShader(Shader shader);

    Shader getShader();

    std::map<std::string, UniformTexture> *getTextureUniforms();

    std::map<std::string, vec3> *getVec3Uniforms();

    std::map<std::string, float> *getFloatUniforms();

    std::map<std::string, bool> *getBoolUniforms();

    /**
     * Binds all specified uniforms to this material's specified shader (also binds specified textures).
     * Assumes currently bound shader is the same as the material's.
     */
    void bindUniforms();
};
