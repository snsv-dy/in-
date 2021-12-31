#ifndef _TERRACE_GENERATOR_HPP_
#define _TERRACE_GENERATOR_HPP_

#include <cmath>
#include <json.hpp>
#include "Generator.hpp"

using json = nlohmann::json;

class TerraceGenerator : public Generator {
public:
	DynamcTexture* input1 = nullptr;

	// DynamcTexture* dynamc = nullptr;

	// CombineFunctions func = CombineFunctions::ADD;

	bool drawGui() {
		bool changed = false;

		changed |= ImGui::DragInt("Number of terraces", &nTerraces, 0.1f);
		changed |= ImGui::DragFloat("Slope angle", &slopeAngle, 0.01f, 0.0f, 1.0f);

		if (ImGui::Button("generate")) {
			if (slopeAngle < 0.0f) {
				slopeAngle = 0.0f;
			} else if (slopeAngle > 1.0f) {
				slopeAngle = 1.0f;
			}

			if (nTerraces <= 0) {
				nTerraces = 1;
			}

			gen();
			changed = true;
		}

		return changed;
	}

	int nTerraces = 4;
	float slopeAngle = 0.5;

	float bparam = 1.0f / slopeAngle;
	float wparam = 1.0f / nTerraces;

	float remap(float h) {
		float d = floor(h / wparam);
		float d1 = (h - d * wparam) / wparam;
		float d2 = fmin(d1 * bparam, 1.0f);
		return (d2 + d) * wparam;
	}

	void gen() {
		if (dynamc != nullptr) {
			if (input1 != nullptr) {
				bparam = 1.0f / slopeAngle;
				wparam = 1.0f / nTerraces;

				for (int y = 0; y < height; y++) {
					int yw = y * width;
					for (int x = 0; x < height; x++) {
						int index = yw + x;

						dynamc->data[index] = remap(input1->data[index]);
					}
				}
			} else {
				dynamc->clear();
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
		return "Terraces";
	}

private:
	json serialize() {
		json result;
		result["n_terraces"] = nTerraces;
		result["slope_angle"] = slopeAngle;
	
		return result;
	}

	void unpackParams(const json& json_data) {
		nTerraces = json_data.at("n_terraces");
		slopeAngle = json_data.at("slope_angle");
	}
};

#endif