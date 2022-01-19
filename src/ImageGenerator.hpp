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
#include <stb_image_resize.h>

#include "DynamcTexture.hpp"
#include "Generator.hpp"

using json = nlohmann::json;

class ImageGenerator : public Generator {
	bool dirty = false;
public:
	ImageGenerator() {
	}
	std::string src = "";
	bool resize = false;

	bool drawGui() {
		bool changed = false;

		// ImGui::BeginDisabled();
		ImGui::InputText("File path", &src);
		// ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::MenuItem("Browse")) {
			ImGuiFileDialog::Instance()->OpenDialog("ImageGeneratorPath", "Choose File", ".*,.png", ".");
		}
		
		if (ImGuiFileDialog::Instance()->Display("ImageGeneratorPath")) {
			printf("wth man?\n");
			if (ImGuiFileDialog::Instance()->IsOk()) {
				// printf("jes ok\n");
				auto [filename, path] = *ImGuiFileDialog::Instance()->GetSelection().begin();
				src = path;
				changed = true;
				// printf("opening file: %s, %s\n", filename.c_str(), path.c_str());
				// src = ImGuiFileDialog::Instance()->GetFilePathName();
			}

			ImGuiFileDialog::Instance()->Close();
		}

		changed |= ImGui::Checkbox("Resize", &resize);
		changed |= ImGui::Button("Reload");

		if (changed) {
			gen();
		}

		return changed;
	}

	void gen() {
		if (dynamc != nullptr) {
			//
			if (dirty) {
				dynamc->clear();
			}

			int w, h, channels;
			stbi_set_flip_vertically_on_load(true);
			unsigned char *tex_ptr = stbi_load(src.c_str(), &w, &h, &channels, 1);
			if (tex_ptr != NULL) {
				bool resized = false;
				if (resize && (w != dynamc->width || h != dynamc->height)) {
					unsigned char* temp_buffer = new unsigned char[dynamc->width * dynamc->height];
					if (temp_buffer != nullptr) {
						stbir_resize_uint8(tex_ptr, w, h, 0, temp_buffer, dynamc->width, dynamc->height, 0, 1);

						stbi_image_free(tex_ptr);
						tex_ptr = temp_buffer;
						w = dynamc->width;
						h = dynamc->height;
						resized = true;
					}
				}
				printf("Texture load channels: %d, [%d x %d] \n", channels, w, h);

				int minw = min(dynamc->width, w);
				int minh = min(dynamc->height, h);

				// int channels_mul = channels == 1 ? 1 : 3;

				// copy texture
				for (int y = 0; y < minh; y++) {
					int yi = y * dynamc->width;
					int texy = y * w;
					for (int x = 0; x < minw; x++) {
						// int index = yi + x;
						dynamc->data[yi + x] = tex_ptr[texy + x] / 255.0f;
					}
				}

				// if (dirty && (w < dynamc->width || h < dynamc->height)) {
				// 	int xstart = w;
				// 	int ystart = h;

				// 	for (int y = ystart; y < dynamc->height; y++) {
				// 		int yi = y * dynamc->width;
				// 		for (int x = xstart; x < dynamc->width; x++) {
				// 			dynamc->data[yi + x] = 0.0f;
				// 		}
				// 	}
				// }

				if (resized) {
					delete[] tex_ptr;
				} else {
					stbi_image_free(tex_ptr);
				}

				dynamc->updateGL();
				dirty = true;
			} else {
				printf("Texture loading failed: %s\n", stbi_failure_reason());
			}
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
		src = json_data.at("src");
	}
};

#endif