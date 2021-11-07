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
#include "CombinerGenerator.hpp"

using namespace std;

// struct Node {

// };

struct Link {
	int id;
	int beg, end;
};

struct UiNode {
    int id;
	
	vector<int> inputs;
	vector<int> outputs;
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

	void draw() {
		const float node_width = 100.0f;

		ImNodes::BeginNode(id);

		ImNodes::BeginNodeTitleBar();
		// string 
		ImGui::Text(generator->getName());
		ImNodes::EndNodeTitleBar();

		for (const int& i : inputs) {
			ImNodes::BeginInputAttribute(i);
			ImGui::PushItemWidth(node_width);
			ImGui::Text("input %d", i);
			// ImGui::DragFloat("##hidelabel", &value, 0.01f, 0.0f, 1.0f);
			ImGui::PopItemWidth();
			ImNodes::EndInputAttribute();
		}

		// if (generator->hasInput()) {
			

		// 	ImNodes::BeginInputAttribute(input);
		// 	ImGui::Text("Test");
		// 	ImGui::SameLine();
		// 	ImGui::PushItemWidth(node_width);
		// 	ImGui::DragFloat("##hidelabel", &value, 0.01f, 0.0f, 1.0f);
		// 	ImGui::PopItemWidth();
		// 	ImNodes::EndInputAttribute();
		// }

		for (const int& o : outputs) {
			// ImNodes::BeginInputAttribute(i);
			// ImGui::PushItemWidth(node_width);
			// ImGui::Text("input %d", i);
			// // ImGui::DragFloat("##hidelabel", &value, 0.01f, 0.0f, 1.0f);
			// ImGui::PopItemWidth();
			// ImNodes::EndInputAttribute();

			ImNodes::BeginOutputAttribute(o);
			ImGui::Indent(40);
			ImGui::Text("output %d", o);
			ImNodes::EndOutputAttribute();
		}

		ImNodes::EndNode();
	}

	bool hasInput(int id) {
		for (const int& inp : inputs) {
			if (inp == id) {
				return true;
			}
		}

		return false;
	}

	bool hasOutput(int id) {
		for (const int& outp : outputs) {
			if (outp == id) {
				return true;
			}
		}

		return false;
	}

	bool giveInput(int id, DynamcTexture* dynamc) {
		int input_index = 0;
		for (const int& inp : inputs) {
			if (id == inp) {
				break;
			}
			input_index++;
		}

		return generator->setInput(input_index, dynamc);
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