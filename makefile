
.PHONY = all

IMGUI_DIR = ../imgui

VPATH = include/glad

EXECUTABLE = prog
SRC = main.cpp include/glad/glad.c Shader.cpp
SRC += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SRC += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SRC))))

CC = g++
CCFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I include
CCFLAGS += -g -Wall -Wformat -std=c++17
LFLAGS = -L. -lGL -ldl -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread

$(info $(SRC))

all: $(EXECUTABLE)
	@echo eh

%.o:%.cpp
	$(CC) $(CCFLAGS) -c -o $@ $<

%.o:%.c
	$(CC) $(CCFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c -o $@ $<

%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CC) $(CCFLAGS) -c -o $@ $<

$(EXECUTABLE): $(OBJS)
	$(CC) -o $@ $^ $(CCFLAGS) $(LFLAGS)