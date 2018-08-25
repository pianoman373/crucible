#pragma once

#include <crucible/Math.hpp>
#include <string>

class Texture {
private:
    unsigned int id;

	std::string filepath;
public:
    void load(const std::string &file, bool pixelated=false);

    void loadFromSingleColor(const vec4 &color);

    void bind(unsigned int unit = 0) const;

    unsigned int getID() const;

	void setID(unsigned int id);

	std::string getFilepath() const;

	void destroy();
};

class Cubemap {
private:
    unsigned int id;

public:
    void load(const std::string &file1, const std::string &file2, const std::string &file3, const std::string &file4,
			  const std::string &file5, const std::string &file6);

	void loadEquirectangular(const std::string &file, int resolution = 512);

    void bind(unsigned int unit = 0) const;

    unsigned int getID() const;

	void setID(unsigned int id);
};
