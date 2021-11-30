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

void NodeEditor::reset() {
	links.clear();
	nodes.clear();
	current_id = 0;
	links_id = 0;
	selectedNode = nullptr;
}

// Dodaj jakieś komunikaty w gui w przypadku błędów przy zapisie/odczycie.
void NodeEditor::save(const string& filename) {
	json json_data;

	json json_nodes = json::array();
	for (auto& [id, node] : nodes) {
		json node_json = node->serialize();
		
		ImVec2 gridSpace = ImNodes::GetNodeGridSpacePos(id);
		node_json["position"] = {
			{"x", gridSpace.x}, {"y", gridSpace.y}
		};

		json_nodes.push_back(node_json);
	}
	json_data["nodes"] = json_nodes;

 	json json_links = json::array();
	for (auto& [id, link] : links) {
		json_links.push_back({
			{"id", link.id},
			{"beg", link.beg},
			{"end",  link.end},
			{"begNode", link.begNode},
			{"endNode", link.endNode}
		});
	}
	json_data["links"] = json_links;

	json_data["current_id"] = current_id;
	json_data["links_id"] = links_id;
	
	ofstream out_file (filename);
	if (out_file.is_open()) {
		out_file << setw(4) << json_data;
		if (out_file.bad()) {
			printf("Out file bad\n");	
		}
		out_file.close();
	} else {
		printf("File not open\n");
	}
}

bool NodeEditor::verify_link(const Link& link) {
	return nodes.count(link.begNode) == 1 &&
			nodes.count(link.endNode) == 1;
}

// Topological sort
void topSort() {

}

void NodeEditor::load(const string& filename) {
	ifstream in_file(filename);

	if (in_file.good()) {
		json json_data;
		try {
			in_file >> json_data;

			this->reset();

			current_id = json_data["current_id"];
			links_id = json_data["links_id"];

			for (const json& node : json_data["nodes"] ) {
				unpackNode(node);
			}

			for (const json& link : json_data["links"] ) {
				Link l = {
					link["id"],
					link["beg"],
					link["end"],
					link["begNode"],
					link["endNode"]
				};

				if (verify_link(l)) {
					links[l.id] = l;
					shared_ptr<UiNode>& begNode = nodes[l.begNode];
					shared_ptr<UiNode>& endNode = nodes[l.endNode];
					begNode->addLink(l);
					endNode->addLink(l);
					endNode->giveInput(l.end, &begNode->dynamc);
				}
			}

			refreshAll();

		} catch (const json::parse_error& err) {
			printf("Exception while reading file: %s\n", err.what());
		}
	}
}

// Temporary for loading
// Currently only for single graph, if there are more graphs in editor
// behaviour is not known.
void NodeEditor::refreshAll() {

	std::set<int> visited;
	for (const auto& [id, link] : links) {
		if (visited.count(link.begNode) == 0) {
			visited.insert(link.begNode);
		}
	}

	vector<int> last_nodes;

	// Find last node in graph
	for (const auto& [id, node] : nodes) {
		if (visited.count(id) == 0) {
			last_nodes.push_back(id);
		}
	}

	vector<int> stack;
	vector<int> order;
	for (const int& id : last_nodes) {
		const shared_ptr<UiNode>& node = nodes[id];
		stack.push_back(id);

		while (!stack.empty()) {
			const int id = stack.back();
			stack.pop_back();
			const shared_ptr<UiNode>& node = nodes[id];
			bool isConnectedTo = false;
			for (const Link& link : node->links) {
				if (link.endNode == id) {
					isConnectedTo = true;
					stack.push_back(link.begNode);
				}
			}

			if (isConnectedTo) {
				order.push_back(id);
			}
		}
	}

	printf("Last nodes: ");
	for (int in : last_nodes) {
		printf("%d ", in);
	}
	printf("\n");

	reverse(order.begin(), order.end());

	for (int& id : order) {
		nodes[id]->generator->gen();
	}
}

void NodeEditor::unpackNode(const json& json_node) {
	shared_ptr<UiNode> node = make_shared<UiNode>(json_node);

	ImVec2 position {json_node["position"]["x"], json_node["position"]["y"]};
	ImNodes::SetNodeGridSpacePos(node->id, position);
	nodes[node->id] = node;
}

void NodeEditor::addNode(const int& type, int id, ImVec2 position) {
	unique_ptr<Generator> generator;

	ImU32 color = NODE_COLOR_DEFAULT;
	ImU32 colorSelected = NODE_COLOR_DEFAULT_SELECTED;

	switch (type) {
		case 0: generator = make_unique<SinGenerator>(); break;
		case 1: generator = make_unique<GradGenerator>(); break;
		case 2: generator = make_unique<CombinerGenerator>(); break;
		case 3: generator = make_unique<ColorGenerator>(); break;
	}

	shared_ptr<UiNode> node = make_shared<UiNode>(move(generator), type == 3 ? false : true);
	node->id = ++current_id;

	node->type_i = type;
	if (type == 2) {
		node->inputs.push_back(++current_id);
		node->inputs.push_back(++current_id);
		color = NODE_COLOR_GREEN;
		colorSelected = NODE_COLOR_GREEN_SELECTED;
	}

	if (type == 3) {
		node->inputs.push_back(++current_id);
		color = NODE_COLOR_YELLOW;
		colorSelected = NODE_COLOR_YELLOW_SELECTED;
	}

	node->outputs.push_back(++current_id);
	node->setColors(color, colorSelected);

	// nodes.push_back(node);
	nodes[node->id] = node;

	ImNodes::SetNodeScreenSpacePos(node->id, position);
}

