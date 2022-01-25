// Tasks:
// - Docking windows (Done)
// - Height value from texture

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <ImGuiFileDialog.h>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdlib>
#include <cstdio>

#include <imnodes/imnodes.h>


// #define STB_IMAGE_WRITE_IMPLEMENTATION
// #include <stb_image_write.h>

#include "NodeEditor.hpp"
#include "Shader.hpp"
// #include "DynamcTexture.hpp"
// #include "SinGen.hpp"
// #include "GradGen.hpp"
// #include "ColorGenerator.hpp"

#include "Preview.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

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
unsigned int loadTexture(const std::string& filename, unsigned int& width, unsigned int& height);
void exportObj(const std::string filename, DynamcTexture* texture);

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
	stbi_flip_vertically_on_write(1);

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

	const int gridSize = 500;
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
	const unsigned int fb_size = 2000;
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

	unsigned int projectionLocation = glGetUniformLocation(shader.getProgram(), "projection");
	unsigned int viewLocation = glGetUniformLocation(shader.getProgram(), "view");
	unsigned int modelLocation = glGetUniformLocation(shader.getProgram(), "model");
	unsigned int lightPosLocation = glGetUniformLocation(shader.getProgram(), "sunPos");

	unsigned int heightLocation = glGetUniformLocation(shader.getProgram(), "heightmapTexture");
	unsigned int colorLocation = glGetUniformLocation(shader.getProgram(), "colorTexture");
	unsigned int colorFlagLocation = glGetUniformLocation(shader.getProgram(), "colorData");

	int lastWindowSize[2] = {-1, -1};
	ImGui::GetIO().ConfigWindowsMoveFromTitleBarOnly = true;

	unsigned int hdata_width, hdata_heigth, hdata_channels;
	// unsigned char *tex_ptr = stbi_load("height.png", &hdata_width, &hdata_heigth, &hdata_channels, 0);
	unsigned int heightmap_texture = loadTexture("height.png", hdata_width, hdata_heigth);
	if (heightmap_texture == 0) {
		printf("No heightmap!\n");
	}

	int dw = 512;
	DynamcTexture dynamc(dw, dw);
	
	ImGui::GetStyle().ScaleAllSizes(1.5f);

	string project_name = "[untitled.json]";
	glfwSetWindowTitle(window, project_name.c_str());

	Preview default_preview;
	// Preview second_preview;
	map<int, shared_ptr<Preview>> previews;
	set<int> previewed_nodes;

	// bool save_dialog_opened = false;

	while (!glfwWindowShouldClose(window)) {
		

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGuiID dock_id = ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		
		UiNode* activeNode = nullptr;
		if (node_editor.selectedNode != nullptr) {
			activeNode = node_editor.selectedNode.get();
			if (activeNode->generator->genInprogress) {
				activeNode->generator->gen();
				glfwPostEmptyEvent();
			}
		}

		ImGui::BeginMainMenuBar();
		if (ImGui::MenuItem("Open project")) {
			ImGuiFileDialog::Instance()->OpenDialog("OpenProject", "Choose File", ".json,.*", ".");
		}

		if (ImGui::MenuItem("Save project", "CTRL+S")) {
			ImGuiFileDialog::Instance()->OpenDialog("SaveProject", "Choose File", ".json,.*", ".", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
			printf("save\n");
		}

		if (ImGui::MenuItem("Export heightmap")) {
			ImGuiFileDialog::Instance()->OpenDialog("ExportImage", "Choose File", ".png,.*", ".", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
		}

		if (ImGui::MenuItem("Export as obj model")) {
			if (activeNode != nullptr) {
				ImGuiFileDialog::Instance()->OpenDialog("ExportModel", "Choose File", ".obj,.*", ".", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
			}
		}

		if (ImGui::MenuItem("Set light position")) {
			printf("Setting light position\n");
			default_preview.setLightPos();
		}

		if (ImGui::MenuItem("Export view image")) {
			if (activeNode != nullptr) {
				ImGuiFileDialog::Instance()->OpenDialog("ExportView", "Choose File", ".png,.*", ".", "", 1, nullptr, ImGuiFileDialogFlags_ConfirmOverwrite);
			}
		}
		

		// ImGui::MenuItem("Zapisz jako");
		// ImGui::MenuItem("Ostatnie pliki");
		ImGui::EndMainMenuBar();

		ImGui::Begin("Terrain");
		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();

		// This is needed.
		if (lastWindowSize[0] != size.x || lastWindowSize[1] != size.y) {
			// change perspective
			default_preview.projection = glm::perspective(glm::radians(55.0f), size.x / size.y, 0.1f, 1000.0f);

			lastWindowSize[0] = size.x;
			lastWindowSize[1] = size.y;
		}

		//
		// Model rotation
		default_preview.updateMovement();
		// //
		// // 3d drawing

		shader.use();
		glUniform1i(heightLocation, 0);
		glUniform1i(colorLocation, 1);
		default_preview.draw(node_editor.selectedNode, colorFlagLocation, VAO, gridTrigCount, projectionLocation, viewLocation, modelLocation, lightPosLocation);

		ImGui::GetWindowDrawList()->AddImage((void *)(intptr_t)default_preview.fbTexture, ImVec2(pos.x, pos.y), ImVec2(pos.x + size.x, pos.y + size.y), ImVec2(0, 1), ImVec2(1, 0)); // uv changed (imgui assumes that 0,0 is top left, and opengl bottom left).
		ImGui::End(); // Teren


		if (!previewed_nodes.empty()) {
			// for (const int& id : previewed_nodes) {
			for (set<int>::iterator it = previewed_nodes.begin(); it != previewed_nodes.end();) {
				int id = *it;

				if (previews.find(id) == previews.end()) {
					continue;
				}

				char name[100] = "";
				sprintf(name, "texture %d", id);
				const char* window_id = (const char*)name;
				shared_ptr<UiNode> node = node_editor.getNode(id);

				// Turns out, windows should'n be created this way.
				bool opened = true;
				bool closed = ImGui::Begin(window_id, &opened);
				ImVec2 size = ImGui::GetContentRegionAvail();
				int img_size = size.x < size.y ? size.x : size.y;
				// pos.x += size.y < size.x ? size.x - size.y : 0;
				// pos.y += size.x < size.y ? size.y - size.x : 0;

				ImGui::BeginChild("3d", {img_size, img_size});

				// pos = ImGui::GetWindowPos();
				// size = ImGui::GetWindowSize();

				shader.use();

				glUniform1i(heightLocation, 0);
				glUniform1i(colorLocation, 1);

				if (previews.find(id) != previews.end()) {
					shared_ptr<Preview> preview = previews[id];
					preview->updateMovement();
					preview->draw(node, colorFlagLocation, VAO, gridTrigCount, projectionLocation, viewLocation, modelLocation, lightPosLocation);
					ImGui::Image((void *)(intptr_t)preview->fbTexture, {img_size, img_size}, ImVec2(0, 1), ImVec2(1, 0));
				} else {
					ImGui::Text("No preview");
				}
				ImGui::EndChild();

				// ImGui::SameLine();

				ImGui::BeginChild("2d", {img_size, img_size});
				// pos = ImGui::GetWindowPos();
				// size = ImGui::GetWindowSize();

				if (node != nullptr) {
					ImGui::Image((void *)(intptr_t)node->dynamc.texture, {img_size, img_size}, ImVec2(1, 1), ImVec2(0, 0));
				}

				ImGui::EndChild();
				ImGui::End();

				if (!opened) {
					it = previewed_nodes.erase(it);
					previews.erase(id);
				} else {
					it++;
				}
			}
		}

		ImGui::Begin("Parameters");
		// ImGui::Text("previews size: %d", previews.size());
		// ImGui::Text("previewed nodes size: %d", previewed_nodes.size());
		// ImGui::Text("pos: %f %f", pos.x, pos.y);
		// ImGui::Text("size: %f %f", size.x, size.y);
		// ImGui::Checkbox("keyCtrl", &ImGui::GetIO().KeyCtrl);
		// ImGui::Text("Links size: %d\n", node_editor.getLinksSize());
		// ImGui::Text("Camera rotation: %d\n", node_editor.getLinksSize());
		// ImGui::Text("cameraRotation: %2.2f %2.2f\n", default_preview.cameraRotation.x, default_preview.cameraRotation.y);
		// ImGui::Text("cameraOrigin: %2.2f %2.2f %2.2f\n", default_preview.cameraOrigin.x, default_preview.cameraOrigin.y, default_preview.cameraOrigin.z);
		// ImGui::Text("cameraUp: %2.2f %2.2f %2.2f\n", default_preview.cameraUp.x, default_preview.cameraUp.y, default_preview.cameraUp.z);
		// node_editor.debgz();
		// if (ImGui::Button("Save nodes")) {
		// 	node_editor.save();
		// }

		// if (ImGui::Button("Load nodes")) {
		// 	node_editor.load();
		// }

		// if (ImGui::SliderFloat("Divider", &dynamc.divider, 0.0f, 300.0f)) {
		// 	dynamc.gen();
		// }
		// sigen.drawGui();
		if (activeNode != nullptr) {
			if (activeNode->drawGui()) {
				node_editor.nodesChanged({activeNode->id});
			}
		} else {
			ImGui::Text("Select node to show its parameters");
		}
		ImGui::End(); // Paramz?

		// File dialogs are here because they modify node editor state and set activeNode (selectedNode in NodeEditor) 
		// to nullptr, and as a result program tries to use nonexistent object (not operating on raw pointer here in main
		// would probably fixed that too).
		if (ImGuiFileDialog::Instance()->Display("SaveProject")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				printf("File choosed `%s`\n", ImGuiFileDialog::Instance()->GetFilePathName().c_str());
				node_editor.save(ImGuiFileDialog::Instance()->GetFilePathName());
				project_name = "[" + ImGuiFileDialog::Instance()->GetCurrentFileName() + "]";
				glfwSetWindowTitle(window, project_name.c_str());
			}

			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("OpenProject")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				// printf("File choosed `%s`\n", ImGuiFileDialog::Instance()->GetFilePathName().c_str());
				node_editor.load(ImGuiFileDialog::Instance()->GetFilePathName());
				project_name = "[" + ImGuiFileDialog::Instance()->GetCurrentFileName() + "]";
				glfwSetWindowTitle(window, project_name.c_str());
			}

			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("ExportImage")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				if (activeNode != nullptr) {
					activeNode->dynamc.save(ImGuiFileDialog::Instance()->GetFilePathName().c_str());
				}
				// node_editor.load();
				// project_name = "[" + ImGuiFileDialog::Instance()->GetCurrentFileName() + "]";
				// glfwSetWindowTitle(window, project_name.c_str());
			}

			ImGuiFileDialog::Instance()->Close();
		}

		if (ImGuiFileDialog::Instance()->Display("ExportView")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				if (activeNode != nullptr) {
					// DynamcTexture tdynamc(default_preview.fb_size, default_preview.fb_size, false);

						unsigned char* tdata = new unsigned char[default_preview.fb_size * default_preview.fb_size * 3];

						// Load initial height
						glBindTexture(GL_TEXTURE_2D, default_preview.fbTexture);
						// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, tdata);
						glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, tdata);
						glBindTexture(GL_TEXTURE_2D, 0);

						// for (int y = 0; y < default_preview.fb_size; y++) {
						// 	for (int x = 0; x < default_preview.fb_size; x++) {
						// 		int index = (y * default_preview.fb_size + x);
						// 		tdynamc.data[index] = tdata[index * 4];
						// 		tdynamc.data[index + 1] = tdata[index * 4 + 1];
						// 		tdynamc.data[index + 2] = tdata[index * 4 + 2];
						// 	}
						// }
						// tdynamc.save(ImGuiFileDialog::Instance()->GetFilePathName().c_str());

					int write_result = stbi_write_png(ImGuiFileDialog::Instance()->GetFilePathName().c_str(), default_preview.fb_size, default_preview.fb_size, 3, tdata, default_preview.fb_size * 3);
					if (write_result == 0) {
						printf("Texture saving failed\n");
					}

						delete[] tdata;

						// dynamc->updateGL();
					// }
				}
			}

			ImGuiFileDialog::Instance()->Close();
		}


		if (ImGuiFileDialog::Instance()->Display("ExportModel")) {
			if (ImGuiFileDialog::Instance()->IsOk()) {
				if (activeNode != nullptr) {
					exportObj(ImGuiFileDialog::Instance()->GetFilePathName(), &activeNode->dynamc);
				}
			}

			ImGuiFileDialog::Instance()->Close();
		}

		//
		// Node editor
		bool new_preview = false;
		int preview_id = -1;
		node_editor.draw(&new_preview, &preview_id);

		if (new_preview) {
			// for (const int id : node_editor.previewedNodes) {
			if (auto it = previews.find(preview_id); it == previews.end()) {
				shared_ptr<Preview> preview = make_shared<Preview>();
				previews[preview_id] = preview;
				previewed_nodes.insert(preview_id);
			} else {
				printf("Found preview %d\n", preview_id);
			}
			// }
		}
		//
		//

		// ImGui::End(); // nodeeditor draw
		
		ImGui::Begin("Texture preview");
		pos = ImGui::GetWindowPos();
		size = ImGui::GetWindowSize();

		if (activeNode != nullptr) {
			// ImGui::Image((void *)activeNode->dynamc.texture, ImVec2(512, 512));
			int img_size = size.x < size.y ? size.x : size.y;
			pos.x += size.y < size.x ? (size.x - size.y) / 2 : 0;
			pos.y += size.x < size.y ? (size.y - size.x) / 2 : 0;
			
			if (activeNode->type_i == 5) {
				ErosionGenerator* gen = (ErosionGenerator*)activeNode->generator.get();
				// ImGui::GetWindowDrawList()->AddImage((void *)(intptr_t)gen->flow_texture, ImVec2(pos.x, pos.y), ImVec2(pos.x + img_size, pos.y + img_size), ImVec2(0, 1), ImVec2(1, 0)); // uv changed (imgui assumes that 0,0 is top left, and opengl bottom left).
				// ImGui::GetWindowDrawList()->AddImage((void *)(intptr_t)gen->water_texture, ImVec2(pos.x, pos.y), ImVec2(pos.x + img_size, pos.y + img_size), ImVec2(0, 1), ImVec2(1, 0), IM_COL32(255, 255, 0, 255)); // uv changed (imgui assumes that 0,0 is top left, and opengl bottom left).
				// ImGui::GetWindowDrawList()->AddImage((void *)(intptr_t)gen->compute_texture, ImVec2(pos.x, pos.y), ImVec2(pos.x + img_size, pos.y + img_size), ImVec2(0, 1), ImVec2(1, 0), IM_COL32(255, 0, 0, 255)); // uv changed (imgui assumes that 0,0 is top left, and opengl bottom left).
				// ImGui::Image((void *)(intptr_t)gen->water_texture, {img_size, img_size}, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 1.0, 0.0f, 1.0f));
				// ImGui::Image((void *)(intptr_t)gen->water_texture, {img_size, img_size}, ImVec2(0, 1), ImVec2(1, 0), ImVec4(0.0f, 0.0, 1.0f, 1.0f));
				ImGui::Image((void *)(intptr_t)gen->compute_texture, {img_size, img_size}, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 0.0, 0.0f, 1.0f));
				ImGui::Image((void *)(intptr_t)gen->debug_texture, {img_size, img_size}, ImVec2(0, 1), ImVec2(1, 0), ImVec4(1.0f, 0.0, 1.0f, 1.0f));
			} else {
				ImGui::GetWindowDrawList()->AddImage((void *)(intptr_t)activeNode->dynamc.texture, ImVec2(pos.x, pos.y), ImVec2(pos.x + img_size, pos.y + img_size), ImVec2(1, 1), ImVec2(0, 0)); // uv changed (imgui assumes that 0,0 is top left, and opengl bottom left).
			}
		}
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

