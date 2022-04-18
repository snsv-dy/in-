# Thesis project
Simple heightmap generator, that creates terrain by combining nodes responsible for modyfying terrain in some way.

## Features
* Noise generation
* Hydraulic erosion using droplet simulation method
* Basic coloring based on height
* Heightmap import/export
* Saving project in JSON format
* Preview of multiple nodes at once

https://user-images.githubusercontent.com/52980049/163799143-85558d8a-0201-4711-b805-342ca961dbe7.mp4

## Used libraries
* https://github.com/nlohmann/json
* ![Dear ImGui](https://github.com/ocornut/imgui), with extensions:
	* ![imnodes](https://github.com/Nelarius/imnodes)
	* ![ImGuiFileDialog](https://github.com/aiekick/ImGuiFileDialog)
* ![FastNoiseLite](https://github.com/Auburn/FastNoiseLite)
* ![GLFW](https://www.glfw.org/)
* ![GLAD](https://glad.dav1d.de/)
* Parts from ![SebLague's hydraulic erosion](https://github.com/SebLague/Hydraulic-Erosion) code

