#ifndef _ANGLE_COLOR_GENERATOR_HPP_
#define _ANGLE_COLOR_GENERATOR_HPP_

#include <cmath>
#include <vector>
#include <json.hpp>

#include "imgui.h"
#include "imgui_internal.h"
#include <glm/glm.hpp>

#include "Generator.hpp"
#include "ColorGenerator.hpp"

using namespace std;
using json = nlohmann::json;

class AngleColorGenerator : public Generator {
public:
	DynamcTexture* input1 = nullptr;
	vector<float> normalMap;
	bool outside_gen = true;

	vector<colorStep> colorSteps {
		{0.0f, {0.2f, 1.0f, 0.05f, 1.0f}},
		{0.6f, {0.5f, 0.5f, 0.5f, 1.0f}},
		{0.9f, {1.0f, 1.0f, 1.0f, 1.0f}},
	};

	bool drawGui() {
		bool changed = false;
		ImGui::BeginTable("colors", 3);
		for (int i = 0; i < colorSteps.size(); i++) {
			colorStep& step = colorSteps[i];

			ImGui::TableNextRow();
			
			// ImGui::TableSetColumnIndex(0);
			ImGui::PushID(i);
			ImGui::BeginGroup();

			// if (ImGui::IsItemActive() && !ImGui::IsItemHovered()) {
			// 	int i_next = i + (ImGui::GetMouseDragDelta(0).y < 0.0f ? -1 : 1);
			// 	if (i_next >= 0 && i_next < colorSteps.size() - 1) {
			// 		colorSteps[i] = colorSteps[i_next];
			// 		colorSteps[i_next] = step;
			// 		ImGui::ResetMouseDragDelta();
			// 	}
			// }
			// ImGui::TableSetColumnIndex(0);
			ImGui::TableNextColumn();
			if (ImGui::SliderFloat("Elevation", &step.threshold, 0.0f, 1.0f)) {
				// colorSteps[i].first = new_value;
				changed = true;
			}

			// float tcolor[3] = {stop.color.x, color.y, color.z};
			// ImGui::TableSetColumnIndex(1);
			ImGui::TableNextColumn();
			changed |= ImGui::ColorEdit3("Color", (float*)&step.color);

			// ImGui::TableSetColumnIndex(2);
			ImGui::TableNextColumn();
			if (colorSteps.size() > 1 && ImGui::Button("x")) {
				colorSteps.erase(colorSteps.begin() + i);
				i--;
				changed = true;
			}

			ImGui::EndGroup();
			ImGui::PopID();

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
				ImGui::SetDragDropPayload("COLOR_STOP", &i, sizeof(int));
				ImGui::EndDragDropSource();
				changed = true;
			}

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("COLOR_STOP")) {
					if (payload->DataSize != sizeof(int)) {
						printf("not int dropped\n");
					} else {
						int payload_index = *(int *)payload->Data;
						colorStep t = colorSteps[i];
						colorSteps[i] = colorSteps[payload_index];
						colorSteps[payload_index] = t;
						changed = true;
					}
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::EndTable();

		if (ImGui::Button("Add color")) {
			colorSteps.push_back({1.0f, {0.0f, 0.0f, 0.0f, 1.0f}});
			changed = true;
		}

		if (ImGui::Button("generate") || changed) {
			outside_gen = false;
			gen();
		}

		return changed;
	}