// void genTexture(void *data, const unsigned int width, const unsigned int height) {

// }

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


unsigned int loadTexture(const std::string& filename, unsigned int& width, unsigned int& height) {
	int w, h, channels;
	
	unsigned char *tex_ptr = stbi_load(filename.c_str(), &w, &h, &channels, 4);
	
	if(tex_ptr == NULL){
		printf("Failed to load texture: \"%s\"!\n", filename);
		
		return 0;
	}
	
	width = w;
	height = h;

	printf("loaded image with dimensions %d %d\n", width, height);
	
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_ptr);
	glGenerateMipmap(GL_TEXTURE_2D);
	
	stbi_image_free(tex_ptr);
	
	return texture;
}

void exportObj(const std::string filename, DynamcTexture* texture) {
	if (texture != nullptr) {
		ofstream out_file (filename);
	if (out_file.is_open()) {
		
		// vertices
		float finc = 1.f / texture->width;
		float fy = 0.f;
		for (int y = 0; y < texture->height; y++, fy += finc) {
			float fx = 0.f;
			int yw = y * texture->width;
			for (int x = 0; x < texture->width; x++, fx += finc) {
				const float& val = texture->data[yw + x];
				out_file << std::fixed << std::setprecision(6) << "v " << fx << " " << val << " " << fy << "\n";
			}
		}
		
		out_file << "\n";

		// faces 
		// float finc = 1.f;
		// float fy = 0.f;
		for (int y = 0; y < texture->height - 1; y++) {
			int yw = y * texture->width;
			for (int x = 0; x < texture->width - 1; x++) {
				int index = yw + x + 1;
				out_file <<  "f " << (index + 1) << " " << (index + texture->width + 1) << " " << (index + texture->width) << "\n";
				out_file <<  "f " << (index + 1) << " " << (index + texture->width) << " " << (index) << "\n";
			}
		}
		if (out_file.bad()) {
			printf("[Obj] Out file bad\n");	
		}
		out_file.close();
	} else {
		printf("[Obj] Can't open file\n");
	}
	} else {
		printf("[Obj] Provided texture is null\n");
	}
}