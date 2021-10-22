
.PHONY = all

IMGUI_DIR = ../imgui
OBJ_DIR = bin

VPATH = include/glad include/imnodes src

EXECUTABLE = prog
SRC = main.cpp include/glad/glad.c Shader.cpp NodeEditor.cpp
SRC += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/backends/imgui_impl_glfw.cpp $(IMGUI_DIR)/backends/imgui_impl_opengl3.cpp
SRC += $(IMGUI_DIR)/imgui.cpp $(IMGUI_DIR)/imgui_demo.cpp $(IMGUI_DIR)/imgui_draw.cpp $(IMGUI_DIR)/imgui_tables.cpp $(IMGUI_DIR)/imgui_widgets.cpp
SRC += include/imnodes/imnodes.cpp
OBJS = $(addsuffix .o, $(basename $(notdir $(SRC))))
OBJS := $(OBJS:%.o=$(OBJ_DIR)/%.o)

$(info $(OBJS))

CC = g++
CCFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends -I include
CCFLAGS += -g -Wall -Wformat -std=c++17
LFLAGS = -L. -lGL -ldl -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread

all: $(OBJ_DIR)/$(EXECUTABLE)
	@echo eh

$(OBJ_DIR)/%.o:%.cpp
	$(CC) $(CCFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:%.c
	$(CC) $(CCFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:$(IMGUI_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o:$(IMGUI_DIR)/backends/%.cpp
	$(CC) $(CCFLAGS) -c -o $@ $<

$(OBJ_DIR)/$(EXECUTABLE): $(OBJS)
	$(CC) -o $@ $^ $(CCFLAGS) $(LFLAGS)