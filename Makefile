# Compiler
CXX = x86_64-w64-mingw32-g++.exe

# Output executable
OBJ_NAME = main.exe

# Project source files
SRCS = $(wildcard src/*.cpp)

# Dear ImGui source files
IMGUI_DIR = external/imgui
IMGUI_SRCS = \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/imgui_demo.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

GLM_DIR = external/glm


# Include paths and compiler options
CXXFLAGS = -Wall -g -std=c++17 \
	-IC:/msys64/ucrt64/include/SDL2 \
	-I$(IMGUI_DIR) \
	-I$(IMGUI_DIR)/backends \
	-I$(GLM_DIR)

# Library paths
LDFLAGS = -LC:/msys64/ucrt64/lib

# Libraries
LIBS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf

# Build target
all:
	$(CXX) $(SRCS) $(IMGUI_SRCS) $(CXXFLAGS) $(LDFLAGS) -o $(OBJ_NAME) $(LIBS)

# Clean target
clean:
	rm -f $(OBJ_NAME)

.PHONY: all clean