	void calculateNormal() {
		printf("terrain0: %.15f", input1->data[0]);
		const float dist = 1.0f / 512.0f;//1.0f / input1->width;
		float* datap = input1->data;
		for (int y = 1; y < input1->height - 1; y++) {
			// bool yboundary = y == 0 || y == input1->height - 1;
			int yi = y * input1->width;
			for (int x = 1; x < input1->width - 1; x++) {
				// bool xboundary = x == 0 || x == input1->height - 1;
				float current = input1->data[yi + x];

				// float top = (y > 0) ? input1->data[(y - 1) * input1->width + x] : current;
				// float bottom = current; //(y < (input1->height - 1)) ? input1->data[(y + 1) * input1->width + x] : current;

				// float left = current; //(x > 0) ? input1->data[yi + x - 1] : current;
				// float right = current; //(x < input1->width - 1) ? input1->data[yi + x + 1] : current;
				float top = datap[(y - 1) * 512 + x];
				float bottom = datap[(y + 1) * 512 + x];
				float left = datap[y * 512 + x - 1];
				float right = datap[y * 512 + x + 1];
				
				glm::vec3 v = glm::vec3(0.0f, top - bottom, dist);
				glm::vec3 u = glm::vec3(dist, right - left, 0.0f);
				glm::vec3 n = glm::cross(v, u);
				n /= sqrtf(n.x * n.x + n.y * n.y + n.z * n.z);
				// glm::vec3 n = glm::normalize(glm::vec3(dist * (right - left), dist * (bottom - top), 1.0f));
				float angle = glm::dot(n, glm::vec3(0.0f, 1.0f, 0.0f));
				normalMap[yi + x] = acos(n[1]) / 3.141592 * 2.0f;
				// normalMap[yi + x] = 1.0f - n.y;//glm::length(n);
				// printf("%2.2f ", angle);
				// printf("v: %2.2f %2.2f %2.2f %2.2f %2.2f\n", v.x, v.y, v.z, top, bottom);
				// printf("u: %2.2f %2.2f %2.2f\n", u.x, u.y, u.z);
				// printf("n: %2.2f %2.2f %2.2f\n", n.x, n.y, n.z);
			}
			// printf("\n");
		}
	}

	void gen() {
		if (dynamc != nullptr && !dynamc->monochrome && input1 != nullptr) {
			const auto [width, height] = dynamc->getSize();
			float fwidth = (float)width;
			float fheight = (float)height;

			if (outside_gen) {
				calculateNormal();
			}

			// Temp
			for (int y = 0; y < height; y++) {
				int yw = y * width;

				for (int x = 0; x < width; x++) {
					int index = (yw + x);
					float norm = normalMap[index];

					dynamc->data[index * 3] = norm;
					dynamc->data[index * 3 + 1] = norm;
					dynamc->data[index * 3 + 2] = norm;
				}
			}

			printf("Gened\n");

			dynamc->updateGL();
			return;
			
			for (int y = 0; y < height; y++) {
				int yw = y * width;

				for (int x = 0; x < width; x++) {
					int index = (yw + x);
					float input_value = normalMap[index];

					// for (auto [threshold, color] : colorSteps) {
					for (int i = 0; i < colorSteps.size(); i++) {
						auto [threshold, color] = colorSteps[i];
						if (input_value >= threshold) {
							ImVec4 result = color;
							// if (i < colorSteps.size() - 1) {
							// 	auto [threshold1, next_color] = colorSteps[i + 1];
							// 	float distance = threshold1 - threshold;
        					// 	float delta = (input_value - threshold) / distance;
							// 	result = ImLerp(result, next_color, delta);
							// }	

							dynamc->data[index * 3] = result.x;
							dynamc->data[index * 3 + 1] = result.y;
							dynamc->data[index * 3 + 2] = result.z;
						}
					}
				}
			}

			outside_gen = true;
			dynamc->updateGL();
		}
	}

	bool setInput(int index, DynamcTexture* dynamc) {
		if (index == 0 && input1 == nullptr) {
			printf("input1 set!\n");
			input1 = dynamc;
			if (normalMap.size() != input1->width * input1->height) {
				normalMap.resize(input1->width * input1->height);
				calculateNormal();
			}

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

	bool hasInput() {
		return true;
	}

	const char* getName() {
		return "AngleColor";
	}

	json serialize() {
		json result;
		json steps = json::array();
		for (const colorStep& step : colorSteps) {
			steps.push_back({
				{"threshold", step.threshold},
				{"color", {step.color.x, step.color.y, step.color.z, step.color.w}}
			});
		}

		result["steps"] = steps;

		return result;
	}

	void unpackParams(const json& json_data) {
		colorSteps.clear();
		for (const json& step : json_data["steps"]) {
			colorSteps.push_back({
				step["threshold"],
				{step["color"][0], step["color"][1], step["color"][2], step["color"][3]}
			});
		}
	}

	void bindTextures() {
		glActiveTexture(GL_TEXTURE0);
		int heightTexture = 0;
		if (input1 != nullptr) {
			heightTexture = input1->texture;
		} 

		glBindTexture(GL_TEXTURE_2D, heightTexture);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, dynamc->texture);
	}
};

#endif