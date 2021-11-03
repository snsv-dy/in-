// Tasks:
// - Docking windows (Done)
// - Height value from texture

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <cstdio>

#include <imnodes/imnodes.h>
#include "NodeEditor.hpp"

#include "Shader.hpp"

int screen_width = 1200;
int screen_height = 1000;
const char* glsl_version = "#version 460";

int opengl_context(GLFWwindow* window);

void error_callback(int error, const char* msg) {
	fputs(msg, stderr);
}

std::pair<unsigned int, unsigned int> createBufferForGridSize(unsigned int size);
void setBufferData(unsigned int VAO, unsigned int VBO, float* data, unsigned int dataSize);
void setFlatGrid(float* data, unsigned int size);

int main() {
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}

	// Use most recent opengl version ;)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(screen_width, screen_height, "nodes test", NULL, NULL);

	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	opengl_context(window);

	glfwTerminate();

	return 0;
}


int opengl_context(GLFWwindow* window) {
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	NodeEditor node_editor;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	glViewport(0, 0, screen_width, screen_height);
	bool show_demo = true;

	float fparam = 17.0f;

	// 
	//	Buffer init (will be moved later)

	const std::string vsPath = "shaders/basic.vs";
	const std::string fsPath = "shaders/basic.fs";

	const int gridSize = 20;
	auto [VAO, VBO] = createBufferForGridSize(gridSize);
	const int gridTrigCount = (gridSize - 1) * (gridSize - 1) * 2;
	const int gridDataSize = gridTrigCount * 3 * 3; // (3 vertices 3 parameters of each vertex (x, y, z))
	printf("%d, %d, %d\n", gridSize, gridTrigCount, gridDataSize);
	float *gridData = new float[gridDataSize];
	setFlatGrid(gridData, gridSize);
	setBufferData(VAO, VBO, gridData, gridDataSize);
	



	Shader shader(vsPath, fsPath);
	// 
	//

	//
	// Framebuffer for rendering in window
	unsigned int fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	unsigned int fbTexture;
	glGenTextures(1, &fbTexture);
	glBindTexture(GL_TEXTURE_2D, fbTexture);
	const unsigned int fb_size = 1000;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb_size, fb_size, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbTexture, 0);

	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fb_size, fb_size);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	if(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
		// 🤔
		printf("Framebuffer Ok!\n");
	}
	//
	//
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::mat4 projection = glm::perspective(glm::radians(55.0f), 1.0f, 0.1f, 1000.0f);
	glm::vec2 cameraRotation = glm::vec2(0.0f);
	glm::vec3 cameraOrigin = glm::vec3(0.0f, 0.0f, -4.0f);
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 view = glm::lookAt(cameraOrigin, -cameraOrigin, cameraUp);
	// glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(45.0f), glm::vec3(1.0f, 0.5f, 0.0f));
	glm::mat4 model = glm::mat4(1.0f);

	unsigned int projectionLocation = glGetUniformLocation(shader.getProgram(), "projection");
	unsigned int viewLocation = glGetUniformLocation(shader.getProgram(), "view");
	unsigned int modelLocation = glGetUniformLocation(shader.getProgram(), "model");

	int lastWindowSize[2] = {-1, -1};
	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

	while (!glfwWindowShouldClose(window)) {
		

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuiID dock_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

		ImGui::Begin("Teren");
		// ImGui::Text("then will you still call me superman?");
		// ImGui::SliderFloat("If i'm alive", &fparam, 0.0f, 100.0f);
		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();

		if (lastWindowSize[0] != size.x || lastWindowSize[1] != size.y) {
			// change perspective
			projection = glm::perspective(glm::radians(55.0f), size.x / size.y, 0.1f, 1000.0f);

			lastWindowSize[0] = size.x;
			lastWindowSize[1] = size.y;
		}

		//
		// Model rotation
		if(ImGui::IsWindowFocused() && ImGui::IsMouseDragging(0)) {
			ImVec2 delta = ImGui::GetMouseDragDelta();
			const float mouseSensitivity = 0.5;
			cameraRotation.x -= delta.x * mouseSensitivity;
			cameraRotation.y += delta.y * mouseSensitivity;
			if (cameraRotation.y <= -89.9f) {
				cameraRotation.y = -89.9f;
			} else if (cameraRotation.y >= 89.9f) {
				cameraRotation.y = 89.9f;
			}

			glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(cameraRotation.x), glm::vec3(0.0f, 1.0f, 0.0f));
			rotationMatrix = glm::rotate(rotationMatrix, glm::radians(cameraRotation.y), glm::vec3(1.0f, 0.0f, 0.0f));
			glm::vec3 cameraPos = glm::vec3(rotationMatrix * glm::vec4(cameraOrigin, 1.0f));
			view = glm::lookAt(cameraPos, -cameraPos, cameraUp);
			ImGui::ResetMouseDragDelta();
		}
		//
		//

		//
		// 3d drawing
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, fb_size, fb_size);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAO);
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawArrays(GL_TRIANGLES, 0, gridTrigCount * 3);
		// glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// 3d drawing end
		//

		ImGui::GetWindowDrawList()->AddImage((void *)(intptr_t)fbTexture, ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(0, 1), ImVec2(1, 0)); // uv changed (imgui assumes that 0,0 is top left, and opengl bottom left).
		ImGui::End();

		ImGui::Begin("Paramz");
		ImGui::Text("pos: %f %f", pos.x, pos.y);
		ImGui::Text("size: %f %f", size.x, size.y);
		ImGui::Checkbox("keyCtrl", &ImGui::GetIO().KeyCtrl);
		ImGui::Text("Links size: %d\n", node_editor.getLinksSize());
		ImGui::Text("Camera rotation: %d\n", node_editor.getLinksSize());
		ImGui::Text("cameraRotation: %2.2f %2.2f\n", cameraRotation.x, cameraRotation.y);
		ImGui::Text("cameraOrigin: %2.2f %2.2f %2.2f\n", cameraOrigin.x, cameraOrigin.y, cameraOrigin.z);
		ImGui::Text("cameraUp: %2.2f %2.2f %2.2f\n", cameraUp.x, cameraUp.y, cameraUp.z);
		ImGui::End();

		//
		// Node editor
		node_editor.draw();
		//
		//

		ImGui::End();
		
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Windows outside main window handling.
		// if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		// 	GLFWwindow* backup_current_context = glfwGetCurrentContext();
		// 	ImGui::UpdatePlatformWindows();
		// 	ImGui::RenderPlatformWindowsDefault();
		// 	glfwMakeContextCurrent(backup_current_context);
		// }

		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
	glDeleteFramebuffers(1, &fbo);
	glDeleteTextures(1, &fbTexture);
	delete[] gridData;

	ImGui_ImplOpenGL3_Shutdown();
	ImGui::DestroyContext();
}

