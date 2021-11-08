#ifndef _COMBINER_GENERATOR_HPP_
#define _COMBINER_GENERATOR_HPP_

#include "Generator.hpp"

class CombinerGenerator : public Generator {
public:
	DynamcTexture* input1 = nullptr;
	DynamcTexture* input2 = nullptr;

	// DynamcTexture* dynamc = nullptr;

	// bool setInput(DynamcTexture* input, int which = 0) {
	// 	which == 0 ? input1 = input : input2 = input;
	// 	return 
	// }

	// bool unsetInput(int which = 0) {
	// 	which == 0 ? input1 = nullptr : input2 = nullptr;
	// }

	void drawGui() {
		ImGui::Text("Sup.\n");
		if (ImGui::Button("generate")) {
			gen();
		}
	}

	void gen() {
		if (dynamc != nullptr) {
			if ((input1 != nullptr && input2 == nullptr) || (input2 != nullptr && input1 == nullptr) ) {
				copyInput(input1 != nullptr ? input1 : input2);
			} else if (input1 != nullptr && input2 != nullptr) {
				combine();
			}
			dynamc->updateGL();
		}
	}

	bool setInput(int index, DynamcTexture* dynamc) {
		if (index == 0 && input1 == nullptr) {
			printf("input1 set!\n");
			input1 = dynamc;
			return true;
		} else if (index == 1 && input2 == nullptr) {
			printf("input2 set!\n");
			input2 = dynamc;
			return true;
		}
		
		return false;
	}

	bool unsetInput(int index) {
		if (index == 0 && input1 != nullptr) {
			input1 = nullptr;
			return true;
		} else if (index == 1 && input2 != nullptr) {
			input2 = nullptr;
			return true;
		}

		return false;
	}

	bool hasInput() {
		return true;
	}

	const char* getName() {
		return "Combine";
	}

private:
	// Generation helpers
	void copyInput(DynamcTexture* input) {
		for (int y = 0; y < height; y++) {
			int yw = y * width;
			for (int x = 0; x < height; x++) {
				int index = yw + x;
				
				dynamc->data[index] = input->data[index];
			}
		}
	}

	void combine() {
		for (int y = 0; y < height; y++) {
			int yw = y * width;
			for (int x = 0; x < height; x++) {
				int index = yw + x;
				float out = input1->data[index] + input2->data[index];

				dynamc->data[index] = out;
			}
		}
	}
};

#endif