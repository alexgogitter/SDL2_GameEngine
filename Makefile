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
CMAKE_BUILD_DIR := build/msvc-vs2022-x64
DEBUG_BIN_DIR := $(CMAKE_BUILD_DIR)/Debug
RELEASE_BIN_DIR := $(CMAKE_BUILD_DIR)/Release
DEBUG_EXE     := $(DEBUG_BIN_DIR)/$(GAME_NAME).exe
RELEASE_EXE   := $(RELEASE_BIN_DIR)/$(GAME_NAME).exe

PACKAGE_DIR   := $(DIST_DIR)/Game/Release
PACKAGE_ZIP   := $(DIST_DIR)/Game-windows-x64.zip

# The current engine uses NVIDIA PhysX through vcpkg, so the supported build
# path is MSVC/CMake. The helper script discovers Visual Studio and vcpkg.
MSVC_BUILD := .\\tools\\build_msvc.bat

# ------------------------------------------------------------
# Source files
# Keep the runtime and editor source directories explicit for the legacy build.
# ------------------------------------------------------------

SRC_DIR := src
SRCS := \
	$(wildcard $(SRC_DIR)/EngineCore/*.cpp) \
	$(wildcard $(SRC_DIR)/Editor/*.cpp)

GLAD_DIR := external/glad
GLAD_SRC := $(GLAD_DIR)/src/glad.c

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
	$(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp

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
	-I$(SRC_DIR)/EngineCore \
	-I$(SRC_DIR)/Editor \
	-I$(IMGUI_DIR) \
	-I$(IMGUI_DIR)/backends \
	-I$(GLAD_DIR)/include \
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
	-lopengl32 \
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

debug:
	cmd.exe //d //c $(MSVC_BUILD) Debug $(GAME_NAME)

# ------------------------------------------------------------
# Release build
# ------------------------------------------------------------

release:
	cmd.exe //d //c $(MSVC_BUILD) Release $(GAME_NAME)

# ------------------------------------------------------------
# Portable Windows package for friends
# ------------------------------------------------------------

package:
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File tools/package_game.ps1

scripts:
	cmd.exe //d //c $(MSVC_BUILD) Debug UserScripts

player:
	cmd.exe //d //c $(MSVC_BUILD) Release StageGame

# ------------------------------------------------------------
# Clean output only
# ------------------------------------------------------------

clean:
	rm -f $(DEBUG_EXE)
	rm -f $(RELEASE_EXE)

clean-build:
	rm -rf build/*

clean-package:
	rm -rf $(PACKAGE_DIR)
	rm -f $(PACKAGE_ZIP)

clean-box2d:
	rm -rf $(BOX2D_BUILD)

.PHONY: all box2d debug release package scripts player clean clean-build clean-package clean-box2d