std::pair<unsigned int, unsigned int> createBufferForGridSize(unsigned int size) {
	unsigned int VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, (size - 1) * (size - 1) * 2 * 3 * 3 * sizeof(float), nullptr, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	
	glBindVertexArray(0);

	return {VAO, VBO};
}


void setBufferData(unsigned int VAO, unsigned int VBO, float* data, unsigned int dataSize) {
	// glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	
	// glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, dataSize * sizeof(float), data, GL_STATIC_DRAW);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);
	
	glBindVertexArray(0);
}


// Data must have at least (size * size * 3) length.
void setFlatGrid(float* data, unsigned int size) {
	int index = 0;
	float gridUnit = 2.0f / (size - 1); // Size of individual square.
	for (int y = 0; y < size - 1; y++) {
		for (int x = 0; x < size - 1; x++) {
			// float fx = x * gridUnit - 0.5f * size;
			// float fy = y * gridUnit - 0.5f * size;
			float fx = x * gridUnit - 1.0f;
			float fy = y * gridUnit - 1.0f;

			data[index] = fx;
			data[index + 1] = 0.0f;
			data[index + 2] = fy;

			data[index + 3] = fx;
			data[index + 4] = 0.0f;
			data[index + 5] = fy + gridUnit;

			data[index + 6] = fx + gridUnit;
			data[index + 7] = 0.0f;
			data[index + 8] = fy;

			data[index + 9] = fx + gridUnit;
			data[index + 10] = 0.0f;
			data[index + 11] = fy;
			
			data[index + 12] = fx;
			data[index + 13] = 0.0f;
			data[index + 14] = fy + gridUnit;

			data[index + 15] = fx + gridUnit;
			data[index + 16] = 0.0f;
			data[index + 17] = fy + gridUnit;

			// for (int i = 0; i < 18; i++) {
			// 	printf("%2.2f ", data[index + i]);
			// }
			// printf("\n");

			index += 18;
		}
	}
}