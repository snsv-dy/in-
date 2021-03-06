#ifndef _COLOR_GENERATOR_HPP_
#define _COLOR_GENERATOR_HPP_

#include <cmath>
#include <vector>
#include <json.hpp>

#include "imgui.h"
#include "imgui_internal.h"
#include <glm/glm.hpp>
#include <FastNoiseLite.h>

#include "Generator.hpp"

using namespace std;
using json = nlohmann::json;

struct colorStep {
	float threshold;
	ImVec4 color;
};

class ColorGenerator : public Generator {
public:
	DynamcTexture* input1 = nullptr;
	FastNoiseLite noise;
	float noise_frequency = 0.06f;
	float lerp = 0.25f;

	vector<colorStep> colorSteps {
		{0.0f, {0.2f, 1.0f, 0.05f, 1.0f}},
		{0.6f, {0.5f, 0.5f, 0.5f, 1.0f}},
		{0.9f, {1.0f, 1.0f, 1.0f, 1.0f}},
	};

	bool drawGui() {
		bool changed = false;
		changed |= ImGui::DragFloat("blend", &noise_frequency, 0.001f);
		changed |= ImGui::DragFloat("lerp", &lerp, 0.01f, .0f, .5f);

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
			gen();
		}

		return changed;
	}

	float smoothst(const float& x) {
		return 3.f * x * x - 2.f * x * x * x;
	}

	float blendValue(float value, float blend_amount) {
		return ImSaturate(1.f / (1.f - 2.f * blend_amount) * (value - blend_amount));
	}

	void gen() {
		if (dynamc != nullptr && !dynamc->monochrome && input1 != nullptr) {
			const auto [width, height] = dynamc->getSize();
			float fwidth = (float)width;
			float fheight = (float)height;
			
			for (int y = 0; y < height; y++) {
				int yw = y * width;

				for (int x = 0; x < width; x++) {
					int index = (yw + x);
					float input_value = input1->data[index];

					// for (auto [threshold, color] : colorSteps) {
					for (int i = 0; i < colorSteps.size(); i++) {
						auto [threshold, color] = colorSteps[i];
						if (input_value >= threshold) {
							ImVec4 result = color;
							if (i < colorSteps.size() - 1) {
								auto [threshold1, next_color] = colorSteps[i + 1];
								float distance = threshold1 - threshold;
        						float delta = (input_value - threshold) / distance;
								result = ImLerp(result, next_color, delta);
							}	

							dynamc->data[index * 3] = result.x;
							dynamc->data[index * 3 + 1] = result.y;
							dynamc->data[index * 3 + 2] = result.z;
						}
					}
				}
			}

			dynamc->updateGL();
		}
	}

	bool setInput(int index, DynamcTexture* dynamc) {
		if (index == 0 && input1 == nullptr) {
			printf("input1 set!\n");
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

	bool hasInput() {
		return true;
	}

	const char* getName() {
		return "Color";
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