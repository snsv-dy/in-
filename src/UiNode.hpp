#ifndef _UINODE_HPP_
#define _UINODE_HPP_

// #include <cstdio>
#include <sstream>
#include <vector>
#include <map>
#include <memory>
// #include <algorithm>

// #define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>

#include <imgui.h>
#include <imnodes/imnodes.h>
#include <json.hpp>

#include "DynamcTexture.hpp"

#include "Generator.hpp"
#include "SinGen.hpp"
#include "GradGen.hpp"
#include "CombinerGenerator.hpp"
#include "ColorGenerator.hpp"

using namespace std;
using json = nlohmann::json;

struct Link {
	int id;
	int beg;
	int end;
	int begNode = -1;
	int endNode = -1;
	// Link(int id, int beg, int end)
};

// Node titlebar colors
const ImU32 NODE_COLOR_DEFAULT = IM_COL32(11, 109, 191, 255);
const ImU32 NODE_COLOR_DEFAULT_SELECTED = IM_COL32(81, 148, 204, 255);
const ImU32 NODE_COLOR_GREEN = IM_COL32(11, 191, 109, 255);
const ImU32 NODE_COLOR_GREEN_SELECTED = IM_COL32(81, 204, 148, 255);
const ImU32 NODE_COLOR_YELLOW = IM_COL32(191, 180, 11, 255);
const ImU32 NODE_COLOR_YELLOW_SELECTED = IM_COL32(204, 197, 81, 255);

struct UiNode {
    int id;
	int type_i = 0;
	
	vector<int> inputs;
	vector<int> outputs;
	vector<Link> links;
	vector<int> dependant_nodes;
	int output;

    float value = 0.0f;

	const int texWidth = 512;
	DynamcTexture dynamc;
	unique_ptr<Generator> generator;
	bool preview = false;

	ImU32 color = NODE_COLOR_DEFAULT;
	ImU32 colorSelected = NODE_COLOR_DEFAULT_SELECTED;

	UiNode(unique_ptr<Generator> generator, bool monochromeTexture);
	UiNode(const json& json_data);

	void setColors(ImU32 color, ImU32 colorSelected);
	bool drawGui();
	bool draw();
	bool hasInput(int id);
	bool hasOutput(int id);
	bool giveInput(int id, DynamcTexture* dynamc);
	bool unsetInput(int id);
	bool addLink(Link link);
	bool removeLink(int id);
	json serialize();

	~UiNode();
};

#endif