void NodeEditor::debgz() {
	ImGui::Text("nodes size: %d", nodes.size());
}

void NodeEditor::draw(bool* new_preview, int* new_preview_id) {
	if (new_preview != nullptr) {
		*new_preview = false;
	}

	if (new_preview_id != nullptr) {
		*new_preview_id = -1;
	}

	ImGui::Begin("Nodes");
	ImNodes::BeginNodeEditor();

	const bool node_add_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
								ImNodes::IsEditorHovered() &&
								ImGui::IsMouseReleased(1);
	bool right_click = false;
	if (ImGui::IsAnyItemHovered && node_add_popup) {
		ImGui::OpenPopup("Add node");
		right_click = true;
	} 
	// else {
	// 	printf("no popup: %d %d %d %d\n", !ImGui::IsAnyItemHovered(), ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows),
	// 						ImNodes::IsEditorHovered(),
	// 						ImGui::IsMouseReleased(2));
	// }


	if (ImGui::BeginPopup("Add node")) {
		const ImVec2 mouse_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

		bool addingNode = false;
		int type = 0;
		ImU32 color = NODE_COLOR_DEFAULT;
		ImU32 colorSelected = NODE_COLOR_DEFAULT_SELECTED;

		if (ImGui::MenuItem("Sin")) {
			addingNode = true;
		} 

		if (ImGui::MenuItem("Grad")) {
			addingNode = true;
			type = 1;
		} 
		
		if (ImGui::MenuItem("Combine")) {
			addingNode = true;
			type = 2;
		} 
		
		if (ImGui::MenuItem("Color")) {
			addingNode = true;
			type = 3;
		} 

		if (addingNode) {
			addNode(type, -1, mouse_pos);
		}

		ImGui::EndPopup();
	}

	//
	// Drawing nodes
	//
	const float node_width = 100.0f;
	// for (shared_ptr<UiNode>& node : nodes) {
	// for (map<int, UiNode>::iterator it = nodes.begin(); it != nodes.end(); it++) {
	for (auto& [id, node] : nodes) {
		if (node->draw()) {
			previewedNodes.insert(id);
			if (new_preview != nullptr) {
				*new_preview = true;
			}
			if (new_preview_id != nullptr) {
				*new_preview_id = id;
			}
		}
	}
	// ImNodes::MiniMap();

	// Drawing links between nodes
	int link_i = 0;
	for (const auto& [id, link] : links) {
		ImNodes::Link(id, link.beg, link.end);
	}

	ImNodes::EndNodeEditor();
	ImGui::End(); // If this is not after EndNodeEditor, then something messes up in imgui window stack.
	
	int n_nodes_selected = ImNodes::NumSelectedNodes();
	if (n_nodes_selected > 0 && right_click) {
		printf("nodes selected > 0\n");
		// ImGui::MenuItem("Nodes selected: > 0");
		// vector<int> node_ids;
		// node_ids.resize(n_nodes_selected);
	}

	int beg, end;
	if (ImNodes::IsLinkCreated(&beg, &end)) {
		addLink(beg, end);
	}

	int destroyId;
	if (ImNodes::IsLinkDestroyed(&destroyId)) {
		printf("Link id: %d\n", destroyId);
		Link& link = links[destroyId];
		printf("link: %d\n", link.id);

		shared_ptr<UiNode>& beg_node = nodes[link.begNode];
		shared_ptr<UiNode>& end_node = nodes[link.endNode];
		int nodesGot = 2;

		printf("removing %d %d\n", link.beg, link.end);

		if (nodesGot == 2) {
			if (end_node->removeLink(link.id)) {
				int nremoved = beg_node->removeLink(link.id) * 1;
				if (nremoved != 1) {
					printf("\t\tnremoved != 2\n");
				}
				nodesChanged({link.endNode});
				links.erase(destroyId);
			} else {
				printf("Could not unset node\n");
			}
		}
	}

	// Check for node selecting
	int node_id;
	if (ImGui::IsMouseDoubleClicked(0) && ImNodes::IsNodeHovered(&node_id)) {
		if (auto elem = nodes.find(node_id); elem != nodes.end()) {
			selectedNode = elem->second;
		}
		printf("doubleclicked node: %d\n", node_id);
	}

	
	// Removing nodes
	if (ImGui::IsKeyPressed(GLFW_KEY_DELETE)) {
		set<int> nodes_to_update;

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
				// int node_index = 0;
				// for (shared_ptr<UiNode>& node : nodes) {
				// 	if ( node->id == node_id) {
				shared_ptr<UiNode>& node = nodes[node_id];
				if (auto node_pair = nodes.find(node_id); node_pair != nodes.end()) {
					shared_ptr<UiNode> node = node_pair->second;
					vector<Link>* node_links = &node->links;
					for (int i = 0; i < node_links->size(); i++) {
						Link& link = (*node_links)[i];

						if (link.begNode == node_id) {
							nodes_to_update.insert(link.endNode);
						}

						nodes_linking_to.push_back({link.begNode == node_id ? link.endNode : link.begNode, link.id});
						// ImNodes::ClearLinkSelection(link.id);
						links.erase(link.id);
						// auto iter = std::find_if(links.begin(), links.end(), [_link_id](const Link& link) -> bool {
						// 		return link.id == _link_id;
						// 	});
						// assert(iter != links.end());
						// this->links.erase(iter);
					}

					if (selectedNode != nullptr && selectedNode->id == node->id) {
						selectedNode = nullptr;
					}

					ImNodes::ClearNodeSelection(node_id);
					nodes.erase(node->id);
				}
						// break;
					// }
					// node_index++;
				// }

				for (const auto& [node_id, link_id] : nodes_linking_to) {
					if (auto node_pair = nodes.find(node_id); node_pair != nodes.end()) {
						node_pair->second->removeLink(link_id);
					}
				}
			}
			printf("\n");
		}

		int n_links_selected = ImNodes::NumSelectedLinks();

		if (n_links_selected > 0) {
			vector<int> link_ids;
			link_ids.resize(n_links_selected);
			
			ImNodes::GetSelectedLinks(link_ids.data());
			for (const int& link_id : link_ids) {
				if (auto link_it = links.find(link_id); link_it != links.end()) {
					Link& link = link_it->second;

					if (auto node_pair = nodes.find(link.begNode); node_pair != nodes.end()) {
						node_pair->second->removeLink(link_id);
					}

					if (auto node_pair = nodes.find(link.endNode); node_pair != nodes.end()) {
						node_pair->second->removeLink(link_id);
					}

					nodes_to_update.insert(link.endNode);
					links.erase(link.id);
				}
				ImNodes::ClearLinkSelection(link_id);
			}
		}

		// This is unoptimized because some nodes might be refreshed twice.
		// To optimize this you would somehow sort nodes like in loading.
		// But sorting in loading is still weird and won't work 
		// there are loops in graph (there shouldn't be any by the way).
		
		set<int> visited;
		vector<int> queue;
		// queue.push_back(node_id);
		
		for (const int& id : nodes_to_update) {
			if (auto node_pair = nodes.find(id); node_pair != nodes.end()) {
				// nodesChanged(id);
				queue.push_back(id);
			}
		}

		nodesChanged(queue);
	}
}

