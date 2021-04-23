#pragma once
#include "ColourPalette.h"

/////////////////////////////////////////////////////////////////////////////////////////////

// The size of the image to generate.
const int WIDTH = 1024;
const int HEIGHT = 1024;

// The number of times to iterate before we assume that a point isn't in the Mandelbrot set.
// (You may need to turn this up if you zoom further into the set.)
// This number MUST be greater than colour palette size.
// as we index the colour palette based on the iteration, otherwise you will get oob errors.
// Alos keep this a multiple of the colour palette size.
// If you zoom into an area of the mandlebrot you will need to increase the palette size for better resolution of image.
// Good rule of thumb is to keep this and the palette size the same number.
const int MAX_ITERATIONS = 400;

class Mandlebrot
{
public:
	Mandlebrot();
	~Mandlebrot();

	void write_tga(const char* filename);
	void compute_mandelbrot_with_AMP(float left, float right, float top, float bottom, int yPosSt = 0, int yPosEnd = HEIGHT);
	void compute_mandelbrot_with_AMP_tiling(float left, float right, float top, float bottom, int yPosSt = 0, int yPosEnd = HEIGHT);
	void runMultipleTimingsNoExplicitTile();
	void runMultipleTimingsExplicitTile();
	void setUpCSV();

	// GETTERS / SETTERS
	int getHeight();
	int getWidth();

private:

	void blur();

	ColourPalette palette;
};

/////////////////////////////////////////////////////////////////////////////////////////////