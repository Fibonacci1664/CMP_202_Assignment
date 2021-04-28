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
// Also keep this a multiple of the colour palette size.
// If you zoom into an area of the mandlebrot you will need to increase the palette size for better resolution of image.
// Good rule of thumb is to keep this and the palette size the same number.
const int MAX_ITERATIONS = 256;

//struct constDimension
//{
//	const int WIDTH;
//	const int HEIGHT;
//
//	constDimension(int width, int height) : WIDTH(width), HEIGHT(height) {}
//};

class Mandlebrot
{
public:
	//Mandlebrot(int SIZE);
	Mandlebrot();
	~Mandlebrot();

	void initImageContainers(int SIZE);
	void write_tga(const char* filename, bool blur);
	void compute_mandelbrot_with_AMP(float left, float right, float top, float bottom, int yPosSt = 0, int yPosEnd = HEIGHT, bool blur = false);
	void applyBlur(uint32_t* inputImage);
	void runMultipleTimings();
	void setUpCSV();

	// GETTERS / SETTERS
	int getHeight();
	int getWidth();

private:

	/*uint32_t** image;
	uint32_t** blurImage;*/

	/*std::vector<std::vector<uint32_t>> image;
	std::vector<std::vector<uint32_t>> blurImage;

	int size;
	int WIDTH;
	int HEIGHT;*/

	ColourPalette palette;
};

/////////////////////////////////////////////////////////////////////////////////////////////