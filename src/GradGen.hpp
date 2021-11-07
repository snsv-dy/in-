#ifndef _GRAD_GENERATOR_HPP_
#define _GRAD_GENERATOR_HPP_

#include <math.h>

#include "imgui.h"
#include "DynamcTexture.hpp"
#include "Generator.hpp"

class GradGenerator : public Generator {
	float amp = 1.0f;
	float radius = 100.0f;
	float xtranslation = 0.0f;
	float ytranslation = 0.0f;

	// int width = 512;
	// int height = width;

	// DynamcTexture *dynamc;
public:
	GradGenerator() {

	}

	void drawGui() {
		bool changed = false;
		changed |= ImGui::DragFloat("Height", &amp, 0.01f);
		changed |= ImGui::DragFloat("Radius", &radius, 1.0f);
		changed |= ImGui::DragFloat("xranslation", &xtranslation, 0.1f);
		changed |= ImGui::DragFloat("ytranslation", &ytranslation, 0.1f);

		if (changed) {
			gen();
		}
	}
	
	float distance(float x1, float y1, float x2, float y2) {
		float dx = (x2 - x1);
		float dy = (y2 - y1);
		return sqrt(dx * dx + dy * dy);
	}

	void gen() {
		if (dynamc != nullptr) {
			float maxDist = radius;
			for (int y = 0; y < height; y++) {
				int yw = y * width;
				// float value = sinf(fy / period + xtranslation) * amplitude + ytranslation;

				for (int x = 0; x < height; x++) {
					// float fy = (float)y - fh / 2.0f;
					// float fx = (float)x - fw / 2.0f;
					float fy = (float)y - fh / 2.0f;
					float fx = (float)x - fw / 2.0f;
					float DistFromCenter = fmin(distance(xtranslation, ytranslation, fx, fy) / maxDist, amp);
					dynamc->data[yw + x] =  amp - DistFromCenter;
				}
			}
			dynamc->updateGL();
		}
	}

	const char* getName() {
		return "Grad";
	}
};

#endif