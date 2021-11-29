#ifndef _PREVIEW_HPP_
#define _PREVIEW_HPP_

#include <memory>
#include <glad/glad.h>

#include "UiNode.hpp"

using namespace std;

class Preview {
	unsigned int fbo = 0;
	const unsigned int fb_size = 2000;
	unsigned int rbo = 0;
public:
	unsigned int fbTexture = 0;

	Preview();
	void draw(const shared_ptr<UiNode>& node, unsigned int colorFlagLocation, unsigned int VAO, unsigned int gridTrigCount);
	~Preview();
};

#endif