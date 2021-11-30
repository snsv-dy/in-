#ifndef _NOISE_GENERATOR_HPP_
#define _NOISE_GENERATOR_HPP_

#include <cmath>
#include <json.hpp>
#include <imgui.h>
#include <FastNoiseLite.h>

#include "DynamcTexture.hpp"
#include "Generator.hpp"

using json = nlohmann::json;


class NoiseGenerator : public Generator {
	const char* noise_type_str[6] = {
		"NoiseType_OpenSimplex2",
        "NoiseType_OpenSimplex2S",
        "NoiseType_Cellular",
        "NoiseType_Perlin",
        "NoiseType_ValueCubic",
        "NoiseType_Value"
	};

	FastNoiseLite::NoiseType type = FastNoiseLite::NoiseType_OpenSimplex2;
	int seed = 123456789;
	float frequency = 0.01;
	int octaves = 1;
	float xtranslation = 0.0f;
	float ytranslation = 0.0f;
	ImVec2 translation = ImVec2(0.0f, 0.0f);

	// DynamcTexture *dynamc = nullptr;
	FastNoiseLite noise;
public:
	NoiseGenerator() {
		noise.SetFractalType(FastNoiseLite::FractalType_FBm);
		updateNoiseParams();
	}

	bool drawGui() {
		bool changed = false;
		if (ImGui::BeginCombo("Type", noise_type_str[type])) {
			for (int i = 0; i < 6; i++) {
				if (ImGui::Selectable(noise_type_str[i], i == type)) {
					type = (FastNoiseLite::NoiseType)i;
					changed = true;
				}
			}
			ImGui::EndCombo();
		}
		changed |= ImGui::DragInt("Seed", &seed, 1);
		changed |= ImGui::DragInt("Fractal octaves", &octaves, 0.1f, 1, 20);
		changed |= ImGui::DragFloat("Frequency", &frequency, 0.0001f);
		changed |= ImGui::DragFloat2("translation", &translation.x, 0.1f);
		// ImGui::SameLine();
		// changed |= ImGui::DragFloat("ytranslation", &ytranslation, 0.1f);

		if (changed) {
			updateNoiseParams();
			gen();
		}

		return changed;
	}

	void updateNoiseParams() {
		noise.SetNoiseType(type);
		noise.SetSeed(seed);
		noise.SetFrequency(frequency);
		noise.SetFractalOctaves(octaves);
	}

	// void setTexture(DynamcTexture* dynamc1) {
	// 	printf("Interface exec %d\n", this);
	// 	dynamc = dynamc1;
	// 	fw = (float)dynamc1->width;
	// 	fh = (float)dynamc1->height;
	// 	width = dynamc1->width;
	// 	height = dynamc1->height;
	// }

	void gen() {
		if (dynamc != nullptr) {
			for (int y = 0; y < height; y++) {
				int yw = y * width;
				float fy = (float)y + translation.y;

				for (int x = 0; x < height; x++) {
					int index =y * width + x;
					float fx = (float)x + translation.x;
					float value = noise.GetNoise(fx, fy);
					value = value * 0.5f + 0.5f;
					dynamc->data[index] = value;
				}
			}
			dynamc->updateGL();
		}
	}

	const char* getName() {
		return "OpenSimplex2";
	}

	json serialize() {
		json result;
		result["type"] = type;
		result["octaves"] = octaves;
		result["seed"] = seed;
		result["frequency"] = frequency;
		result["xtranslation"] = xtranslation;
		result["ytranslation"] = ytranslation;
		return result;
	}

	void unpackParams(const json& json_data) {
		seed = json_data["seed"];
		type = json_data["type"];
		octaves = json_data["octaves"];
		frequency = json_data["frequency"];
		xtranslation = json_data["xtranslation"];
		ytranslation = json_data["ytranslation"];

		updateNoiseParams();
	}
};

#endif