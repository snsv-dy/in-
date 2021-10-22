#include "NodeEditor.hpp"

NodeEditor::NodeEditor() {
	printf("Node beg\n");
	ImNodes::CreateContext();
	ImNodes::StyleColorsDark();
    // ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
    ImNodes::GetIO().LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;
}

void NodeEditor::draw() {
	ImGui::Begin("Nodes");
	ImNodes::BeginNodeEditor();

	ImNodes::BeginNode(1);

	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted("simple node xD");
	ImNodes::EndNodeTitleBar();

	ImNodes::BeginInputAttribute(2);
	ImGui::Text("input");
	ImNodes::EndInputAttribute();

	ImNodes::BeginOutputAttribute(3);
	ImGui::Indent(40);
	ImGui::Text("output");
	ImNodes::EndOutputAttribute();

	ImNodes::EndNode();

	ImNodes::BeginNode(4);

	ImNodes::BeginNodeTitleBar();
	ImGui::TextUnformatted("Same things make us laugh");
	ImNodes::EndNodeTitleBar();

	ImNodes::BeginInputAttribute(5);
	ImGui::Text("input");
	ImNodes::EndInputAttribute();

	ImNodes::BeginOutputAttribute(6);
	ImGui::Indent(40);
	ImGui::Text("output");
	ImNodes::EndOutputAttribute();

	ImNodes::EndNode();

	// ImNodes::MiniMap();

	// Drawing links between nodes
	for (const auto& [id, link] : links) {
		ImNodes::Link(id, link.beg, link.end);
	}

	ImNodes::EndNodeEditor();

	int beg, end;
	if (ImNodes::IsLinkCreated(&beg, &end)) {
		int link_id = current_id++;
		links[link_id] = {link_id, beg, end};

		printf("link! %d %d\n", beg, end);
	}

	int destroyId;
	if (ImNodes::IsLinkDestroyed(&destroyId)) {
		links.erase(destroyId);
	}
}

int NodeEditor::getLinksSize() {
	return links.size();
}

NodeEditor::~NodeEditor() {
	ImNodes::DestroyContext();
	printf("Node end\n");
}