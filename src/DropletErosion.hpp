#ifndef _EROSION3_HPP_
#define _EROSION3_HPP_

#include <cmath>
#include <fstream>
#include <glad/glad.h>

#include <json.hpp>
#include <imgui.h>

#include "DynamcTexture.hpp"
#include "Generator.hpp"

// To do for now
// - pass and modify parameters in buffer instead of texture
// - check if it's possible to write/read to 2d texture in 1d shader

struct alignas(16) pstruct{
	float posx; float posy;
	float dirx; float diry;
	float speed;
	float water;
	float sediment;
	int alive;
};

typedef struct pstruct ParticleStruct;

class DropletErosion : public Generator {
private:
	int current_i = 0;
public:
	DynamcTexture* input1 = nullptr;
	int iter_per_gen = 1;
	int n_iterations = 100;
	int tex_w = 512;
	int tex_h = 512;

	GLuint writable_texture = 0;
	GLuint shader_program = 0;
	GLuint ssbo = 0;

	static constexpr int n_data_items = 100;
	ParticleStruct tempData[n_data_items];

	void initSsbo() {
		// glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		// glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo); // 0 to chyba to samo co w binding w glsl.
		ParticleStruct* dataPtr;
		dataPtr = (ParticleStruct*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ParticleStruct) * n_data_items, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
		if (dataPtr != nullptr) {
			for (int i = 0; i < n_data_items; i++) {
					ParticleStruct p;
					p.alive = 1;
					p.posx = (float)(rand() % tex_w);
					p.posy = (float)(rand() % tex_h);
					p.dirx = (rand() % 100 < 50) ? 1.0 : -1.0;
					p.diry = (rand() % 100 < 50) ? 1.0 : -1.0;
					p.speed = startSpeed;
					p.water = startWater;
					p.sediment = 0.0f;
					dataPtr[i] = p;
			}
			glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
			printf("Data inited\n");
		} else {
			printf("Sorry no star today\n");
		}
		// glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0); // 0 to chyba to samo co w binding w glsl.
	}

