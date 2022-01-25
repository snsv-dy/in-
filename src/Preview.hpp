#ifndef _PREVIEW_HPP_
#define _PREVIEW_HPP_

#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include "UiNode.hpp"
#include "ColorGenerator.hpp"

using namespace std;

class Preview {
	unsigned int fbo = 0;
	unsigned int rbo = 0;
public:
	const unsigned int fb_size = 2000;
	unsigned int fbTexture = 0;
	glm::mat4 projection = glm::perspective(glm::radians(55.0f), 1.0f, 0.1f, 1000.0f);
	glm::vec2 cameraRotation = glm::vec2(0.0f);
	glm::vec3 cameraOrigin = glm::vec3(0.0f, 0.0f, -4.0f);
	const float cameraScrollSpeed = 0.1f;
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 view = glm::lookAt(cameraOrigin, -cameraOrigin, cameraUp);
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.5f, 0.0f)); // Moved model 0.5 down, so that model will be centered in preview.
	glm::vec3 lightPos = glm::vec3(0.f, 10.f, 0.f);

	void updateMovement();

	Preview();
	void draw(const shared_ptr<UiNode>& node, unsigned int colorFlagLocation, unsigned int VAO, unsigned int gridTrigCount, unsigned int projectionLocation, unsigned int viewLocation, unsigned int modelLocation, unsigned int lightPosLocation);
	void setLightPos();
	~Preview();
};

#endif