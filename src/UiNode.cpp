#include "UiNode.hpp"

UiNode::UiNode(unique_ptr<Generator> generator, bool monochromeTexture = true): dynamc{texWidth, texWidth, monochromeTexture}, generator{move(generator)} {
// generator{make_unique<SinGenerator>(&dynamc, texWidth, texWidth)} {
	this->generator->setTexture(&dynamc);

	this->generator->gen();
}

UiNode::UiNode(const json& json_data): dynamc{texWidth, texWidth, json_data["type"] != 3} {
	int type = json_data["type"];
	
	switch (type) {
		case 0: generator = move(make_unique<SinGenerator>()); break;
		case 1: generator = move(make_unique<GradGenerator>()); break;
		case 2: generator = move(make_unique<CombinerGenerator>()); break;
		case 3: generator = move(make_unique<ColorGenerator>()); break;
	}

	this->generator->setTexture(&dynamc);

	// shared_ptr<UiNode> node = make_shared<UiNode>(move(generator), type == 3 ? false : true);
	this->id = json_data["id"];
	this->type_i = type;
	
	ImU32 color = NODE_COLOR_DEFAULT;
	ImU32 colorSelected = NODE_COLOR_DEFAULT_SELECTED;

	if (type == 2) {
		// this->inputs.push_back(++current_id);
		// this->inputs.push_back(++current_id);
		color = NODE_COLOR_GREEN;
		colorSelected = NODE_COLOR_GREEN_SELECTED;
	}

	if (type == 3) {
		// this->inputs.push_back(++current_id);
		color = NODE_COLOR_YELLOW;
		colorSelected = NODE_COLOR_YELLOW_SELECTED;
	}

	for (const int& input : json_data["inputs"]) {
		inputs.push_back(input);
	}

	for (const int& output : json_data["outputs"]) {
		outputs.push_back(output);
	}

	this->setColors(color, colorSelected);
	this->generator->gen();
}

void UiNode::setColors(ImU32 color = IM_COL32(11, 109, 191, 255), ImU32 colorSelected = IM_COL32(81, 148, 204, 255)) {
	this->color = color;
	this->colorSelected = colorSelected;
}

void UiNode::drawGui() {
	generator->drawGui();
	ImGui::Text("Debug information");
	ImGui::Text("Node id: %d", id);
	ImGui::Text("Links %d", links.size());
	for (const Link& link : links) {
		ImGui::Text("id: %d, beg: %d, end: %d, bNode: %d, eNode: %d\n", 
					link.id, link.beg, link.end, link.begNode, link.endNode);
	}
}

void UiNode::draw() {
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

bool UiNode::hasInput(int id) {
	for (const int& inp : inputs) {
		if (inp == id) {
			return true;
		}
	}

	return false;
}

bool UiNode::hasOutput(int id) {
	for (const int& outp : outputs) {
		if (outp == id) {
			return true;
		}
	}

	return false;
}

bool UiNode::giveInput(int id, DynamcTexture* dynamc) {
	int input_index = 0;
	for (const int& inp : inputs) {
		if (id == inp) {
			break;
		}
		input_index++;
	}

	return generator->setInput(input_index, dynamc);
}

bool UiNode::unsetInput(int id) {
	printf("\tunsetting id: %d\n", id);
	int input_index = 0;
	for (const int& inp : inputs) {
		if (id == inp) {
			return generator->unsetInput(input_index);
		}
		input_index++;
	}

	printf("\tinput index = %d\n", input_index);

	return false;	
}

bool UiNode::addLink(Link link) {
	links.push_back(link);
}

bool UiNode::removeLink(int id) {
	for (auto link = links.begin(); link != links.end(); link++) {
		if (link->id == id) {
			bool removable = link->endNode == this->id ? this->unsetInput(link->end) : true;
			if (removable) {
				links.erase(link);
			}
			return removable;
		}
	}
	return false;
}

json UiNode::serialize() {
	json result;
	result["id"] = id;
	result["type"] = type_i;
	result["inputs"] = inputs;
	result["outputs"] = outputs;

	return result;
}

UiNode::~UiNode() {
	printf("Removing node: %d\n", id);
}