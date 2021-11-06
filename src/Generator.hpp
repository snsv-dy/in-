#ifndef _GENERATOR_HPP_
#define _GENERATOR_HPP_

#include "DynamcTexture.hpp"

class Generator {
	DynamcTexture* dynamc;
public:
	virtual void drawGui() =0;
	virtual void gen() =0;
	virtual unsigned int& getTexture() {
		return dynamc->texture;
	}
	virtual ~Generator() {}
};

#endif