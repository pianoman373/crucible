#pragma once

#include <vector>
#include <crucible/Texture.hpp>

class Framebuffer {
private:
   
    unsigned int rbo;
    unsigned int texture;
    int width;
    int height;

	std::vector<Texture> attachments;
	int numAttachments = 0;

public:
	unsigned int fbo;

	void setup(int width, int height);

	void attachTexture(unsigned int internalFormat, unsigned int format, unsigned int type);

	void attachRBO();

    /**
     * Initializes this framebuffer with data for shadow maps.
     */
    void setupShadow(int width, int height);

    /**
     * Binds the actual framebuffer (makes all render calls draw do it.
     */
    void bind();

	Texture getAttachment(int num);

    /**
     * Binds the texture of this framebuffer just like binding a normal Texture object.
     */
    void bindTexture(int unit = 0);

    int getTextureID();

    int getWidth();

    int getHeight();
};
