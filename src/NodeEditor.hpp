#ifndef NODEEDITOR_HPP
#define NODEEDITOR_HPP

#include <cstdio>
#include <vector>
#include <map>
#include <memory>
#include <algorithm>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imnodes/imnodes.h>


#include "DynamcTexture.hpp"
#include "Generator.hpp"
#include "SinGen.hpp"
#include "GradGen.hpp"
#include "CombinerGenerator.hpp"
#include "ColorGenerator.hpp"
#include "UiNode.hpp"

using namespace std;


class NodeEditor {
	map<int, Link> links; // change to list?
	map<int, shared_ptr<UiNode>> nodes;
	int current_id = 0;
	int links_id = 0;
public:
	shared_ptr<UiNode> selectedNode = nullptr;
	NodeEditor();
	void draw();
	int getLinksSize();
	void DeleteNode(int nodeId);
	~NodeEditor();
};

#endif