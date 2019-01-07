#pragma once

#include <crucible/Math.hpp>
#include <string>
#include <crucible/Path.hpp>

class Texture {
private:
    unsigned int id;

	std::string filepath;
public:
    void load(const Path &file, bool pixelated=false);

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
    void load(const Path &file1, const Path &file2, const Path &file3, const Path &file4,
			  const Path &file5, const Path &file6);

	void loadEquirectangular(const Path &file, int resolution = 512);

    void bind(unsigned int unit = 0) const;

    unsigned int getID() const;

	void setID(unsigned int id);
};
