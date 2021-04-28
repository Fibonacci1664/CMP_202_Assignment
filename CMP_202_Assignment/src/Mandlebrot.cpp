#include "Mandlebrot.h"
#include "ComplexNum.h"
#include "Filter.h"

/////////////////////////////////////////////////////////////////////////////////////////////

#include <amp.h>
#include <chrono>
#include <iostream>
#include <fstream>

/////////////////////////////////////////////////////////////////////////////////////////////

//  Good info.
//  http://warp.povusers.org/Mandelbrot/

/*
	Useful for getting coords to plot a specific mandlebrot fractle
	//https://sciencedemos.org.uk/mandelbrot.php

	Nice to play with
	//https://math.hws.edu/eck/js/mandelbrot/MB.html?ex=examples%2fspikey.xml
*/

// Import things we need from the standard library
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::cout;
using std::endl;
using std::ofstream;
using namespace concurrency;

/////////////////////////////////////////////////////////////////////////////////////////////

// GLOBALS
std::ofstream timings("timings.csv");

// The image data. Each pixel is represented as 0xRRGGBB.
uint32_t image[HEIGHT][WIDTH];
uint32_t blurImage[HEIGHT][WIDTH];

// Define the alias "the_clock" for the clock type we're going to use.
typedef std::chrono::steady_clock the_clock;

/////////////////////////////////////////////////////////////////////////////////////////////

// CONSTRUCTOR / DESTRUCTOR
Mandlebrot::Mandlebrot()
{

}

Mandlebrot::~Mandlebrot()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

// FUNCTIONS

