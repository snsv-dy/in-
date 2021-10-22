#ifndef NODEEDITOR_HPP
#define NODEEDITOR_HPP

#include <cstdio>
#include <vector>
#include <map>
#include "imgui.h"
#include <imnodes/imnodes.h>

using namespace std;

// struct Node {

// };

struct Link {
	int id;
	int beg, end;
};

struct UiNode {
    int id;
	
	int input;
	int output;

    float value = 0.0f;
};

class NodeEditor {
	map<int, Link> links; // change to map?
	vector<UiNode> nodes;
	int current_id = 0;
public:
	NodeEditor();
	void draw();
	int getLinksSize();
	~NodeEditor();
};

#endif