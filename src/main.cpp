#include <SDL.h>
#include <SDL_image.h>

#include <iostream>
#include <vector>
#include "mapReader.h"


int mapReader::width, mapReader::height, mapReader::n;


int main(int argc, char** argv){


	std::vector<std::vector<colorVals>> map;
	int mapWidth, mapHeight;

	std::string mapName = "res/maps/Map2.png";

	mapReader::Reader(map, mapWidth, mapHeight, mapName);
	mapReader::printMap(map);












	std::cout << "Hello, world!" << std::endl;

	return 0;
}