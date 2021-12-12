#ifndef _GENERATOR_HPP_
#define _GENERATOR_HPP_

#include <json.hpp>
#include "DynamcTexture.hpp"

using json = nlohmann::json;

class Generator {
protected:
	DynamcTexture* dynamc = nullptr;
	float fw = 0.0f;
	float fh = 0.0f;

	int width = 512;
	int height = width;
public:
	bool genInprogress = false;
	float genProgress = 0.0f;
	virtual bool drawGui() =0;
	virtual void gen() =0; // If true, it need's to be called 

	virtual void setTexture(DynamcTexture* dynamc) {
		this->dynamc = dynamc;
		fw = (float)dynamc->width;
		fh = (float)dynamc->height;
		width = dynamc->width;
		height = dynamc->height;
	}

	virtual unsigned int& getTexture() {
		return dynamc->texture;
	}

	virtual const char* getName() =0;
	virtual bool hasInput() { return false; }
	virtual bool setInput(int index, DynamcTexture* dynamc) { return false; }
	virtual bool unsetInput(int index) { return false; }
	virtual json serialize() { return json({}); };

	virtual void unpackParamsWrap(const json& json_data) {
		try {
			unpackParams(json_data);
		} catch (const nlohmann::detail::out_of_range& e) {
			printf("Node unpacking error: %s\n", e.what());
		}
	}

	// Use at() when accessing elements of json data, 
	// so that function above can catch exception if it happens.
	virtual void unpackParams(const json& json_data) =0;

	virtual ~Generator() {}
};

#endif