#pragma once

#include <crucible/Mesh.hpp>

class Primitives {
public:
    static void torus(Mesh &m, float r1, float r2, int numSteps1, int numSteps2);

	static void sphere(Mesh &m, unsigned int xSegments, unsigned int ySegments);

    static void sprite(Mesh &m);

	static void framebuffer(Mesh &m);

    static void cube(Mesh &m);

    static void skybox(Mesh &m);
};