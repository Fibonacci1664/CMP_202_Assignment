#pragma once
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////////

struct Colour
{
	float red;
	float green;
	float blue;
};

class ColourPalette
{
public:
	ColourPalette();
	~ColourPalette();

	Colour rgb(double ratio);
	//std::vector<float> createPalette();
	std::vector<Colour> createPalette();

	double getColourPalSize();

	double colourPaletteSize;
};

/////////////////////////////////////////////////////////////////////////////////////////////