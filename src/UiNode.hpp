#ifndef _UINODE_HPP_
#define _UINODE_HPP_

// #include <cstdio>
#include <vector>
#include <map>
#include <memory>
// #include <algorithm>

// #define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>

#include <imgui.h>
#include <imnodes/imnodes.h>

#include "DynamcTexture.hpp"
#include "Generator.hpp"

using namespace std;

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

struct UiNode {
    int id;
	
	vector<int> inputs;
	vector<int> outputs;
	vector<Link> links;
	int output;

    float value = 0.0f;

	const int texWidth = 512;
	DynamcTexture dynamc;
	unique_ptr<Generator> generator;

	ImU32 color = NODE_COLOR_DEFAULT;
	ImU32 colorSelected = NODE_COLOR_DEFAULT_SELECTED;

	UiNode(unique_ptr<Generator> generator);

	void setColors(ImU32 color, ImU32 colorSelected);
	void drawGui();
	void draw();
	bool hasInput(int id);
	bool hasOutput(int id);
	bool giveInput(int id, DynamcTexture* dynamc);
	bool unsetInput(int id);
	bool addLink(Link link);
	bool removeLink(int id);

	~UiNode();
};

#endif