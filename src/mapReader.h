#ifndef MAPREADER_H
#define MAPREADER_H

#include "stb_image.h"
#include <iostream>
#include <vector>

struct colorVals {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	unsigned char a;
};

// Defining == oporator for the colorVal struct
bool operator==(const colorVals& _a, const colorVals& _b) {
	if (_a.r == _b.r && _a.g == _b.g && _a.b == _b.b && _a.a == _b.a) {
		return true;
	}
	return false;
}


// Color vals refer to these blocks


colorVals CVwall = { 255, 0, 0, 1 };
colorVals CVstart = { 255, 255, 0, 1 };
colorVals CVend = { 0, 255, 0, 1 };
colorVals CVpath = { 0, 0, 255, 1 };




class mapReader {
private:
	// Doesnt work

	// These arent needed here
	// Just a test to see how static variables are dealt with in this
	static int width, height, n;

	static bool pathCheck(std::vector<std::vector<colorVals>>& map, int x, int y) {

		// check up down left right

		// Hit a wall 
		if (map[x][y] == CVwall) {
			return false;
		}
		// We good
		else if (map[x][y] == CVend) {
			return true;
		}
		// Recursive
		else {
			pathCheck(map, x + 1, y);
			pathCheck(map, x - 1, y);
			pathCheck(map, x, y + 1);
			pathCheck(map, x, y - 1);
		}
	}

public:

	static void Reader(std::vector<std::vector<colorVals>>& map, int &_width, int &_height, std::string& fileName) {

		unsigned char* data = stbi_load(fileName.c_str(), &width, &height, &n, 0);

		_width = width;
		_height = height;

		std::cout << "Width: " << width << " | Height: " << height << std::endl;

		// We put the map in a array now

		// If we have read some image data
		if (data != nullptr && width > 0 && height > 0) {
			if (n == 3) { // RGB

				// Will be only using this tbh so this can be optomized at a later date
				for (int i = 0; i < height; i++) {
					std::vector<colorVals> row;
					for (int j = 0; j < width; j++) {
						colorVals col;
						col.r = static_cast<int>(data[(i * width + j) * n]);
						col.g = static_cast<int>(data[(i * width + j) * n + 1]);
						col.b = static_cast<int>(data[(i * width + j) * n + 2]);
						col.a = 1;

						row.push_back(col);
					}
					map.push_back(row);
				}

			}
			else if (n == 4) { // RGBA

				for (int i = 0; i < height; i++) {
					std::vector<colorVals> row;
					for (int j = 0; j < width; j++) {
						colorVals col;
						col.r = static_cast<int>(data[(i * width + j) * n]);
						col.g = static_cast<int>(data[(i * width + j) * n + 1]);
						col.b = static_cast<int>(data[(i * width + j) * n + 2]);
						col.a = static_cast<int>(data[(i * width + j) * n + 3]);

						row.push_back(col);
					}
					map.push_back(row);
				}
			}
		}
		else {
			std::cout << "Read error\n";
		}

		// data now loaded into the 2d vector
		stbi_image_free(data);

	}

	static void printMap(std::vector<std::vector<colorVals>>& map) {
		for (int i = 0; i < map.size(); i++) {
			std::cout << "\n";
			for (int j = 0; j < map[0].size(); j++) {

				if (map[i][j] == CVwall) {
					std::cout << "0";
				}
				else if (map[i][j] == CVstart) {
					std::cout << "5";
				}
				else if (map[i][j] == CVend) {
					std::cout << "3";
				}
				else if (map[i][j] == CVpath) {
					std::cout << "7";
				}
				else {
					std::cout << "e";
				}
			}
		}
	}

	// Checks if there is a path from start to end
	static bool validMap(std::vector<std::vector<colorVals>>& map) {

		//bool valid = true;

		//// First we check that there if one start and finish and that all pixels are "textured"
		for (int i = 0; i < map.size(); i++) {
			for (int j = 0; j < map[0].size(); j++) {

				if (map[i][j] == CVstart) {
					// Then we start the recursive funtion

					return pathCheck(map, i, j);

				}

			}
		}
		return false;




	}


};

#endif // !MAPREADER_H
