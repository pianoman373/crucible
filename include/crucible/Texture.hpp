#pragma once

#include <crucible/Math.hpp>
#include <string>

class Texture {
private:
    unsigned int id;

	std::string filepath;
public:
    void load(const char *file, bool pixelated=false);

    void loadFromSingleColor(vec4 color);

    void bind(unsigned int unit = 0);

    unsigned int getID();

	void setID(unsigned int id);

	std::string getFilepath();
};

class Cubemap {
private:
    unsigned int id;

public:
    void load(const char *file1, const char *file2, const char *file3, const char *file4, const char *file5, const char *file6);

	void loadEquirectangular(std::string file, int resolution=512);

    void bind(unsigned int unit = 0);

    unsigned int getID();

	void setID(unsigned int id);
};