void NodeEditor::addLink(const int& beg, const int& end) {
	shared_ptr<UiNode> beg_node = nullptr;
	shared_ptr<UiNode> end_node = nullptr;
	int nodesGot = 0;

	printf("attempt %d %d\n", beg, end);

	for (auto& [id, node] : nodes) {
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

		links[link.id] = link;
		beg_node->addLink(link);
		end_node->addLink(link);

		printf("link[%d]! %d %d\n", links.size() - 1, beg, end);
		nodesChanged({beg_node->id});
	}
}

// Updated nodes so that no node will be updated twice and in wrong order.
void NodeEditor::nodesChanged(const vector<int> node_ids) {
	set<int> order_visited;
	list<int> order;

	//
	// Bfs here
	for (const int& current_id : node_ids) {
		set<int> local_visited;
		queue<int> local_queue;
		local_queue.push(current_id);

		printf("%d bfs, updating nodes: ", current_id);
		while (!local_queue.empty()) {
			int id = local_queue.front();
			local_queue.pop();

			if (local_visited.count(id) == 0) {
				printf("%d ", id);
				local_visited.insert(id);

				const shared_ptr<UiNode>& node = nodes[id];
				for (const Link& link : node->links) {
					if (link.begNode == id) {
						local_queue.push(link.endNode);
					}
				}
			} 
			
			if (order_visited.count(id) == 1) {
				// Change order so this node will be updated later.
				order.erase(find(order.begin(), order.end(), id));
				order.push_back(id);
			} else {
				order.push_back(id);
			}

			order_visited.insert(id);
		}
	}

	printf("\n");

	printf("Order of nodes: ");
	for (int id : order) {
		const shared_ptr<UiNode>& node = nodes[id];
		node->generator->gen();
		printf("%d ", id);
	}
	printf("\n");
}

void NodeEditor::DeleteNode(int nodeId) {
	// for (shared_ptr<UiNode>& node : nodes) {
	// 	if (node->id == nodeId) {
	// 		printf("got output: %d\n", node->id);
	// 	}
	// }
}

int NodeEditor::getLinksSize() {
	return links.size();
}

shared_ptr<UiNode> NodeEditor::getNode(const int& id) {
	if (auto it = nodes.find(id); it != nodes.end()) {
		return it->second;
	}

	return nullptr;
}

NodeEditor::~NodeEditor() {
	ImNodes::DestroyContext();
	printf("Node end\n");
}