# SDL2 Game Engine

This project is a small SDL2-based game engine prototype with resource caching, an ImGui-backed debug UI, a renderer wrapper, and a lightweight object/component system.

Made by Matthew Bannister and Alex Riddell.

## Documentation

The generated documentation lives in [Documentation/Index.md](/Documentation/Index.md).

Start with [Documentation/Overview.md](/Documentation/Overview.md) for the architecture and [Documentation/Resource_Manager.md](/Documentation/Resource_Manager.md) for the asset cache.

# Git etiquette

We will be using feature branches to implement main features from the trello page.
Smaller issues can be used and referenced via the issue tracker, for every commit reference either issue tracker or trello.

Once a feature has been done on the branch it is then ready to merge and you will make a pull request to merge to main.

Milestone is an option we may decide at a later data

# Code conventions

Comment methods, return, args, brief description
Code indentation is tabs set to space 4.
Method scopes should be done on a new line.
Put spaces between oporators e.g. char var='c'; ==> char var = 'c';

pdata for a pointer to data

## Methods
Capitals
Camel case

## Variables
Private scope - start with underscore lowercase
Public - start with capital
Camel case

RAII usage for heap objects (make_unique, smart ptrs)

# Debugging
usage of #ifdef and assert statments when possible
unit tests - https://github.com/catchorg/Catch2

# [Documentation](/Documentation/Index.md)
