#include "UiNode.hpp"

UiNode::UiNode(unique_ptr<Generator> generator): dynamc{texWidth, texWidth}, generator{move(generator)} {
// generator{make_unique<SinGenerator>(&dynamc, texWidth, texWidth)} {
	this->generator->setTexture(&dynamc);

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

UiNode::~UiNode() {
	printf("Removing node: %d\n", id);
}