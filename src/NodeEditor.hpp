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

class NodeEditor {
	map<int, Link> links; // change to map?
	int current_id = 0;
public:
	NodeEditor();
	void draw();
	int getLinksSize();
	~NodeEditor();
};

#endif