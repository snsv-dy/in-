#ifndef _CPUEROSION_HPP_
#define _CPUEROSION_HPP_

#include <cmath>

class CPUErosion : public Generator {
private:
	int current_i = 0;
public:
	DynamcTexture* input1 = nullptr;
	int iter_per_gen = 10;
	int n_iterations = 300 / iter_per_gen;

	bool drawGui() {
		bool changed = false;

		ImGui::DragInt("iterations", &n_iterations);
		ImGui::DragInt("iterations multiplier", &iter_per_gen);

		if (ImGui::Button("generate")) {
			gen();
			changed = true;
		}

		return changed;
	}

	bool gradient(float* map, float posx, float posy, float* return_data) {
		int x = (int)posx;
		int y = (int)posy;
		int mapw = 512;
		if (x > 0 && x < 511 && y > 0 && y < 511) {
			float NW = map[y * mapw + x];
			float NE = map[y * mapw + x + 1];
			float SW = map[(y + 1) * mapw + x];
			float SE = map[(y + 1) * mapw + x + 1];

			float lx = posx - floor(posx);
			float ly = posy - floor(posy);

			float gradx = (NE - NW) * (1 - ly) + (SE - SW) * ly;
			float grady = (SW - NW) * (1 - lx) + (SE - NE) * lx;

			float height = NW * (1 - lx) * (1 - ly) + NE * lx * (1 - ly) + SW * (1 - lx) * ly + SE * lx * ly;

			return_data[0] = gradx;
			return_data[1] = grady;
			return_data[2] = height;

			return true;
		}

		return_data[0] = 0.0f;
		return_data[1] = 0.0f;
		return_data[2] = 0.0f;

		return false;
	}

	float* dataPtr = nullptr;
	float w = 0.0f;
	float h = 0.0f;
	void preIterationSetup() {
		dataPtr = dynamc->data;
		w = (float)dynamc->width;
		h = (float)dynamc->height;
		// copy texture from input
		for (int y = 0; y < dynamc->height; y++) {
			for (int x = 0; x < dynamc->width; x++) {
				int index = y * dynamc->width + x;
				dataPtr[index] = input1->data[index];
			}
		}
	}

	void iteration() {
		printf("iteration: %d\n", current_i);
		const float capacityParam = 4.0f; 
		const float minCapacity = 0.01f;
		const float depositionRate = 0.3f;
		const float erosionRate = 0.3f;
		const float evaporationRate = 0.02f;
		const float gravity = 4.0f;
		const float inertia = 0.05f;

		// gradient test
		// float* tdata = new float
		// float grad[3];
		// for (int y = 0; y < dynamc->height; y++) {
		// 	for (int x = 0; x < dynamc->width; x++) {
		// 		int index = y * dynamc->width + x;
		// 		if (gradient(dataPtr, x, y, grad)) {
		// 			dynamc->data[index] = grad[2];
		// 		}
		// 	}
		// }
		// delete[] tdata;
		// test end
		int& mapwidth = dynamc->width;

		int maxlife = 50;
		float dt = 0.1;

		for (int i = 0; i < iter_per_gen; i++) {
			// float posx = 250.0 + (rand() % 100 - 50);
			// float posy = 250.0 + (rand() % 100 - 50);
			float posx = rand() % 512;
			float posy = rand() % 512;
			float dirx = 0.0f;
			float diry = 0.0f;
			float speed = 1.0f;
			float water = 1.0f;
			float sediment = 0.0f;
			for (int i = 0; i < maxlife; i++) {
				int oldix = floor(posx);
				int oldiy = floor(posy);
				float cellx = posx - oldix;
				float celly = posy - oldiy;

				float gr_data[3];
				bool gradres = gradient(dataPtr, posx, posy, gr_data);
				if (!gradres) {
					printf("break1\n");
					break;
				}
				
				dirx = dirx * inertia - gr_data[0] * (1 - inertia);
				diry = diry * inertia - gr_data[1] * (1 - inertia);

				float dirlen = max(0.01f, sqrtf(dirx * dirx + diry * diry));
				dirx /= dirlen;
				diry /= dirlen;

				posx += dirx;
				posy += diry;

				if (posx < 0 || posx >= w - 1 || posy < 0 || posy >= h - 1) {
					printf("breka\n");
					break;
				}

				float oldHeight = gr_data[2];
				gradres = gradient(dataPtr, posx, posy, gr_data);
				if (!gradres) {
					printf("breka3\n");
					break;
				}
				float newHeight = gr_data[2];
				float dheight = newHeight - oldHeight;
				// printf("oldh: %f, newh: %f\n", oldHeight, newHeight);
				// printf("sed: %f\n", sediment);

				float capacity = max(-dheight * speed * water * capacityParam, minCapacity);

				if (sediment > capacity || dheight > 0.0f) {
					float depositedAmount = (dheight > 0.0f) ? min(dheight, sediment) : (sediment - capacity) * depositionRate;
					sediment -= depositedAmount;

					int index = oldiy * mapwidth + oldix;
					dataPtr[index] += depositedAmount * (1 - cellx) * (1 - celly);
					dataPtr[index + 1] += depositedAmount * cellx * (1 - celly);
					dataPtr[index + mapwidth] += depositedAmount * (1 - cellx) * celly;
					dataPtr[index + mapwidth + 1] += depositedAmount * cellx * celly;
				} else {
					// Erode terrain
					float erosionAmount = min((capacity - sediment) * erosionRate, -dheight);
					sediment += erosionAmount;
					// printf("Erosion amount: %f\n", erosionAmount);

					int index = oldiy * mapwidth + oldix;
					dataPtr[index] -= erosionAmount * (1 - cellx) * (1 - celly);
					dataPtr[index + 1] -= erosionAmount * cellx * (1 - celly);
					dataPtr[index + mapwidth] -= erosionAmount * (1 - cellx) * celly;
					dataPtr[index + mapwidth + 1] -= erosionAmount * cellx * celly;
				}

				speed = sqrt(max(0.0f, speed * speed + dheight * gravity));
				water *= (1 - evaporationRate);
			}

		}
		current_i++;
		genInprogress = current_i / (float)n_iterations;
		if (current_i >= n_iterations) {
			genInprogress = false;
			current_i = 0;
		}
	}

	void gen() {
		if (input1 != nullptr) {
			if (!genInprogress) {
				preIterationSetup();
				genInprogress = true;
				// iteration();
			} else {
				iteration();
			}
			
			dynamc->updateGL();
		}
	}

	const char* getName() {
		return "CPUErosion";
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