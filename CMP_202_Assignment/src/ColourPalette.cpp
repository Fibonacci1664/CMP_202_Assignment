#include "ColourPalette.h"
#include <iostream>

/////////////////////////////////////////////////////////////////////////////////////////////

// CONSTRUCTOR / DESTRUCTOR
ColourPalette::ColourPalette()
{
	colourPaletteSize = 256;
}

ColourPalette::~ColourPalette()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

// FUNCTIONS

// Create a gradient of colours, used to remove the colour banding in the mandlebrot.
// Input: ratio is between 0 to 1
// Output: rgb color
// Source: https://stackoverflow.com/questions/40629345/fill-array-dynamicly-with-gradient-color-c
Colour ColourPalette::rgb(double ratio)
{
	/*
	 * We want to normalize ratio so that the entire spectrum is divided
	 * in to x number of regions where each region is 256 units long
	*/

	int numOfRegions = 6;	// Same as the amount of colours we have below in the switch.
	int normalized = int(ratio * 256 * numOfRegions);

	// Find the distance to the start of the closest region
	int x = normalized % 256;

	int red = 0, grn = 0, blu = 0;

	switch (normalized / 256)
	{
	case 0: red = 255;      grn = x;        blu = 0;       break;//red
	case 1: red = 255 - x;  grn = 255;      blu = 0;       break;//yellow
	case 2: red = 0;        grn = 255;      blu = x;       break;//green
	case 3: red = 0;        grn = 255 - x;  blu = 255;     break;//cyan
	case 4: red = x;        grn = 0;        blu = 255;     break;//blue
	case 5: red = 255;      grn = 0;        blu = 255 - x; break;//magenta
	}

	Colour colour{};
	colour.colChannel_1 = red;
	colour.colChannel_2 = grn;
	colour.colChannel_3 = blu;

	return colour;
}

/////////////////////////////////////////////////////////////////////////////////////////////

std::vector<Colour> ColourPalette::createPalette()
{
	std::vector<Colour> palette;

	// A palette of 256 colours.
	for (double i = 0; i < colourPaletteSize; ++i)
	{
		Colour col = rgb(i / colourPaletteSize);
		palette.push_back(col);
	}

	return palette;
}

/////////////////////////////////////////////////////////////////////////////////////////////

// GETTERS / SETTERS
double ColourPalette::getColourPalSize()
{
	return colourPaletteSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////