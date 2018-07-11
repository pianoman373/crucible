#pragma once

#include <vector>
#include <crucible/Texture.hpp>

class Framebuffer {
private:
   
    unsigned int rbo = 0;
    int width;
    int height;

	std::vector<Texture> attachments;
	int numAttachments = 0;

public:
	unsigned int fbo;

	void setup(int width, int height);

	void attachTexture(unsigned int internalFormat, unsigned int format, unsigned int type);

	/**
	 * Attaches a depth and stencil buffer.
	 */
	void attachRBO();

    /**
     * Initializes this framebuffer with data for shadow maps.
     */
    void attachShadow(int width, int height);

    /**
     * Binds the framebuffer (makes all render calls draw do it.
     */
    void bind();

	Texture getAttachment(int num);

    int getWidth();

    int getHeight();

    void destroy();
};
