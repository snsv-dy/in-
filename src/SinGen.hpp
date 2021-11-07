#ifndef _SIN_GENERATOR_HPP_
#define _SIN_GENERATOR_HPP_

#include <cmath>
#include "imgui.h"
#include "DynamcTexture.hpp"
#include "Generator.hpp"

class SinGenerator : public Generator {
	float amplitude = 0.5f;
	float period = 100.0f;
	float xtranslation = 0.0f;
	float ytranslation = 0.5f;
	int axis = 0;

	// DynamcTexture *dynamc = nullptr;
public:
	SinGenerator() {
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
	}

	// unsigned int& getTexture() {
	// 	return dynamc->texture;
	// }
};

#endif