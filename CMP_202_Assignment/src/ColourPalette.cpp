#include "ColourPalette.h"
#include <iostream>

/////////////////////////////////////////////////////////////////////////////////////////////

// CONSTRUCTOR / DESTRUCTOR
ColourPalette::ColourPalette()
{
	colourPaletteSize = 255;
}

ColourPalette::~ColourPalette()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

// FUNCTIONS

// Create a gradient of colours, useD to remove the colour banding in the mandlebrot.
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
	colour.red = red;
	colour.green = grn;
	colour.blue = blu;

	return colour;

	//return red | (grn << 8) | (blu << 16);
	//return (red << 16) | (grn << 8) | (blu);
}

/////////////////////////////////////////////////////////////////////////////////////////////

std::vector<Colour> ColourPalette::createPalette()
{
	//std::vector<unsigned int> palette;
	std::vector<Colour> palette;

	// A palette of 256 colours.
	for (double i = 0; i < colourPaletteSize; ++i)
	{
		Colour col = rgb(i / colourPaletteSize);
		palette.push_back(col);
	}

	// Find max / min for nomalising the palette to range 0 - 1
	/*int max = 0;
	int min = INT_MAX;

	for (int i = 0; i < palette.size(); ++i)
	{
		std::cout << "Palette value red at index " << i << ": " << palette[i].red << '\n';
		std::cout << "Palette value green at index " << i << ": " << palette[i].green << '\n';
		std::cout << "Palette value blue at index " << i << ": " << palette[i].blue << '\n';

		if (palette[i].red > max)
		{
			max = palette[i].red;
		}

		if (palette[i].red < min)
		{
			min = palette[i].red;
		}
	}

	std::cout << "Max palette value: " << max << '\n';
	std::cout << "Min palette value: " << min << '\n';*/

	// Normalise palette
	/*for (int i = 0; i < palette.size(); ++i)
	{
		float norm = (palette[i] - min) / (max - min);
		palette[i] = norm;
		std::cout << "Palette value at index " << i << ": " << palette[i] << '\n';
	}*/

	return palette;
}

/////////////////////////////////////////////////////////////////////////////////////////////

// GETTERS / SETTERS
double ColourPalette::getColourPalSize()
{
	return colourPaletteSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////