#include "NodeEditor.hpp"

NodeEditor::NodeEditor() {
	printf("Node beg\n");
	ImNodes::CreateContext();
	ImNodes::StyleColorsDark();
    // ImNodes::PushAttributeFlag(ImNodesAttributeFlags_EnableLinkDetachWithDragClick);
    ImNodes::GetIO().LinkDetachWithModifierClick.Modifier = &ImGui::GetIO().KeyCtrl;

	// UiNode node1;
	// node1.id = ++current_id;
	// node1.input = ++current_id;
	// node1.output = ++current_id;

	// UiNode node2;
	// node2.id = ++current_id;
	// node2.input = ++current_id;
	// node2.output = ++current_id;

	// nodes[node1.id] = node1;
	// nodes[node2.id] = node2;
	// nodes.push_back(node1);
	// nodes.push_back(node2);
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

		bool addNode = false;
		int type = 0;
		ImU32 color = NODE_COLOR_DEFAULT;
		ImU32 colorSelected = NODE_COLOR_DEFAULT_SELECTED;

		if (ImGui::MenuItem("Sin")) {
			addNode = true;
		} 

		if (ImGui::MenuItem("Grad")) {
			addNode = true;
			type = 1;
		} 
		
		if (ImGui::MenuItem("Combine")) {
			addNode = true;
			type = 2;
			color = NODE_COLOR_GREEN;
			colorSelected = NODE_COLOR_GREEN_SELECTED;
		} 

		if (addNode) {
			unique_ptr<Generator> generator;
			switch (type) {
				case 0: generator = make_unique<SinGenerator>(); break;
				case 1: generator = make_unique<GradGenerator>(); break;
				case 2: generator = make_unique<CombinerGenerator>(); break;
			}

			shared_ptr<UiNode> node = make_shared<UiNode>(move(generator));
			node->id = ++current_id;
			if (type == 2) {
				node->inputs.push_back(++current_id);
				node->inputs.push_back(++current_id);
			}

			node->outputs.push_back(++current_id);
			node->setColors(color, colorSelected);

			nodes.push_back(node);

			ImNodes::SetNodeScreenSpacePos(node->id, mouse_pos);
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
	for (shared_ptr<UiNode>& node : nodes) {
	// for (map<int, UiNode>::iterator it = nodes.begin(); it != nodes.end(); it++) {
		node->draw();
	}
	// ImNodes::MiniMap();

	// Drawing links between nodes
	int link_i = 10;
	for (const auto& link : links) {
		ImNodes::Link(link_i++, link.beg, link.end);
	}

	ImNodes::EndNodeEditor();

	int beg, end;
	if (ImNodes::IsLinkCreated(&beg, &end)) {
		shared_ptr<UiNode> beg_node = nullptr;
		shared_ptr<UiNode> end_node = nullptr;
		int nodesGot = 0;


		printf("attempt %d %d\n", beg, end);

		for (shared_ptr<UiNode>& node : nodes) {
			if (node->hasOutput(beg)) {
				beg_node = node;
				nodesGot++;
				printf("got output: %d\n", node->id);
			} else if(node->hasInput(end)) {
				end_node = node;
				nodesGot++;
				printf("got input: %d\n", node->id);
			}
		}

		printf("Nodes got: %d\n", nodesGot);

		if (nodesGot == 2 && end_node->giveInput(end, &beg_node->dynamc)) {
			Link link = {
				links_id++,
				beg, end,
				beg_node->id, end_node->id
			};

			links.push_back(link);
			beg_node->addLink(link);
			end_node->addLink(link);

			printf("link[%d]! %d %d\n", links.size() - 1, beg, end);
		}
	}

	int destroyId;
	if (ImNodes::IsLinkDestroyed(&destroyId)) {
		printf("Link id: %d\n", destroyId);
		destroyId -= 10;
		Link& link = links[destroyId];
		printf("link: %d\n", link.id);

		shared_ptr<UiNode> beg_node = nullptr;
		shared_ptr<UiNode> end_node = nullptr;
		int nodesGot = 0;

		printf("removing %d %d\n", link.beg, link.end);

		for (shared_ptr<UiNode>& node : nodes) {
			if (node->hasOutput(link.beg)) {
				beg_node = node;
				nodesGot++;
				printf("got output: %d\n", node->id);
			} else if(node->hasInput(link.end)) {
				end_node = node;
				nodesGot++;
				printf("got input: %d\n", node->id);
			}
		}

		if (nodesGot == 2) {
			if (end_node->unsetInput(link.end)) {
				int nremoved = beg_node->removeLink(link.id) * 1;
				nremoved += end_node->removeLink(link.id) * 1;
				if (nremoved != 2) {
					printf("\t\tnremoved != 2\n");
				}
				links.erase(links.begin() + destroyId);
			} else {
				printf("Could not unset node\n");
			}
		}
	}

	// Check for node selecting
	int node_id;
	if (ImGui::IsMouseDoubleClicked(0) && ImNodes::IsNodeHovered(&node_id)) {
		for (shared_ptr<UiNode>& n : nodes) {
			if ( n->id == node_id) {
				selectedNode = n;
				break;
			}
		}
		printf("doubleclicked node: %d\n", node_id);
	}

	// Removing nodes
	if (ImGui::IsKeyPressed(GLFW_KEY_DELETE)) {
		int n_nodes_selected = ImNodes::NumSelectedNodes();
		if (n_nodes_selected > 0) {
			vector<int> node_ids;
			node_ids.resize(n_nodes_selected);
			
			ImNodes::GetSelectedNodes(node_ids.data());
			printf("Selected nods: ");
			for (int node_id : node_ids) {
				// Deleting node

				// printf("%d ", a);
				// node_id, link_id
				vector<pair<int, int>> nodes_linking_to;
				int node_index = 0;
				for (shared_ptr<UiNode>& node : nodes) {
					if ( node->id == node_id) {
						vector<Link>* node_links = &node->links;
						for (int i = 0; i < node_links->size(); i++) {
							Link& link = (*node_links)[i];
							int _link_id = link.id;
							nodes_linking_to.push_back({link.begNode == node_id ? link.endNode : link.begNode, _link_id});
							auto iter = std::find_if(links.begin(), links.end(), [_link_id](const Link& link) -> bool {
									return link.id == _link_id;
								});
							assert(iter != links.end());
							this->links.erase(iter);
						}

						nodes.erase(nodes.begin() + node_index);

						break;
					}
					node_index++;
				}

				for (const auto& [node_id, link_id] : nodes_linking_to) {
					for (shared_ptr<UiNode>& node : nodes) {
						if ( node->id == node_id) {
							node->removeLink(link_id);
						}
					}	
				}
			}
			printf("\n");
		}
	}
}

void NodeEditor::DeleteNode(int nodeId) {
	for (shared_ptr<UiNode>& node : nodes) {
		if (node->id == nodeId) {
			printf("got output: %d\n", node->id);
		}
	}
}

int NodeEditor::getLinksSize() {
	return links.size();
}

NodeEditor::~NodeEditor() {
	ImNodes::DestroyContext();
	printf("Node end\n");
}