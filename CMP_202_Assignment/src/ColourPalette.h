#pragma once
#include <vector>

/////////////////////////////////////////////////////////////////////////////////////////////

struct Colour
{
	float colChannel_1;
	float colChannel_2;
	float colChannle_3;
};

/////////////////////////////////////////////////////////////////////////////////////////////

class ColourPalette
{
public:
	ColourPalette();
	~ColourPalette();

	Colour rgb(double ratio);
	std::vector<Colour> createPalette();
	double getColourPalSize();

	double colourPaletteSize;
};

/////////////////////////////////////////////////////////////////////////////////////////////