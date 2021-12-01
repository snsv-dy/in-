#ifndef _NOISE_GENERATOR_HPP_
#define _NOISE_GENERATOR_HPP_

#include <cmath>
// #include <stdexcept>

#include <json.hpp>
#include <imgui.h>
#include <FastNoiseLite.h>

#include "DynamcTexture.hpp"
#include "Generator.hpp"

using json = nlohmann::json;

class NoiseGenerator : public Generator {
	const char* noise_type_str[6] = {
		"OpenSimplex2",
        "OpenSimplex2S",
        "Cellular",
        "Perlin",
        "ValueCubic",
        "Value"
	};

	const char* fractal_type_str[6] = {
		"None",
		"FBm",
		"Ridged",
		"PingPong",
		"DomainWarpProgressive",
		"DomainWarpIndependent"
	};
	
	const char* cellular_distance_str[4] = {
	    "Euclidean",
        "EuclideanSq",
        "Manhattan",
        "Hybrid"
	};

	const char* cellular_return_type_str[7] = {
	    "CellValue",
        "Distance",
        "Distance2",
        "Distance2Add",
        "Distance2Sub",
        "Distance2Mul",
        "Distance2Div"
	};

	FastNoiseLite::NoiseType type = FastNoiseLite::NoiseType_OpenSimplex2;
	int seed = 123456789;
	float frequency = 0.01;
	ImVec2 translation = ImVec2(0.0f, 0.0f);

	FastNoiseLite::FractalType fractal_type = FastNoiseLite::FractalType_None;
	int octaves = 1;
	float lacunarity = 2.0f;
	float gain = 0.5;
	float weighted_strength = 0.0f;
	float pingpong_strength = 2.0f;

	FastNoiseLite::CellularDistanceFunction cellular_func = FastNoiseLite::CellularDistanceFunction_EuclideanSq;
	FastNoiseLite::CellularReturnType cellular_return_type = FastNoiseLite::CellularReturnType_Distance;
	float cellular_jitter = 1.0f;
	// DynamcTexture *dynamc = nullptr;
	FastNoiseLite noise;
public:
	NoiseGenerator() {
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
		changed |= ImGui::DragFloat("Frequency", &frequency, 0.0001f);
		changed |= ImGui::DragFloat2("translation", &translation.x, 0.1f);
		
		ImGui::Separator();

		if (ImGui::BeginCombo("Fractal type", fractal_type_str[fractal_type])) {
			for (int i = 0; i < 6; i++) {
				if (ImGui::Selectable(fractal_type_str[i], i == fractal_type)) {
					fractal_type = (FastNoiseLite::FractalType)i;
					changed = true;
				}
			}
			ImGui::EndCombo();
		}
		changed |= ImGui::DragInt("Fractal octaves", &octaves, 0.1f, 1, 20);

		changed |= ImGui::DragFloat("lacunarity", &lacunarity, 0.01f, 0.0f, 100.f);
		changed |= ImGui::DragFloat("gain", &gain, 0.01f, 0.0f, 100.f);
		changed |= ImGui::DragFloat("weighted_strength", &weighted_strength, 0.01f, 0.0f, 100.f);
		changed |= ImGui::DragFloat("pingpong_strength", &pingpong_strength, 0.01f, 0.0f, 100.f);
		// ImGui::SameLine();
		// changed |= ImGui::DragFloat("ytranslation", &ytranslation, 0.1f);

		ImGui::Separator();

		if (ImGui::BeginCombo("Cellular distance function", cellular_distance_str[cellular_func])) {
			for (int i = 0; i < 6; i++) {
				if (ImGui::Selectable(cellular_distance_str[i], i == cellular_func)) {
					cellular_func = (FastNoiseLite::CellularDistanceFunction)i;
					changed = true;
				}
			}
			ImGui::EndCombo();
		}

		if (ImGui::BeginCombo("Cellular return type", cellular_return_type_str[cellular_return_type])) {
			for (int i = 0; i < 6; i++) {
				if (ImGui::Selectable(cellular_return_type_str[i], i == cellular_return_type)) {
					cellular_return_type = (FastNoiseLite::CellularReturnType)i;
					changed = true;
				}
			}
			ImGui::EndCombo();
		}
		changed |= ImGui::DragFloat("Cellular jitter", &cellular_jitter, 0.01f, 0.0f, 100.f);


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
		noise.SetFractalType(fractal_type);
		noise.SetFractalLacunarity(lacunarity);
		noise.SetFractalGain(gain);
		noise.SetFractalWeightedStrength(weighted_strength);
		noise.SetFractalPingPongStrength(pingpong_strength);
		noise.SetCellularDistanceFunction(cellular_func);
		noise.SetCellularReturnType(cellular_return_type);
		noise.SetCellularJitter(cellular_jitter);
	}

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
		return "Noise";
	}

	json serialize() {
		json result;
		result["type"] = type;
		result["octaves"] = octaves;
		result["seed"] = seed;
		result["frequency"] = frequency;
		result["xtranslation"] = translation.x;
		result["ytranslation"] = translation.y;

		result["fractal_type"] = fractal_type;
		result["lacunarity"] = lacunarity;
		result["gain"] = gain;
		result["weighted_strength"] = weighted_strength;
		result["pingpong_strength"] = pingpong_strength;

		result["cellular_func"] = cellular_func;
		result["cellular_return_type"] = cellular_return_type;
		result["cellular_jitter"] = cellular_jitter;

		return result;
	}

	void unpackParams(const json& json_data) {
		seed = json_data.at("seed");
		type = json_data.at("type");
		octaves = json_data.at("octaves");
		frequency = json_data.at("frequency");
		translation.x = json_data.at("xtranslation");
		translation.y = json_data.at("ytranslation");

		fractal_type = json_data.at("fractal_type");
		lacunarity = json_data.at("lacunarity");
		gain = json_data.at("gain");
		weighted_strength = json_data.at("weighted_strength");
		pingpong_strength = json_data.at("pingpong_strength");
		
		cellular_func = json_data.at("cellular_func");
		cellular_return_type = json_data.at("cellular_return_type");
		cellular_jitter = json_data.at("cellular_jitter");

		updateNoiseParams();
	}
};

#endif