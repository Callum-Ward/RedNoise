#pragma once

#include <iostream>

struct Colour {
	std::string name;
	int red{};
	int green{};
	int blue{};
	int isTexture;
	std::vector<uint32_t> rowTexture;
	Colour();
	Colour(int r, int g, int b);
	Colour(std::string n,float r, float g, float b);
	Colour(std::string n, int r, int g, int b);
};

std::ostream &operator<<(std::ostream &os, const Colour &colour);
