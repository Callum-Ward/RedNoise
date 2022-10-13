#include "Colour.h"
#include <utility>
#include <math.h>

Colour::Colour() = default;
Colour::Colour(int r, int g, int b) : red(r), green(g), blue(b) {}
Colour::Colour(std::string n, int r, int g, int b) :
		name(std::move(n)),
		red(r), green(g), blue(b) {}

Colour::Colour(std::string n, float r, float g, float b) :
		name(std::move(n)),
		red(round(r*255)), green(round(g*255)), blue(round(b*255)) {}

std::ostream &operator<<(std::ostream &os, const Colour &colour) {
	os << colour.name << " ["
	   << colour.red << ", "
	   << colour.green << ", "
	   << colour.blue << "]";
	return os;
}
