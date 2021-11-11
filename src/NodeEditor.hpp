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

using namespace std;

// struct Node {

// };

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

	ImU32 color = IM_COL32(11, 109, 191, 255);
	ImU32 colorSelected = IM_COL32(81, 148, 204, 255);

	UiNode(unique_ptr<Generator> generator): dynamc{texWidth, texWidth}, generator{move(generator)} {
	// generator{make_unique<SinGenerator>(&dynamc, texWidth, texWidth)} {
		this->generator->setTexture(&dynamc);

		this->generator->gen();
	}

	void setColors(ImU32 color = IM_COL32(11, 109, 191, 255), ImU32 colorSelected = IM_COL32(81, 148, 204, 255)) {
		this->color = color;
		this->colorSelected = colorSelected;
	}

	void drawGui() {
		generator->drawGui();
		ImGui::Text("Debug information");
		ImGui::Text("Node id: %d", id);
		ImGui::Text("Links %d", links.size());
		for (const Link& link : links) {
			ImGui::Text("id: %d, beg: %d, end: %d, bNode: %d, eNode: %d\n", 
						link.id, link.beg, link.end, link.begNode, link.endNode);
		}
	}

	void draw() {
		const float node_width = 100.0f;
		ImNodes::PushColorStyle(ImNodesCol_TitleBar, color);
		ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, colorSelected);
		ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, colorSelected);

		ImNodes::BeginNode(id);

		ImNodes::BeginNodeTitleBar();
		// string 
		ImGui::Text(generator->getName());
		ImNodes::EndNodeTitleBar();

		int input_index = 1;
		for (const int& i : inputs) {
			ImNodes::BeginInputAttribute(i);
			ImGui::PushItemWidth(node_width);
			ImGui::Text("input %d", input_index++);
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

		int output_index = 1;
		for (const int& o : outputs) {
			// ImNodes::BeginInputAttribute(i);
			// ImGui::PushItemWidth(node_width);
			// ImGui::Text("input %d", i);
			// // ImGui::DragFloat("##hidelabel", &value, 0.01f, 0.0f, 1.0f);
			// ImGui::PopItemWidth();
			// ImNodes::EndInputAttribute();

			ImNodes::BeginOutputAttribute(o);
			ImGui::Indent(40);
			ImGui::Text("output %d", output_index++);
			ImNodes::EndOutputAttribute();
		}

		ImNodes::EndNode();
		
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
		ImNodes::PopColorStyle();
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

	bool unsetInput(int id) {
		int input_index = 0;
		for (const int& inp : inputs) {
			if (id == inp) {
				break;
			}
			input_index++;
		}

		// printf("unsetting index: %d\n", input_index);

		return generator->unsetInput(input_index);
	}

	bool addLink(Link link) {
		links.push_back(link);
	}

	bool removeLink(int id) {
		for (auto link = links.begin(); link != links.end(); link++) {
			if (link->id == id) {
				links.erase(link);
				if (link->endNode == this->id) {
					this->unsetInput(link->end);
				}
				return true;
			}
		}
		return false;
	}
};

class NodeEditor {
	vector<Link> links; // change to list?
	vector<shared_ptr<UiNode>> nodes;
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