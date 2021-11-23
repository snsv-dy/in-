#ifndef NODEEDITOR_HPP
#define NODEEDITOR_HPP

#include <iostream>
#include <cstdio>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <fstream>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imnodes/imnodes.h>
#include <json.hpp>

#include "DynamcTexture.hpp"
#include "Generator.hpp"
#include "SinGen.hpp"
#include "GradGen.hpp"
#include "CombinerGenerator.hpp"
#include "ColorGenerator.hpp"
#include "UiNode.hpp"

using namespace std;
using json = nlohmann::json;

class NodeEditor {
	map<int, Link> links; // change to list?
	map<int, shared_ptr<UiNode>> nodes;
	int current_id = 0;
	int links_id = 0;
public:
	shared_ptr<UiNode> selectedNode = nullptr;
	NodeEditor();
	void save(const char* filename = "untitled1.json");
	void load(const char* filename = "untitled1.json");
	void addNode(const int& type);
	void draw();
	int getLinksSize();
	void DeleteNode(int nodeId);
	~NodeEditor();
};

#endif