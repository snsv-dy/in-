#include "Preview.hpp"

Preview::Preview() {
	//
	// Framebuffer for rendering in window
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_size, fb_size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture, 0);

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fb_size, fb_size);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		// 🤔
		printf("Framebuffer Ok!\n");
	} else {
		printf("Framebuffer error?\n");
	}
	//
	//
	glBindFramebuffer(GL_FRAMEBUFFER, 0);	
}

void Preview::draw(const shared_ptr<UiNode>& node, unsigned int colorFlagLocation, unsigned int VAO, unsigned int gridTrigCount) {
	//
	// 3d drawing
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, fb_size, fb_size);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// shader.use();
	// glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
	// glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
	// glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

	// glUniform1i(heightLocation, 0);
	// glUniform1i(colorLocation, 1);
	if (node != nullptr) {
		glUniform1i(colorFlagLocation, !node->dynamc.monochrome);

		if (node->dynamc.monochrome) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, node->dynamc.texture);
		} else {
			ColorGenerator* generator = (ColorGenerator*)node->generator.get();
			generator->bindTextures();
		}
	} else {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	glBindVertexArray(VAO);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_TRIANGLES, 0, gridTrigCount * 3);
	// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// 3d drawing end
	//
}

Preview::~Preview() {
	glDeleteRenderbuffers(1, &rbo);
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &fbTexture);
}