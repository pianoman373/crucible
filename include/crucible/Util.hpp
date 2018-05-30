#pragma once

#include <crucible/Camera.hpp>

#define SID(string) Util::custom_simple_hash(string)

namespace Util {
    void updateSpaceCamera(Camera &cam);

    float rand();

    float rand(float min, float max);

	inline unsigned int custom_simple_hash(const std::string &str)
	{
		unsigned int hash = 0;

		for (auto& it : str) {
			// NOTE(Joey): be sure to use prime numbers
			hash = 37 * hash + 17 * static_cast<char>(it);
		}

		// NOTE(Joey): hash value will automatically wrap around 
		return hash;
	}
	// NOTE(Joey): supports c string literals
	inline unsigned int custom_simple_hash(const char* cStr)
	{
		std::string str(cStr);
		return custom_simple_hash(str);
	}
}