// Write the image to a TGA file with the given name.
// Format specification: http://www.gamers.org/dEngine/quake3/TGA.txt
void Mandlebrot::write_tga(const char* filename, bool blur)
{
	ofstream outfile(filename, ofstream::binary);

	uint8_t header[18] = {
		0, // no image ID
		0, // no colour map
		2, // uncompressed 24-bit image
		0, 0, 0, 0, 0, // empty colour map specification
		0, 0, // X origin
		0, 0, // Y origin
		WIDTH & 0xFF, (WIDTH >> 8) & 0xFF, // width
		HEIGHT & 0xFF, (HEIGHT >> 8) & 0xFF, // height
		24, // bits per pixel
		0, // image descriptor
	};
	outfile.write((const char*)header, 18);

	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			if (blur)
			{
				// Write the final blurred image to file.
				uint8_t pixel[3] = {
						 blurImage[y][x] & 0xFF,			// blue channel
						(blurImage[y][x] >> 8) & 0xFF,		// green channel
						(blurImage[y][x] >> 16) & 0xFF,		// red channel
				};
				outfile.write((const char*)pixel, 3);
			}
			else
			{
				// Write the original image to file.
				uint8_t pixel[3] = {
						 image[y][x] & 0xFF,			// blue channel
						(image[y][x] >> 8) & 0xFF,		// green channel
						(image[y][x] >> 16) & 0xFF,		// red channel
				};
				outfile.write((const char*)pixel, 3);
			}
		}
	}

	outfile.close();
	if (!outfile)
	{
		// An error has occurred at some point since we opened the file.
		cout << "Error writing to " << filename << endl;
		exit(1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

// Render the Mandelbrot set into the image array.
// The parameters specify the region on the complex plane to plot.
// This will render the mandlebrot using C++ AMP without tiling explicitly.
void Mandlebrot::compute_mandelbrot_with_AMP(float left, float right, float top, float bottom, int yPosSt, int yPosEnd)
{
	//std::vector<unsigned int> colPalette = palette.createPalette();
	std::vector<Colour> colPalette = palette.createPalette();

	// Create a pointer that points to the same location as the first index of image data 2d array.
	uint32_t* pImage = &(image[0][0]);

	// Create an array view copying in the data of pImage, we need this as the GPU can only work with array_view and NOT arrays.
	// We could have created an extent object and passed that as the second param, in this case we have hard coded the value 2.
	array_view<uint32_t, 2> arrView(HEIGHT, WIDTH, pImage);
	//array_view<unsigned int, 1> paletteArrView(colPalette.size(), colPalette);
	array_view<Colour, 1> paletteArrView(colPalette.size(), colPalette);
	arrView.discard_data();

	try
	{
		parallel_for_each(arrView.extent, [=](concurrency::index<2> idx) restrict(amp)
			{
				/* compute Mandelbrot here i.e. Mandelbrot kernel/shader */

				// USE THREAD ID / INDEX TO MAP INTO THE COMPLEX PLANE
				/*
				 Here we are setting the value of x and y to the value contained at the
				 concurrency::index object idx index position at positions [0] and [1].

				 This is representative of 1 pixel.
				 We create 1 thread per pixel and so this happen concurrently for ALL
				 pixels in the image size, and so the mandlebrot is calculated almost instantly.
				*/
				int y = idx[0];		// The x component of the pixel.
				int x = idx[1];		// The y component of the pixel.

				// Work out the point in the complex plane that
				// corresponds to this pixel in the output image.
				ComplexNum c;
				c.x = left + (x * (right - left) / WIDTH);
				c.y = top + (y * (bottom - top) / HEIGHT);

				// Start off z at (0, 0).
				ComplexNum z;
				z.x = 0.0f;
				z.y = 0.0f;

				// Iterate z = z^2 + c until z moves more than 2 units
				// away from (0, 0), or we've iterated too many times.
				int iterations = 0;
				float escapeRadius = 2.0f;
				

				while (c_abs(z) < escapeRadius && iterations < MAX_ITERATIONS)
				{
					z = c_mul(z, z);
					z = c_add(z, c);

					++iterations;
				}

				if (iterations == MAX_ITERATIONS - 1)
				{
					// z didn't escape from the circle.
					// This point IS in the Mandelbrot set.
					arrView[idx] = 0x000000; // black
				}
				else
				{
					/*int red = paletteArrView[iterations] * 255;
					int green = paletteArrView[iterations] * 255;
					int blue = paletteArrView[iterations] * 255;
					int col = (red << 16) | (green << 8) | (blue);*/

					//arrView[idx] = (iterations) & 0x0000FF;
					//int col = paletteArrView[iterations];
					//float col = paletteArrView[iterations] * 255.0f;

					

					//int col = (red << 16) | (green << 8) | (blue);
					//int col = (blue << 16) | (green << 8) | (red);

					int col = 0;

					if (iterations < 43 || (iterations > 128 && iterations < 171))
					{
						col = paletteArrView[iterations].green;
					}
					else if ((iterations > 42 && iterations < 86) || (iterations > 170 && iterations < 214))
					{
						col = paletteArrView[iterations].red;
					}
					else if ((iterations > 85 && iterations < 129) || (iterations > 213 && iterations < 256))
					{
						col = paletteArrView[iterations].blue;
					}

					/*int red = paletteArrView[iterations].red;
					int green = paletteArrView[iterations].green;
					int blue = paletteArrView[iterations].blue;
					int finalCol = red | green | blue;*/

					//arrView[idx] = col / 255;
					//arrView[idx] = paletteArrView[iterations].green;
					arrView[idx] = col;
				}
			});

		arrView.synchronize();
	}
	catch (const concurrency::runtime_exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error with mandlebrot without explicit tiling", MB_ICONERROR);
	}

	// Write the original image to file before further modifying it.
	write_tga("original_image.tga", false);

	// Find max / min for nomalising the palette to range 0 - 1
	/*int max = 0;
	int min = INT_MAX;

	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			std::cout << "Pixel coord: " << "(" << x << ", " << y << ")" << ", Pixel colour value: " << image[x][y] << '\n';

			if (image[x][y] > max)
			{
				max = image[x][y];
			}

			if (image[x][y] < min)
			{
				min = image[x][y];
			}
		}
	}

	std::cout << "Max image pixel value: " << max << '\n';
	std::cout << "Min image pixel value: " << min << '\n';*/

	//// ################################# END MANDLEBROT AND START BLUR #################################

	//// Pointer to a new empty container ready to store the blurred mandlebrot image.
	uint32_t* pImageOut = &(blurImage[0][0]);

	// For blur
	Filter wrapper;
	// The original source image file has now been populated with the mandlebrot fractle
	array_view<uint32_t, 2> arrViewOut(HEIGHT, WIDTH, pImageOut);

	// PUT THIS IN A TRY CATCH ALSO!
	// HORIZONTAL BLUR
	parallel_for_each(arrView.extent.tile<WIDTH, 1>(), [=](tiled_index<WIDTH, 1> t_idx) restrict(amp)
		{
			index<2> idx = t_idx.global;

			tile_static float horizontal_points[WIDTH];
			horizontal_points[idx[0]] = arrView[idx];

			t_idx.barrier.wait();

			 /*KERNEL_SIZE is the size of the filter matrix, (7x7) or (7x1)
			 Whatever pixel we're at minus 3.*/
			int textureLocationX = idx[0] - (KERNEL_SIZE / 2);

			float pixelBlur = 0.0;

			for (int i = 0; i < KERNEL_SIZE; ++i)
			{
				pixelBlur += horizontal_points[textureLocationX + i] * wrapper.filter[i];
			}

			t_idx.barrier.wait();
			arrViewOut[idx] = pixelBlur;
		});

	arrViewOut.synchronize();

	//Swap and then process the vertical strips next
	std::swap(arrView, arrViewOut);

	// PUT THIS IN A TRY CATCH ALSO!
	// VERTICAL BLUR
	// Our arrViewIn is now the half blurred image, only blurred in horizontal.
	parallel_for_each(arrView.extent.tile<1, HEIGHT>(), [=](tiled_index<1, HEIGHT> t_idx) restrict(amp)
		{
			index<2> idx = t_idx.global;

			tile_static float vertical_points[HEIGHT];
			vertical_points[idx[1]] = arrView[idx];

			t_idx.barrier.wait();

			int textureLocationY = idx[1] - (KERNEL_SIZE / 2);

			float pixelBlur = 0.0;

			for (int i = 0; i < KERNEL_SIZE; ++i)
			{
				pixelBlur += vertical_points[textureLocationY + i] * wrapper.filter[i];
			}

			t_idx.barrier.wait();
			arrViewOut[idx] = pixelBlur;
		});

	//The final sync which should now sync the fully blurred image back to the CPU.
	arrViewOut.synchronize();

	// Write the final blurred image to file.
	write_tga("blurred_image.tga", true);
}

/////////////////////////////////////////////////////////////////////////////////////////////

// Render the Mandelbrot set into the image array.
// The parameters specify the region on the complex plane to plot.
// This will render the mandlebrot using C++ AMP without tiling explicitly.
void Mandlebrot::compute_mandelbrot_with_AMP_tiling(float left, float right, float top, float bottom, int yPosSt, int yPosEnd)
{
	//std::vector<unsigned int> colPalette = palette.createPalette();

	// Create a pointer that points to the same location as the first index of image data 2d array.
	uint32_t* pImage = &(image[0][0]);

	// Create an array view copying in the data of pImage, we need this as the GPU can only work with array_view and NOT arrays.
	// We could have created an extent object and passed that as the second param, in this case we have hard coded the value 2.
	array_view<uint32_t, 2> arrView(HEIGHT, WIDTH, pImage);
	//array_view<unsigned int, 1> paletteArrView(colPalette.size(), colPalette);
	arrView.discard_data();

	try
	{
		parallel_for_each(arrView.extent, [=](concurrency::index<2> idx) restrict(amp)
			{
				/* compute Mandelbrot here i.e. Mandelbrot kernel/shader */

				// USE THREAD ID / INDEX TO MAP INTO THE COMPLEX PLANE
				/*
				 Here we are setting the value of x and y to the value contained at the
				 concurrency::index object idx index position at positions [0] and [1].

				 This is representative of 1 pixel.
				 We create 1 thread per pixel and so this happen concurrently for ALL
				 pixels in the image size, and so the mandlebrot is calculated almost instantly.
				*/
				int y = idx[0];
				int x = idx[1];

				// Work out the point in the complex plane that
				// corresponds to this pixel in the output image.
				ComplexNum c;
				c.x = left + (x * (right - left) / WIDTH);
				c.y = top + (y * (bottom - top) / HEIGHT);

				// Start off z at (0, 0).
				ComplexNum z;
				z.x = 0.0f;
				z.y = 0.0f;

				// Iterate z = z^2 + c until z moves more than 2 units
				// away from (0, 0), or we've iterated too many times.
				int iterations = 0;
				float escapeRadius = 2.0f;

				while (c_abs(z) < escapeRadius && iterations < MAX_ITERATIONS)
				{
					//z = (z * z) + c;
					z = c_mul(z, z);
					z = c_add(z, c);

					++iterations;
				}

				if (iterations == MAX_ITERATIONS)
				{
					// z didn't escape from the circle.
					// This point is in the Mandelbrot set.
					arrView[idx] = 0x000000; // black
				}
				else
				{
					//float smoothedIndex = iterations - std::log(std::log(std::log(std::abs(z.real() * z.real() + z.imag() * z.imag())))) / std::log(2.0f);
					//unsigned int col = paletteArrView[iterations];
					//arrView[idx] = col;// (col * iterations) / MAX_ITERATIONS;
				}
			});

		arrView.synchronize();
	}
	catch (const concurrency::runtime_exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error with mandlebrot without explicit tiling", MB_ICONERROR);
	}

	// ################################# END MANDLEBROT AND START BLUR #################################

	// Pointer to a new empty container ready to store the blurred mandlebrot image.
	uint32_t* pImageOut = &(blurImage[0][0]);

	// For blur
	Filter wrapper;
	// The original source image file has now been populated with the mandlebrot fractle
	//array_view<uint32_t, 2> arrViewIn(HEIGHT, WIDTH, pImageIn);
	array_view<uint32_t, 2> arrViewOut(HEIGHT, WIDTH, pImageOut);

	// PUT THIS IN A TRY CATCH ALSO!
	// HORIZONTAL BLUR
	parallel_for_each(arrView.extent.tile<WIDTH, 1>(), [=](tiled_index<WIDTH, 1> t_idx) restrict(amp)
		{
			index<2> idx = t_idx.global;

			tile_static float horizontal_points[WIDTH];
			horizontal_points[idx[0]] = arrView[idx];

			t_idx.barrier.wait();

			// KERNEL_SIZE is the size of the filter matrix, (7x7) or (7x1)
			// Whatever pixel we're at minus 3.
			int textureLocationX = idx[0] - (KERNEL_SIZE / 2);

			float pixelBlur = 0.0;

			for (int i = 0; i < KERNEL_SIZE; ++i)
			{
				pixelBlur += horizontal_points[textureLocationX + i] * wrapper.filter[i];
				t_idx.barrier.wait();
				arrViewOut[idx] = pixelBlur;
			}
		});

	arrViewOut.synchronize();

	//// Swap and then process the vertical strips next
	std::swap(arrView, arrViewOut);

	// PUT THIS IN A TRY CATCH ALSO!
	// VERTICAL BLUR
	// Our arrViewIn is now the half blurred image, only blurred in horizontal.
	parallel_for_each(arrView.extent.tile<1, HEIGHT>(), [=](tiled_index<1, HEIGHT> t_idx) restrict(amp)
		{
			index<2> idx = t_idx.global;

			tile_static float vertical_points[HEIGHT];
			vertical_points[idx[1]] = arrView[idx];

			t_idx.barrier.wait();

			int textureLocationY = idx[1] - (KERNEL_SIZE / 2);

			float pixelBlur = 0.0;

			for (int i = 0; i < KERNEL_SIZE; ++i)
			{
				pixelBlur += vertical_points[textureLocationY + i] * wrapper.filter[i];
				t_idx.barrier.wait();
				arrViewOut[idx] = pixelBlur;
			}
		});

	//The final sync which should now sync the fully blurred image back to the CPU.
	arrViewOut.synchronize();
}

/////////////////////////////////////////////////////////////////////////////////////////////

//Coordinates
//Left : -0.096649913003
//Right : -0.094281573354
//Top : 0.833947325529
//Bottom : 0.834931197758
void Mandlebrot::runMultipleTimingsNoExplicitTile()
{
	int counter = 0;
	std::cout << "\n#### NO EXPLICIT TILING ###\n";
	timings << "No Explicit Tile,";		// Output to CSV.

	while (counter < 1)
	{
		// Start timing.
		the_clock::time_point start = the_clock::now();

		// This shows the whole set.	
		compute_mandelbrot_with_AMP(-2.0, 1.0, 1.125, -1.125, 16, 498);
		//compute_mandelbrot_with_AMP(-0.096649913003, -0.094281573354, 0.833947325529, 0.834931197758, 0, HEIGHT);

		// Stop timing.
		the_clock::time_point end = the_clock::now();

		// Compute the difference between the two times in milliseconds.
		auto time_taken = duration_cast<milliseconds>(end - start).count();
		cout << "Computing the Mandelbrot set took with no explicit Blur tile: " << time_taken << " ms." << endl;

		timings << time_taken << ",";		// Output to CSV.

		++counter;
	}

	std::cout << '\n';
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Mandlebrot::runMultipleTimingsExplicitTile()
{
	int counter = 0;
	std::cout << "#### EXPLICIT TILING ###\n";
	timings << "\nExplicit Tile,";

	while (counter < 1)
	{
		// Start timing
		the_clock::time_point start = the_clock::now();

		// This shows the whole set.	
		compute_mandelbrot_with_AMP_tiling(-2.0, 1.0, 1.125, -1.125, 16, 498);

		// Stop timing
		the_clock::time_point end = the_clock::now();

		// Compute the difference between the two times in milliseconds
		auto time_taken = duration_cast<milliseconds>(end - start).count();
		cout << "Computing the Mandelbrot set took with explicit Blur tile: " << time_taken << " ms." << endl;

		timings << time_taken << ",";

		++counter;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Mandlebrot::setUpCSV()
{
	for (int i = 0; i < 25; ++i)
	{
		timings << "," << "Time " << (i + 1);
	}

	timings << '\n';
}

/////////////////////////////////////////////////////////////////////////////////////////////

void Mandlebrot::blur()
{
	//parallel_for_each(a.extent.tile<WIDTH, 1>(),[=](concurrency::tiled_index<WIDTH, 1> t_idx) restrict(amp)
	//	{
	//		index<2> idx = t_idx.global;

	//		tile_static float_4 horizontal_points[WIDTH];

	//		horizontal_points[idx[0]] = a[idx];

	//		t_idx.barrier.wait();

	//		int texturelocationx = idx[0] - (KERNEL_SIZE / 2);

	//		float_4 pixelBlur = 0.0;

	//		for (int i = 0; i < KERNEL_SIZE; i++)
	//		{
	//			pixelBlur += horizontal_points[texturelocationx + i] * x.filter1D[i];
	//		}
	//			
	//		t_idx.barrier.wait();
	//		b[idx] = pixelBlur;
	//	});

	//b.synchronize();

	////swap and then process the vertical strips next 
	//std::swap(a, b)
}

/////////////////////////////////////////////////////////////////////////////////////////////

// GETTERS / SETTERS
int Mandlebrot::getHeight()
{
	return HEIGHT;
}

/////////////////////////////////////////////////////////////////////////////////////////////

int Mandlebrot::getWidth()
{
	return WIDTH;
}

/////////////////////////////////////////////////////////////////////////////////////////////