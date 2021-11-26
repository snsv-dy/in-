	#ifndef _DYNAMC_TEXTURE_HPP_
#define _DYNAMC_TEXTURE_HPP_

#include <glad/glad.h>

using namespace std;

class DynamcTexture {
public:
	int width, height;
	float* data;
	unsigned int texture;
	float divider = 10.0f;
	bool monochrome = true;

	// void (*generator)(float*, int, int);
	// Grayscale
	DynamcTexture(const int width, const int height, bool monochrome = true): width{width}, height{height}, data{nullptr} {
		if (monochrome) {
			data = new float[width * height];
		} else {
			data = new float[width * height * 3];
		}

		this->monochrome = monochrome;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (monochrome) {
			GLint swizzleMask[] = {GL_RED, GL_RED, GL_RED, GL_ONE};
			glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
		}
		printf("Dynamc [%p] created\n", data);
		clear();
	}

	void updateGL() {
		glBindTexture(GL_TEXTURE_2D, texture);
		GLuint color_type = monochrome ? GL_RED : GL_RGB;
		glTexImage2D(GL_TEXTURE_2D, 0, color_type, 	width, height, 0, color_type, GL_FLOAT, data);
		// glGenerateMipmap
	}

	void gen() {
		// for (int y = 0; y < height; y++) {
		// 	int yw = y * width;
		// 	for (int x = 0; x < height; x++) {
		// 		float fx = (float)x / divider;
		// 		float fy = (float)y / divider;
		// 		data[yw + x] = (sinf(fx) + cosf(fy) + 2.0f) / 4.0f;
		// 	}
		// }

		updateGL();
	}

	void clear() {
		for (int y = 0; y < height; y++) {
			int yw = y * width;
			for (int x = 0; x < width; x++) {
				if (monochrome) {
					data[yw + x] = 0.0f;
				} else {
					data[yw + x] = 0.0f;
					data[yw + x + 1] = 0.0f;
					data[yw + x + 2] = 0.0f;
				}
			}
		}
	}

	pair<int, int> getSize() {
		return {width, height};
	}

	~DynamcTexture() {
		glDeleteTextures(1, &texture);
		delete[] data;
		printf("Dynamc [%p] deleted\n", data);
	}
};

#endif