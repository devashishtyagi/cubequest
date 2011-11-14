/*
 * fbo.h
 *
 *  Created on: Nov 13, 2011
 *      Author: devashish
 */

#ifndef FBO_H_
#define FBO_H_

#include <GL/glew.h>
#include <iostream>

class fbo {
unsigned int id;
unsigned int depth_id;
unsigned int texture_id;
int width, height;
public:
	fbo(int w, int h);
	virtual ~fbo();
	unsigned int getID();
	unsigned int getDepthID();
	unsigned int getTextureID();
	void initFrameBuffer();
	void initDepthBuffer();
	void initTextureBuffer();
	void bind();
	void unbind();
	void textureBind();
	void textureUnbind();
};



#endif /* FBO_H_ */
