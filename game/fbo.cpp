/*
 * fbo.cpp
 *
 *  Created on: Nov 13, 2011
 *      Author: devashish
 */

#include "fbo.h"

fbo::fbo(int w, int h) {
	// TODO Auto-generated constructor stub
	id = 0;
	depth_id = 0;
	texture_id = 0;
	width  = w;
	height = h;
}

fbo::~fbo() {
	// TODO Auto-generated destructor stub
	this->textureUnbind();
	this->unbind();
}

unsigned int fbo::getID(void){
	return id;
}

unsigned int fbo::getDepthID(void){
	return depth_id;
}

unsigned int fbo::getTextureID(void){
	return texture_id;
}

void fbo::initDepthBuffer(void){
	glGenRenderbuffersEXT(1, &depth_id); // Generate one render buffer and store the ID in fbo_depth
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, depth_id); // Bind the fbo_depth render buffer
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, width, height); // Set the render buffer storage to be a depth component, with a width and height of the window
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_id); // Set the render buffer of this buffer to the depth buffer
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0); // Unbind the render buffer
}

void fbo::initTextureBuffer(void){
	glGenTextures(1, &texture_id); // Generate one texture
	glBindTexture(GL_TEXTURE_2D, texture_id); // Bind the texture fbo_texture

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); // Create a standard texture with the width and height of our window

	// Setup the basic texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// Unbind the texture
	glBindTexture(GL_TEXTURE_2D, 0);
}

void fbo::initFrameBuffer(void){
	initDepthBuffer(); // Initialize our frame buffer depth buffer

	initTextureBuffer(); // Initialize our frame buffer texture

	glGenFramebuffersEXT(1, &id); // Generate one frame buffer and store the ID in fbo
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id); // Bind our frame buffer
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture_id, 0); // Attach the texture fbo_texture to the color buffer in our frame buffer
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth_id); // Attach the depth buffer fbo_depth to our frame buffer
	GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT); // Check that status of our generated frame buffer
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) // If the frame buffer does not report back as complete
	{
	std::cout << "Couldn't create frame buffer" << std::endl; // Output an error to the console
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0); // Unbind our frame buffer
}

void fbo::bind(void){
	glActiveTexture(GL_TEXTURE0);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, id);
	glPushAttrib(GL_VIEWPORT_BIT | GL_ENABLE_BIT);
	glViewport(0,0, width, height);
}

void fbo::unbind(void){
	glPopAttrib();
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void fbo::textureBind(void){
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glGenerateMipmapEXT(GL_TEXTURE_2D);
}

void fbo::textureUnbind(void){
	glBindTexture(GL_TEXTURE_2D, 0);
}