	DropletErosion() {
		printf("pstruct size: %llu\n", sizeof(ParticleStruct));
		createShader("shaders/erosion3.glsl");
		writable_texture = genTexture(GL_TEXTURE0, GL_READ_WRITE);
		printf("Writable tex: %u\n", writable_texture);

		// for (int i = 0; i < n_data_items; i++) {
		// 	ParticleStruct p;
		// 	p.alive = 1;
		// 	p.posx = (float)(rand() % tex_w);
		// 	p.posy = (float)(rand() % tex_h);
		// 	p.dirx = (rand() % 100 < 50) ? 1.0 : -1.0;
		// 	p.diry = (rand() % 100 < 50) ? 1.0 : -1.0;
		// 	p.speed = startSpeed;
		// 	p.water = startWater;
		// 	p.sediment = 0.0f;
		// 	// p.r = (rand() % 100) / 100.0f;
		// 	// p.g = (rand() % 100) / 100.0f;
		// 	// p.b = (rand() % 100) / 100.0f;
		// 	tempData[i] = p;
		// 	// {
		// 	// 	.alive = 1,
		// 	// 	(float)(rand() % tex_w),
		// 	// 	(float)(rand() % tex_h),
		// 	// 	(rand() % 100 < 50) ? 1.0 : -1.0,
		// 	// 	(rand() % 100 < 50) ? 1.0 : -1.0,
		// 	// 	(rand() % 100) / 100.0f,
		// 	// 	(rand() % 100) / 100.0f,
		// 	// 	(rand() % 100) / 100.0f
		// 	// };
		// }

		// SSBO		
		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo); // 0 to chyba to samo co w binding w glsl.
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ParticleStruct) * n_data_items, tempData, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		// initSsbo();
		// SSBO end
	}

	unsigned int genTexture(unsigned int textureId, unsigned int access=GL_WRITE_ONLY) {
		unsigned int texture;
		
		glGenTextures(1, &texture);
		// glActiveTexture(textureId);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		float borderColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
		glBindImageTexture(textureId - GL_TEXTURE0, texture, 0, GL_FALSE, 0, access, GL_RGBA32F);
		glBindTexture(GL_TEXTURE_2D, 0);

		return texture;
	}

	int createShader(const std::string& path) {
		std::ifstream file(path);
		if (file.is_open()) {
			std::stringstream buffer;
			buffer << file.rdbuf();

			std::string vsText = buffer.str();
			const char* cbuffer = vsText.c_str();
			
			
			GLuint ray_shader = glCreateShader(GL_COMPUTE_SHADER);
			glShaderSource(ray_shader, 1, &cbuffer, NULL);
			glCompileShader(ray_shader);
			// check for compilation errors as per normal here
			int success;
			glGetShaderiv(ray_shader, GL_COMPILE_STATUS, &success);
			if(!success) {
				printf("Some shader failed: %s\n", path.c_str());
				char infoLog[512];
				glGetShaderInfoLog(ray_shader, 512, NULL, infoLog);
				printf("%s\n", infoLog);
				return -1;
			}

			shader_program = glCreateProgram();
			glAttachShader(shader_program, ray_shader);
			glLinkProgram(shader_program);
			// check for linking errors and validate program as per normal here
			// int success;
			glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
			if(!success) {
				char error_msg[512];
				glGetProgramInfoLog(shader_program, 512, NULL, error_msg);
				printf("prorgam log:\n%s\n", error_msg);
				shader_program = 0;
				printf("succ = %d\n", success);
			}

			// height_input_location = glGetUniformLocation(shader_program, "heightmap_input");

			return 0;
		}

		return -2;
	}

	bool drawGui() {
		bool changed = false;

		ImGui::DragInt("iterations", &n_iterations);
		// ImGui::DragInt("iterations multiplier", &iter_per_gen);
		
		// ImGui::DragFloat("capacityParam", &capacityParam, 0.01f);
		// ImGui::DragFloat("minCapacity", &minCapacity, 0.01f);
		// ImGui::DragFloat("depositionRate", &depositionRate, 0.01f);
		// ImGui::DragFloat("erosionRate", &erosionRate, 0.01f);
		// ImGui::DragFloat("evaporationRate", &evaporationRate, 0.01f);
		// ImGui::DragFloat("gravity", &gravity, 0.01f);
		// ImGui::DragFloat("inertia", &inertia, 0.01f);
		// ImGui::Separator();
		// ImGui::DragFloat("startWater", &startWater, 0.01f);
		// ImGui::DragFloat("startSpeed", &startSpeed, 0.01f);

		ImGui::Text("Progress");
		ImGui::ProgressBar(genProgress);

		bool disablebeg = false;
		if (genInprogress) {
			ImGui::BeginDisabled();
			disablebeg = true;
		}

		if (ImGui::Button("generate")) {
			gen();
			changed = true;
		}

		if (disablebeg) {
			ImGui::EndDisabled();
		}

		if (ImGui::Button("Stop")) {
			stopGeneration();
		}

		return changed;
	}

	void copyHeightFromResult() {
		// glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		// glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo); // 0 to chyba to samo co w binding w glsl.
		// GLfloat* dataPtr;
		// dataPtr = (GLfloat*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int) * n_data_items, GL_MAP_READ_BIT);
		// if (dataPtr != nullptr) {
		// 	printf("data from gpu: ");
		// 	for (int i = 0; i < n_data_items; i++) {
		// 		printf("%2.2f, ", dataPtr[i]);
		// 	}
		// 	printf("\n");
		// 	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		// } else {
		// 	printf("Sorry no star today\n");
		// }
		// glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0); // 0 to chyba to samo co w binding w glsl.

		float* tdata = new float[512 * 512 * 4];

		// Load initial height
		glBindTexture(GL_TEXTURE_2D, writable_texture);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, tdata);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, tdata);
		glBindTexture(GL_TEXTURE_2D, 0);

		for (int y = 0; y < 512; y++) {
			for (int x = 0; x < 512; x++) {
				int index = (y * 512 + x);
				// float fx = x / 512.0f;
				// float fy = y / 512.0f;
				// dynamc->data[index] = fx + fy;//tdata[index * 4];
				dynamc->data[index] = tdata[index*4];
			}
		}

		delete[] tdata;

		dynamc->updateGL();
	}

	bool gradient(float* map, float posx, float posy, float* return_data) {
		int x = (int)posx;
		int y = (int)posy;
		int mapw = 512;
		if (x > 0 && x < 511 && y > 0 && y < 511) {
			float NW = map[y * mapw + x];
			float NE = map[y * mapw + x + 1];
			float SW = map[(y + 1) * mapw + x];
			float SE = map[(y + 1) * mapw + x + 1];

			float lx = posx - floor(posx);
			float ly = posy - floor(posy);

			float gradx = (NE - NW) * (1 - ly) + (SE - SW) * ly;
			float grady = (SW - NW) * (1 - lx) + (SE - NE) * lx;

			float height = NW * (1 - lx) * (1 - ly) + NE * lx * (1 - ly) + SW * (1 - lx) * ly + SE * lx * ly;

			return_data[0] = gradx;
			return_data[1] = grady;
			return_data[2] = height;

			return true;
		}

		return_data[0] = 0.0f;
		return_data[1] = 0.0f;
		return_data[2] = 0.0f;

		return false;
	}

	float capacityParam = 4.0f; 
	float minCapacity = 0.01f;
	float depositionRate = 0.3f;
	float erosionRate = 0.3f;
	float evaporationRate = 0.02f;
	float gravity = 4.0f;
	float inertia = 0.05f;

	float startWater = 1.0f;
	float startSpeed = 1.0f;


	float* dataPtr = nullptr;
	float w = 0.0f;
	float h = 0.0f;
	void preIterationSetup() {
		dataPtr = dynamc->data;
		w = (float)dynamc->width;
		h = (float)dynamc->height;
		// copy texture from input
		for (int y = 0; y < dynamc->height; y++) {
			for (int x = 0; x < dynamc->width; x++) {
				int index = y * dynamc->width + x;
				dataPtr[index] = input1->data[index];
			}
		}

		// Load initial height
		glBindTexture(GL_TEXTURE_2D, writable_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RED, GL_FLOAT, dataPtr);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void iteration() {
		printf("iteration: %d\n", current_i);


		glUseProgram(shader_program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, writable_texture);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo); // 0 to chyba to samo co w binding w glsl.

		// for (int i = 0; i < 3; i++) {
			initSsbo();
			for (int j = 0; j < 30; j++) {
				glDispatchCompute(n_data_items, 1, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
			}
		// }

		current_i++;
		genProgress = (current_i + 1) / (float)n_iterations;
		if (current_i >= n_iterations) {
			printf("Generation finished\n");
			stopGeneration();
		}
		// glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0); // 0 to chyba to samo co w binding w glsl.

		// gradient test
		// float* tdata = new float
		// float grad[3];
		// for (int y = 0; y < dynamc->height; y++) {
		// 	for (int x = 0; x < dynamc->width; x++) {
		// 		int index = y * dynamc->width + x;
		// 		if (gradient(dataPtr, x, y, grad)) {
		// 			dynamc->data[index] = grad[2];
		// 		}
		// 	}
		// }
		// delete[] tdata;
		// test end
		// int& mapwidth = dynamc->width;

		// int maxlife = 50;
		// float dt = 0.1;

		// for (int i = 0; i < iter_per_gen; i++) {
		// 	// float posx = 250.0 + (rand() % 100 - 50);
		// 	// float posy = 250.0 + (rand() % 100 - 50);
		// 	float posx = rand() % 512;
		// 	float posy = rand() % 512;
		// 	float dirx = 0.0f;
		// 	float diry = 0.0f;
		// 	float speed = startSpeed;
		// 	float water = startWater;
		// 	float sediment = 0.0f;
		// 	for (int i = 0; i < maxlife; i++) {
		// 		int oldix = floor(posx);
		// 		int oldiy = floor(posy);
		// 		float cellx = posx - oldix;
		// 		float celly = posy - oldiy;

		// 		float gr_data[3];
		// 		bool gradres = gradient(dataPtr, posx, posy, gr_data);
		// 		if (!gradres) {
		// 			// printf("break1\n");
		// 			break;
		// 		}
				
		// 		dirx = dirx * inertia - gr_data[0] * (1 - inertia);
		// 		diry = diry * inertia - gr_data[1] * (1 - inertia);

		// 		float dirlen = max(0.01f, sqrtf(dirx * dirx + diry * diry));
		// 		dirx /= dirlen;
		// 		diry /= dirlen;

		// 		posx += dirx;
		// 		posy += diry;

		// 		if (posx < 0 || posx >= w - 1 || posy < 0 || posy >= h - 1) {
		// 			// printf("breka\n");
		// 			break;
		// 		}

		// 		float oldHeight = gr_data[2];
		// 		gradres = gradient(dataPtr, posx, posy, gr_data);
		// 		if (!gradres) {
		// 			// printf("breka3\n");
		// 			break;
		// 		}
		// 		float newHeight = gr_data[2];
		// 		float dheight = newHeight - oldHeight;
		// 		// printf("oldh: %f, newh: %f\n", oldHeight, newHeight);
		// 		// printf("sed: %f\n", sediment);

		// 		float capacity = max(-dheight * speed * water * capacityParam, minCapacity);

		// 		if (sediment > capacity || dheight > 0.0f) {
		// 			float depositedAmount = (dheight > 0.0f) ? min(dheight, sediment) : (sediment - capacity) * depositionRate;
		// 			sediment -= depositedAmount;

		// 			int index = oldiy * mapwidth + oldix;
		// 			dataPtr[index] += depositedAmount * (1 - cellx) * (1 - celly);
		// 			dataPtr[index + 1] += depositedAmount * cellx * (1 - celly);
		// 			dataPtr[index + mapwidth] += depositedAmount * (1 - cellx) * celly;
		// 			dataPtr[index + mapwidth + 1] += depositedAmount * cellx * celly;
		// 		} else {
		// 			// Erode terrain
		// 			float erosionAmount = min((capacity - sediment) * erosionRate, -dheight);
		// 			sediment += erosionAmount;
		// 			// printf("Erosion amount: %f\n", erosionAmount);

		// 			int index = oldiy * mapwidth + oldix;
		// 			dataPtr[index] -= erosionAmount * (1 - cellx) * (1 - celly);
		// 			dataPtr[index + 1] -= erosionAmount * cellx * (1 - celly);
		// 			dataPtr[index + mapwidth] -= erosionAmount * (1 - cellx) * celly;
		// 			dataPtr[index + mapwidth + 1] -= erosionAmount * cellx * celly;
		// 		}

		// 		speed = sqrt(max(0.0f, speed * speed + dheight * gravity));
		// 		water *= (1 - evaporationRate);
		// 	}

		// }
	}

	void stopGeneration() {
		genInprogress = false;
		current_i = 0;

		copyHeightFromResult();
	}

	void gen() {
		// iteration(); // JUST TESTING DELETE LATER
		if (input1 != nullptr) {
			if (!genInprogress) {
				preIterationSetup();
				genInprogress = true;
				// iteration();
			} else {
				iteration();
			}
			
			dynamc->updateGL();
		}
	}

	const char* getName() {
		return "DropletErosion";
	}

	bool setInput(int index, DynamcTexture* dynamc) {
		if (index == 0 && input1 == nullptr) {
			printf("[erosion] input1 set!\n");
			input1 = dynamc;
			return true;
		}
		
		return false;
	}

	bool unsetInput(int index) {
		if (index == 0 && input1 != nullptr) {
			input1 = nullptr;
			return true;
		}

		return false;
	}


	json serialize() {
		json result;
		// result["type"] = type;

		return result;
	}

	void unpackParams(const json& json_data) {
		// type = json_data.at("type");
	}
};

#endif