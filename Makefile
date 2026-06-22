# ============================================================
# SDL2 Game Engine - Makefile
# ============================================================

SHELL := /usr/bin/bash

# Compiler and build tools
CXX   := x86_64-w64-mingw32-g++.exe
CMAKE := cmake

# Game identity
GAME_NAME := SDL2_GameEngine

# Output locations
BIN_DIR       := bin
DIST_DIR      := dist
DEBUG_EXE     := $(BIN_DIR)/$(GAME_NAME)_debug.exe
RELEASE_EXE   := $(BIN_DIR)/$(GAME_NAME).exe

PACKAGE_DIR   := $(DIST_DIR)/$(GAME_NAME)-windows-x64
PACKAGE_ZIP   := $(DIST_DIR)/$(GAME_NAME)-windows-x64.zip

# ------------------------------------------------------------
# Source files
# Uses find so future folders such as src/physics/ also compile.
# ------------------------------------------------------------

SRC_DIR := src
SRCS := $(wildcard $(SRC_DIR)/*.cpp)

# ------------------------------------------------------------
# Dear ImGui
# ------------------------------------------------------------

IMGUI_DIR := external/imgui

IMGUI_SRCS := \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/imgui_demo.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdlrenderer2.cpp

# ------------------------------------------------------------
# GLM
# ------------------------------------------------------------

GLM_DIR := external/glm

# ------------------------------------------------------------
# Box2D
# ------------------------------------------------------------

BOX2D_DIR   := external/box2d
BOX2D_BUILD := $(BOX2D_DIR)/build-ucrt64
BOX2D_LIB   := $(BOX2D_BUILD)/src/libbox2d.a

# ------------------------------------------------------------
# Compiler flags
# ------------------------------------------------------------

COMMON_FLAGS := \
	-Wall \
	-Wextra \
	-std=c++17 \
	-IC:/msys64/ucrt64/include/SDL2 \
	-I$(SRC_DIR) \
	-I$(IMGUI_DIR) \
	-I$(IMGUI_DIR)/backends \
	-I$(GLM_DIR) \
	-I$(BOX2D_DIR)/include

DEBUG_FLAGS := \
	-g \
	-O0

RELEASE_FLAGS := \
	-O2 \
	-DNDEBUG \
	-s

# ------------------------------------------------------------
# Linker paths and libraries
# ------------------------------------------------------------

LDFLAGS := \
	-LC:/msys64/ucrt64/lib

LIBS := \
	-lmingw32 \
	-lSDL2main \
	-lSDL2 \
	-lSDL2_image \
	-lSDL2_ttf \
	$(BOX2D_LIB) \
	-lwinpthread

# ------------------------------------------------------------
# Default target
# ------------------------------------------------------------

all: debug

# ------------------------------------------------------------
# Build Box2D as a static library
# ------------------------------------------------------------

box2d: $(BOX2D_LIB)

$(BOX2D_LIB): $(BOX2D_DIR)/CMakeLists.txt
	@echo Building Box2D...
	$(CMAKE) -S $(BOX2D_DIR) -B $(BOX2D_BUILD) -G Ninja \
		-DCMAKE_BUILD_TYPE=Release \
		-DBUILD_SHARED_LIBS=OFF \
		-DBOX2D_SAMPLES=OFF \
		-DBOX2D_UNIT_TESTS=OFF \
		-DBOX2D_BENCHMARKS=OFF \
		-DBOX2D_DOCS=OFF
	$(CMAKE) --build $(BOX2D_BUILD) --target box2d

# ------------------------------------------------------------
# Debug build
# ------------------------------------------------------------

debug: $(DEBUG_EXE)

$(DEBUG_EXE): $(SRCS) $(IMGUI_SRCS) $(BOX2D_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(SRCS) $(IMGUI_SRCS) \
		$(COMMON_FLAGS) $(DEBUG_FLAGS) \
		$(LDFLAGS) \
		-o $(DEBUG_EXE) \
		$(LIBS)

# ------------------------------------------------------------
# Release build
# ------------------------------------------------------------

release: $(RELEASE_EXE)

$(RELEASE_EXE): $(SRCS) $(IMGUI_SRCS) $(BOX2D_LIB)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(SRCS) $(IMGUI_SRCS) \
		$(COMMON_FLAGS) $(RELEASE_FLAGS) \
		$(LDFLAGS) \
		-static-libgcc \
		-static-libstdc++ \
		-o $(RELEASE_EXE) \
		$(LIBS)

# ------------------------------------------------------------
# Portable Windows package for friends
# ------------------------------------------------------------

package: release
	@echo Creating portable Windows package...
	rm -rf $(PACKAGE_DIR)
	rm -f $(PACKAGE_ZIP)

	mkdir -p $(PACKAGE_DIR)
	mkdir -p $(PACKAGE_DIR)/licenses

	cp $(RELEASE_EXE) $(PACKAGE_DIR)/$(GAME_NAME).exe
	cp SDL2.dll $(PACKAGE_DIR)/
	cp SDL2_image.dll $(PACKAGE_DIR)/
	cp SDL2_ttf.dll $(PACKAGE_DIR)/

	cp -r res $(PACKAGE_DIR)/res
	cp README.md $(PACKAGE_DIR)/README.md

	@if [ -f "$(BOX2D_DIR)/LICENSE" ]; then \
		cp "$(BOX2D_DIR)/LICENSE" "$(PACKAGE_DIR)/licenses/Box2D-LICENSE.txt"; \
	fi

	@if [ -f "$(IMGUI_DIR)/LICENSE.txt" ]; then \
		cp "$(IMGUI_DIR)/LICENSE.txt" "$(PACKAGE_DIR)/licenses/ImGui-LICENSE.txt"; \
	fi

	@if [ -f "$(GLM_DIR)/copying.txt" ]; then \
		cp "$(GLM_DIR)/copying.txt" "$(PACKAGE_DIR)/licenses/GLM-LICENSE.txt"; \
	fi

	@for dll in libgcc_s_seh-1.dll libstdc++-6.dll libwinpthread-1.dll; do \
		if [ -f "/ucrt64/bin/$$dll" ]; then \
			cp "/ucrt64/bin/$$dll" "$(PACKAGE_DIR)/"; \
		fi; \
	done

	powershell.exe -NoProfile -Command "Compress-Archive -Path '$(PACKAGE_DIR)' -DestinationPath '$(PACKAGE_ZIP)' -Force"

	@echo.
	@echo Package created:
	@echo $(PACKAGE_ZIP)

# ------------------------------------------------------------
# Clean output only
# ------------------------------------------------------------

clean:
	rm -f $(DEBUG_EXE)
	rm -f $(RELEASE_EXE)

clean-package:
	rm -rf $(PACKAGE_DIR)
	rm -f $(PACKAGE_ZIP)

clean-box2d:
	rm -rf $(BOX2D_BUILD)

.PHONY: all box2d debug release package clean clean-package clean-box2d