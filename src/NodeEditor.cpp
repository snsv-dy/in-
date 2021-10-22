#include "NodeEditor.hpp"

NodeEditor::NodeEditor() {
	printf("Node beg\n");
	ImNodes::CreateContext();
	ImNodes::StyleColorsDark();
    // ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
    ImNodes::GetIO().LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

	UiNode node1;
	node1.id = ++current_id;
	node1.input = ++current_id;
	node1.output = ++current_id;

	UiNode node2;
	node2.id = ++current_id;
	node2.input = ++current_id;
	node2.output = ++current_id;

	// nodes[node1.id] = node1;
	// nodes[node2.id] = node2;
	nodes.push_back(node1);
	nodes.push_back(node2);
}

void NodeEditor::draw() {
	ImGui::Begin("Nodes");
	ImNodes::BeginNodeEditor();

	const bool node_add_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
								ImNodes::IsEditorHovered() &&
								ImGui::IsMouseReleased(1);
	
	if (ImGui::IsAnyItemHovered && node_add_popup) {
		ImGui::OpenPopup("Add node");
	} 
	// else {
	// 	printf("no popup: %d %d %d %d\n", !ImGui::IsAnyItemHovered(), ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows),
	// 						ImNodes::IsEditorHovered(),
	// 						ImGui::IsMouseReleased(2));
	// }

	if (ImGui::BeginPopup("Add node")) {
		const ImVec2 mouse_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

		if (ImGui::MenuItem("node")) {
			UiNode node;
			node.id = ++current_id;
			node.input = ++current_id;
			node.output = ++current_id;

			nodes.push_back(node);

			ImNodes::SetNodeScreenSpacePos(node.id, mouse_pos);
		}


		// if (ImGui::MenuItem("node1")) {
		// 	UiNode node;
		// 	node.id = ++current_id;
		// 	node.input = ++current_id;
		// 	node.output = ++current_id;

		// 	nodes[node.id] = node;

		// 	ImNodes::SetNodeScreenSpacePos(node.id, mouse_pos);
		// }

		ImGui::EndPopup();
	}

	const float node_width = 100.0f;
	for (UiNode& node : nodes) {
	// for (map<int, UiNode>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		ImNodes::BeginNode(node.id);

		ImNodes::BeginNodeTitleBar();
		// string 
		ImGui::Text("Node %d", node.id);
		ImNodes::EndNodeTitleBar();

		ImNodes::BeginInputAttribute(node.input);
		// ImGui::Text("input");
		// ImGui::SameLine();
		ImGui::PushItemWidth(node_width);
		ImGui::DragFloat("##hidelabel", &node.value, 0.01f, 0.0f, 1.0f);
		ImGui::PopItemWidth();
		ImNodes::EndInputAttribute();

		ImNodes::BeginOutputAttribute(node.output);
		ImGui::Indent(40);
		ImGui::Text("output");
		ImNodes::EndOutputAttribute();

		ImNodes::EndNode();
	}
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