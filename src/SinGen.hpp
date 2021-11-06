#ifndef _SIN_GEN_HPP_
#define _SIN_GEN_HPP_

#include "imgui.h"
#include "DynamcTexture.hpp"

class SinGenerator {
	float amplitude = 1.0f;
	float period = 1.0f;
	float xtranslation = 0.0f;
	float ytranslation = 0.0f;
	int axis = 0;

	int width = 512;
	int height = width;

	DynamcTexture *dynamc;
public:
	SinGenerator(DynamcTexture *dynamc, int width, int height): dynamc{dynamc}, width{width}, height{height} {
	}

	void drawGui() {
		bool changed = false;
		changed |= ImGui::DragFloat("Amplitude", &amplitude, 0.1f);
		changed |= ImGui::DragFloat("Period", &period, 0.1f);
		changed |= ImGui::DragFloat("xranslation", &xtranslation, 0.1f);
		changed |= ImGui::DragFloat("ytranslation", &ytranslation, 0.1f);
		changed |= ImGui::SliderInt("Axis", &axis, 0, 1);

		if (changed) {
			gen();
		}
	}

	void gen() {
		for (int y = 0; y < height; y++) {
			int yw = y * width;
			float fy = (float)y;
			float value = sinf(fy / period + xtranslation) * amplitude + ytranslation;

			for (int x = 0; x < height; x++) {
				// float fx = (float)x / divider;
				int index = axis == 0 ? (y * width) + x : (x * width) + y;
				dynamc->data[index] =  value;
			}
		}
		dynamc->updateGL();
	}

	unsigned int& getTexture() {
		return dynamc->texture;
	}
};

#endif