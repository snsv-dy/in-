# Thesis project
Simple heightmap generator, that creates terrain by combining nodes responsible for modyfying terrain in some way.

## Features
* Noise generation
* Hydraulic erosion using droplet simulation method
* Basic coloring based on height
* Heightmap import/export
* Saving project in JSON format
* Preview of multiple nodes at once

## Used libraries
* https://github.com/nlohmann/json
* ![Dear ImGui](https://github.com/ocornut/imgui), with extensions:
	* ![imnodes](https://github.com/Nelarius/imnodes)
	* ![ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)
* ![FastNoiseLite](https://github.com/Auburn/FastNoiseLite)
* ![GLFW](https://www.glfw.org/)
* ![GLAD](https://glad.dav1d.de/)
* Parts from ![SebLague's hydraulic erosion](https://github.com/SebLague/Hydraulic-Erosion) code

