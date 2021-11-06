#ifndef _SIN_GEN_HPP_
#define _SIN_GEN_HPP_

#include "imgui.h"
#include "DynamcTexture.hpp"
#include "Generator.hpp"

class SinGenerator : public Generator {
	float amplitude = 0.5f;
	float period = 100.0f;
	float xtranslation = 0.0f;
	float ytranslation = 0.5f;
	int axis = 0;

	int width = 512;
	int height = width;

	float fw = 0.0f;
	float fh = 0.0f;

	DynamcTexture *dynamc;
public:
	SinGenerator(DynamcTexture *dynamc, int width, int height): dynamc{dynamc}, width{width}, height{height} {
		fw = (float)width;
		fh = (float)height;
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
			float fy = (float)y - fh / 2.0f;
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