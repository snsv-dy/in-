#ifndef _COMBINER_GENERATOR_HPP_
#define _COMBINER_GENERATOR_HPP_

#include <json.hpp>
#include "Generator.hpp"

using json = nlohmann::json;

enum class CombineFunctions {
	ADD,
	SUBTRACT,
	MULTIPLY,
	DIVIDE
};

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

	CombineFunctions func = CombineFunctions::ADD;

	bool drawGui() {
		bool changed = false;
		ImGui::Text("Sup.\n");

		if (ImGui::RadioButton("Add", func == CombineFunctions::ADD)) {
			func = CombineFunctions::ADD;
			changed = true;
		} 
		
		if (ImGui::RadioButton("Subtract", func == CombineFunctions::SUBTRACT)) {
			func = CombineFunctions::SUBTRACT;
			changed = true;
		}

		if (ImGui::RadioButton("Multiply", func == CombineFunctions::MULTIPLY)) {
			func = CombineFunctions::MULTIPLY;
			changed = true;
		}

		if (ImGui::RadioButton("Divide", func == CombineFunctions::DIVIDE)) {
			func = CombineFunctions::DIVIDE;
			changed = true;
		}

		if (ImGui::Button("generate")) {
			gen();
			changed = true;
		}

		return changed;
	}

	void gen() {
		if (dynamc != nullptr) {
			if ((input1 != nullptr && input2 == nullptr) || (input2 != nullptr && input1 == nullptr) ) {
				copyInput(input1 != nullptr ? input1 : input2);
			} else if (input1 != nullptr && input2 != nullptr) {
				combine();
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
				float &val1 = input1->data[index];
				float &val2 = input2->data[index];
				float out;
				switch (func) {
					case CombineFunctions::SUBTRACT:
						out = val1 - val2;
						break;
					case CombineFunctions::MULTIPLY:
						out = val1 * val2;
						break;
					case CombineFunctions::DIVIDE:
						out = val1 / val2;
						break;
					default:
						out = val1 + val2;
				}

				out = fmin(out, 1.0f);

				dynamc->data[index] = out;
			}
		}
	}

	json serialize() {
		json result;
		result["func"] = func;
	
		return result;
	}

	void unpackParams(const json& json_data) {
		func = json_data["func"];
	}
};

#endif