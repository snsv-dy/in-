#ifndef _DYNAMC_TEXTURE_HPP_
#define _DYNAMC_TEXTURE_HPP_

class DynamcTexture {
	int width, height;
public:
	float* data;
	unsigned int texture;
	float divider = 10.0f;

	// void (*generator)(float*, int, int);
	// Grayscale
	DynamcTexture(const int width, const int height): width{width}, height{height}, data{nullptr} {
		data = new float[width * height];

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	void updateGL() {
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 	width, height, 0, GL_RED, GL_FLOAT, data);
		// glGenerateMipmap
	}

	void gen() {
		for (int y = 0; y < height; y++) {
			int yw = y * width;
			for (int x = 0; x < height; x++) {
				float fx = (float)x / divider;
				float fy = (float)y / divider;
				data[yw + x] = (sinf(fx) + cosf(fy) + 2.0f) / 4.0f;
			}
		}

		updateGL();
	}

	~DynamcTexture() {
		glDeleteTextures(1, &texture);
		delete[] data;
	}
};

#endif