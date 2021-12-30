#ifndef _IMAGE_GENERATOR_HPP_
#define _IMAGE_GENERATOR_HPP_

#include <cmath>
#include <string>

#include <json.hpp>
#include <imgui.h>
#include <ImGuiFileDialog.h>
#include <misc/cpp/imgui_stdlib.h>
#include <FastNoiseLite.h>
#include <stb_image.h>

#include "DynamcTexture.hpp"
#include "Generator.hpp"

using json = nlohmann::json;

class ImageGenerator : public Generator {
	
public:
	ImageGenerator() {
	}
	std::string src = "/home/jacek/pyts/SZKOŁA/INŻ/program/height.png";
	bool drawGui() {
		bool changed = false;

		ImGui::BeginDisabled();
		ImGui::InputText("File path", &src);
		ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::MenuItem("Browse")) {
			ImGuiFileDialog::Instance()->OpenDialog("ImageGeneratorPath", "Choose File", ".png,.*", ".");
		}
		
		if (ImGuiFileDialog::Instance()->Display("ImageGeneratorPath")) {
			if (ImGuiFileDialog::Instance()->IsOk() && ImGuiFileDialog::Instance()->GetFilePathName() != src) {
				src = ImGuiFileDialog::Instance()->GetFilePathName();
				changed = true;
			}

			ImGuiFileDialog::Instance()->Close();
		}

		changed |= ImGui::Button("Reload");

		if (changed) {
			gen();
		}

		return changed;
	}

	void gen() {
		if (dynamc != nullptr) {
			//

			int w, h, channels;
			unsigned char *tex_ptr = stbi_load(src.c_str(), &w, &h, &channels, 1);
			printf("Texture load channels: %d, [%d x %d] \n", channels, w, h);

			int minw = min(dynamc->width, w);
			int minh = min(dynamc->height, h);

			// copy texture
			for (int y = 0; y < minh; y++) {
				int yi = y * minw;
				for (int x = 0; x < minw; x++) {
					int index = yi + x;
					dynamc->data[index] = tex_ptr[index] / 255.0f;
				}
			}

			stbi_image_free(tex_ptr);

			dynamc->updateGL();
		}
	}

	const char* getName() {
		return "Image";
	}

	json serialize() {
		json result;
		result["src"] = src;

		return result;
	}

	void unpackParams(const json& json_data) {
		// seed = json_data.at("seed");
	}
};

#endif