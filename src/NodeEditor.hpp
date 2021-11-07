#ifndef NODEEDITOR_HPP
#define NODEEDITOR_HPP

#include <cstdio>
#include <vector>
#include <map>
#include <memory>
#include "imgui.h"
#include <imnodes/imnodes.h>

#include "DynamcTexture.hpp"
#include "Generator.hpp"
#include "SinGen.hpp"
#include "GradGen.hpp"

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

	const int texWidth = 512;
	DynamcTexture dynamc;
	unique_ptr<Generator> generator;

	UiNode(unique_ptr<Generator> generator): dynamc{texWidth, texWidth}, generator{move(generator)} {
	// generator{make_unique<SinGenerator>(&dynamc, texWidth, texWidth)} {
		this->generator->setTexture(&dynamc);

		this->generator->gen();
	}
};

class NodeEditor {
	map<int, Link> links; // change to map?
	vector<shared_ptr<UiNode>> nodes;
	int current_id = 0;
public:
	shared_ptr<UiNode> selectedNode = nullptr;
	NodeEditor();
	void draw();
	int getLinksSize();
	~NodeEditor();
};

#endif