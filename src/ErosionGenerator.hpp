#ifndef _EROSION_GENERATOR_HPP_
#define _EROSION_GENERATOR_HPP_

#include <fstream>
#include <glad/glad.h>

#include <json.hpp>
#include <imgui.h>

#include "DynamcTexture.hpp"
#include "Generator.hpp"

using json = nlohmann::json;

class ErosionGenerator : public Generator {
public:
	DynamcTexture* input1 = nullptr;
	int n_iterations = 10;

	GLuint compute_texture = 0;
	GLuint water_texture = 0;
	GLuint flow_texture = 0;

	GLuint ray_program = 0;
	int tex_w = 512, tex_h = 512;

	GLuint height_input_location = 0;
	GLuint img_output_location = 0;
	GLuint water_location = 0;
	GLuint flow_location = 0;
	GLuint stage_location = 0;

	ErosionGenerator() {
		// glGenTextures(1, &compute_texture);
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, compute_texture);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
		// NULL);
		// glBindImageTexture(0, compute_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
		compute_texture = genTexture(GL_TEXTURE0);
		water_texture = genTexture(GL_TEXTURE2, GL_READ_WRITE);
		flow_texture = genTexture(GL_TEXTURE3, GL_READ_WRITE);

		int work_grp_count[3];
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_count[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_count[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_count[2]);

		printf("max global (total) work group counts x:%i y:%i z:%i\n",
  work_grp_count[0], work_grp_count[1], work_grp_count[2]);

		int work_grp_size[3];

		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

		printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
		work_grp_size[0], work_grp_size[1], work_grp_size[2]);

		int work_grp_inv;
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
printf("max local work group invocations %i\n", work_grp_inv);

		createShader("shaders/erosion.glsl");
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

			ray_program = glCreateProgram();
			glAttachShader(ray_program, ray_shader);
			glLinkProgram(ray_program);
			// check for linking errors and validate program as per normal here
			// int success;
			glGetProgramiv(ray_program, GL_LINK_STATUS, &success);
			if(!success) {
				char error_msg[512];
				glGetProgramInfoLog(ray_program, 512, NULL, error_msg);
				printf("prorgam log:\n%s\n", error_msg);
				ray_program = 0;
				printf("succ = %d\n", success);
			}

			height_input_location = glGetUniformLocation(ray_program, "heightmap_input");
			img_output_location = glGetUniformLocation(ray_program, "img_output");
			water_location = glGetUniformLocation(ray_program, "water");
			flow_location = glGetUniformLocation(ray_program, "flow");
			stage_location = glGetUniformLocation(ray_program, "stage");

			return 0;
		}

		return -2;
	}

	const char* getName() {
		return "Erosion";
	}

	bool drawGui() {
		bool changed = false;

		ImGui::DragInt("iterations", &n_iterations);

		if (ImGui::Button("generate")) {
			gen();
			changed = true;
		}

		return changed;
	}

	void initWater() {
		float* tdata = new float[512 * 512 * 2];

		for (int y = 0; y < 512; y++) {
			for (int x = 0; x < 512; x++) {
				int index = (y * 512 + x) * 2;
				tdata[index] = input1->data[y * 512 + x];
				tdata[index + 1] = 0.0; // initial water for debug
			}
		}

		// Load initial height
		glBindTexture(GL_TEXTURE_2D, compute_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RG, GL_FLOAT, tdata);
		glBindTexture(GL_TEXTURE_2D, 0);

		delete[] tdata;
	}

	void clearTexture(unsigned int texture) {
		float* tdata = new float[512 * 512];

		for (int y = 0; y < 512; y++) {
			for (int x = 0; x < 512; x++) {
				tdata[y * 512 + x] = 0.0;
			}
		}

		// Load initial height
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RED, GL_FLOAT, tdata);
		glBindTexture(GL_TEXTURE_2D, 0);

		delete[] tdata;
	}

	void clearWater(unsigned int texture) {
		float* tdata = new float[512 * 512 * 4];

		for (int y = 0; y < 512; y++) {
			for (int x = 0; x < 512; x++) {
				int index = (y * 512 + x) * 4;
				tdata[index] = 0.0;
				tdata[index + 1] = 0.0;
				tdata[index + 2] = 0.0;
				tdata[index + 3] = 0.0;
			}
		}

		// Load initial height
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, tdata);
		glBindTexture(GL_TEXTURE_2D, 0);

		delete[] tdata;
	}

	void gen() {
		if (input1 != nullptr) {

			initWater();

			// Load initial height	
			// glBindTexture(GL_TEXTURE_2D, compute_texture);
			// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RED, GL_FLOAT, input1->data);
			// glBindTexture(GL_TEXTURE_2D, 0);

			// Clear flow
			clearTexture(flow_texture);
			// clearTexture(water_texture);
			clearWater(water_texture);

			glUseProgram(ray_program);

			glUniform1i(img_output_location, 0);
			glUniform1i(height_input_location, 1);
			glUniform1i(water_location, 2);
			glUniform1i(flow_location, 3);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, compute_texture);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, input1->texture);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, water_texture);

			glActiveTexture(GL_TEXTURE3);
			glBindTexture(GL_TEXTURE_2D, flow_texture);

			for (int i = 0; i < n_iterations; i++) {

				// Water increment stage
				glUniform1i(stage_location, 6);
				glDispatchCompute(tex_w, tex_h, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);
				
				// Flow stage
				glUniform1i(stage_location, 0);
				glDispatchCompute(tex_w, tex_h, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				// Water stage
				glUniform1i(stage_location, 1);
				glDispatchCompute(tex_w, tex_h, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				// Velocity stage
				glUniform1i(stage_location, 2);
				glDispatchCompute(tex_w, tex_h, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				// Erosion and deposition stage
				glUniform1i(stage_location, 3);
				glDispatchCompute(tex_w, tex_h, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				// Sediment transportation stage
				glUniform1i(stage_location, 4);
				glDispatchCompute(tex_w, tex_h, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				// Water evaporation stage
				glUniform1i(stage_location, 5);
				glDispatchCompute(tex_w, tex_h, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				// glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
				printf("Erosion once\n");
			}
		} else {
			printf("No input in erosion\n");
		}
		// if (dynamc != nullptr && input != nullptr) {
		// 	for (int y = 0; y < height; y++) {
		// 		int yw = y * width;
		// 		float fy = (float)y + translation.y;

		// 		for (int x = 0; x < height; x++) {
		// 			int index = y * width + x;
		// 			dynamc->data[index] = 0.1f;
		// 		}
		// 	}
		// 	dynamc->updateGL();
		// }
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