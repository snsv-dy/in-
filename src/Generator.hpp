#ifndef _GENERATOR_HPP_
#define _GENERATOR_HPP_

#include "DynamcTexture.hpp"

class Generator {
protected:
	DynamcTexture* dynamc = nullptr;
	float fw = 0.0f;
	float fh = 0.0f;
	
	int width = 512;
	int height = width;
public:
	virtual void drawGui() =0;
	virtual void gen() =0;

	virtual void setTexture(DynamcTexture* dynamc) {
		printf("Interface exec %d\n", this);
		this->dynamc = dynamc;
		fw = (float)dynamc->width;
		fh = (float)dynamc->height;
		width = dynamc->width;
		height = dynamc->height;
	}

	virtual unsigned int& getTexture() {
		return dynamc->texture;
	}
	virtual ~Generator() {}
};

#endif