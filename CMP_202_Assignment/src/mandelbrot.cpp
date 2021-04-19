// Mandelbrot set example
// Adam Sampson <a.sampson@abertay.ac.uk>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <complex>
#include <fstream>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <amp.h>
#include <amp_math.h>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Import things we need from the standard library
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::complex;
using std::cout;
using std::endl;
using std::ofstream;
using namespace concurrency;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// GLOBALS
std::ofstream timings("timings.csv");

// For blur
static const float filter[7][7] = {
0.000904706,	0.003157733,	0.00668492,		0.008583607,	0.00668492,		0.003157733,	0.000904706,
0.003157733,	0.01102157,		0.023332663,	0.029959733,	0.023332663,	0.01102157,		0.003157733,
0.00668492,		0.023332663,	0.049395249,	0.063424755,	0.049395249,	0.023332663,	0.00668492,
0.008583607,	0.029959733,	0.063424755,	0.081438997,	0.063424755,	0.029959733,	0.008583607,
0.00668492,		0.023332663,	0.049395249,	0.063424755,	0.049395249,	0.023332663,	0.00668492,
0.003157733,	0.01102157,		0.023332663,	0.029959733,	0.023332663,	0.01102157,		0.003157733,
0.000904706,	0.003157733,	0.00668492,		0.008583607,	0.00668492,		0.003157733,	.000904706
};

// Get all accelerators available to us and store in a vector so we can extract details.
std::vector<accelerator> accls = accelerator::get_all();

// The size of the image to generate.
const int WIDTH = 1920;
const int HEIGHT = 1024;

// The image data. Each pixel is represented as 0xRRGGBB.
uint32_t image[HEIGHT][WIDTH];

// Define the alias "the_clock" for the clock type we're going to use.
typedef std::chrono::steady_clock the_clock;

// The number of times to iterate before we assume that a point isn't in the Mandelbrot set.
// (You may need to turn this up if you zoom further into the set.)
// This number MUST be greater than colour palette size.
// as we index the colour palette based on the iteration, otherwise you will get oob errors.
// Keep this a multiple of the colour palette size.
// If you zoom into an area of the mandlebrot you will need to increase the palette size for better resolution of image.
// Good rule of thumb is to keep this and the palette size the same number.
const int MAX_ITERATIONS = 400;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Using our own Complex number structure and definitions as the Complex type is not available in the Concurrency namespace.
struct Complex1
{
	float x;
	float y;
};

// Struct helper function.
Complex1 c_add(Complex1 c1, Complex1 c2) restrict(cpu, amp) // restrict keyword - able to execute this function on the GPU and CPU
{
	Complex1 tmp;
	float a = c1.x;
	float b = c1.y;
	float c = c2.x;
	float d = c2.y;
	tmp.x = a + c;
	tmp.y = b + d;

	return tmp;
} // c_add

// Struct helper function.
float c_abs(Complex1 c) restrict(cpu, amp)
{
	return concurrency::fast_math::sqrt(c.x * c.x + c.y * c.y);
} // c_abs

// Struct helper function.
Complex1 c_mul(Complex1 c1, Complex1 c2) restrict(cpu, amp)
{
	Complex1 tmp;
	float a = c1.x;
	float b = c1.y;
	float c = c2.x;
	float d = c2.y;
	tmp.x = a * c - b * d;
	tmp.y = b * c + a * d;

	return tmp;
} // c_mul

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Create a gradient band of colours.
// Input: ratio is between 0 to 1
// Output: rgb color

// Source: https://stackoverflow.com/questions/40629345/fill-array-dynamicly-with-gradient-color-c
unsigned int rgb(double ratio)
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

	return red | (grn << 8) | (blu << 16);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Create a colour palette using the above rgb function.
std::vector<unsigned int> createPalette()
{
	double colourPaletteSize = 400;
	std::vector<unsigned int> palette;

	// A palette of 360 colours.
	for (double i = 0; i < colourPaletteSize; ++i)
	{
		unsigned int col = rgb(i / colourPaletteSize);
		palette.push_back(col);
	}

	return palette;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Write the image to a TGA file with the given name.
// Format specification: http://www.gamers.org/dEngine/quake3/TGA.txt
void write_tga(const char *filename)
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
	outfile.write((const char *)header, 18);

	for (int y = 0; y < HEIGHT; ++y)
	{
		for (int x = 0; x < WIDTH; ++x)
		{
			uint8_t pixel[3] = {
				image[y][x] & 0xFF, // blue channel
				(image[y][x] >> 8) & 0xFF, // green channel
				(image[y][x] >> 16) & 0xFF, // red channel
			};
			outfile.write((const char *)pixel, 3);
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Render the Mandelbrot set into the image array.
// The parameters specify the region on the complex plane to plot.
// This will render the mandlebrot using C++ AMP without tiling explicitly.
void compute_mandelbrot_with_AMP(float left, float right, float top, float bottom, int yPosSt = 0, int yPosEnd = HEIGHT)
{
	std::vector<unsigned int> palette = createPalette();

	// Create a pointer that points to the same location as the first index of image data 2d array.
	uint32_t* pImage = &(image[0][0]);

	// Create an array view copying in the data of pImage, we need this as the GPU can only work with array_view and NOT arrays.
	// We could have created an extent object and passed that as the second param, in this case we have hard coded the value 2.
	array_view<uint32_t, 2> arrView(HEIGHT, WIDTH, pImage);
	array_view<unsigned int, 1> paletteArrView(palette.size(), palette);
	arrView.discard_data();

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
			Complex1 c;
			c.x = left + (x * (right - left) / WIDTH);
			c.y = top + (y * (bottom - top) / HEIGHT);

			// Start off z at (0, 0).
			Complex1 z;
			z.x = 0.0f;
			z.y = 0.0f;

			//Complex1 zEnd{};

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
				unsigned int col = paletteArrView[iterations];
				arrView[idx] = col;// (col * iterations) / MAX_ITERATIONS;
			}
		});

	arrView.synchronize();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Render the Mandelbrot set into the image array.
// The parameters specify the region on the complex plane to plot.
// This will render the mandlebrot using C++ AMP without tiling explicitly.
void compute_mandelbrot_with_AMP_tiling(float left, float right, float top, float bottom, int yPosSt = 0, int yPosEnd = HEIGHT)
{
	const int TS = 8;	// This will be how many threads run per tile, if 1D = 8, if 2D = 8x8 = 64, 3D 8x8x8 = 512 threads / tile

	std::vector<unsigned int> palette = createPalette();

	uint32_t* pImage = &(image[0][0]);

	array_view<uint32_t, 2> arrView(HEIGHT, WIDTH, pImage);
	array_view<unsigned int, 1> paletteArrView(palette.size(), palette);
	arrView.discard_data();

	parallel_for_each(arrView.extent.tile<TS, TS>(), [=](tiled_index<TS, TS> t_idx) restrict(amp)
		{
			/* compute Mandelbrot here i.e. Mandelbrot kernel/shader */

			int y = t_idx.global[0];		// The y component of the GPU's global coord system, ROW
			int x = t_idx.global[1];		// The x component of the GPU's global coord system, COL
			
			// So for 1920x1024, that gives us 1920/8 = 240 (x component) and 1024/8 = 128 (y component)
			// So we have 240x128 = 30720 tiles with each tile is size 8x8 (8 threads by 8 threads), therefore each tile having 64 threads.
			// 30,720 tiles x 64 threads = 1,966,080 threads
			// Image dimensions HEIGHT(1920), WIDTH(1024)
			// 1920x1024 = 1,966,080 pixels
			// Therefore 1 thread per pixel. Tiled into groups of 64 threads.
			// Here it seems I must have to pass the amount of tiles in x and y and NOT the TS
			// But why does the lectures have TS in the Toy example?
									 // Y	 X
									 //ROW  COL
			tile_static int tileValues[8][8];
			
			// Missing something here?

			// This is saying, set whatever thread we happen to be in LOCALLY in our tile,
			// set it to the same thread that corrosponds GLOBALLY.
			tileValues[t_idx.local[0]][t_idx.local[1]] = arrView[t_idx.global];	

			//tileValues[t_idx.local[0]][t_idx.local[1]] = arrView[y][x];

			t_idx.barrier.wait();

			// USE THREAD ID / INDEX TO MAP INTO THE COMPLEX PLANE
			/*
			 Here we are setting the value of x and y to the value contained at the
			 concurrency::index object idx index position at positions [0] and [1].

			 This is representative of 1 pixel.
			 We create 1 thread per pixel and so this happen concurrently for ALL
			 pixels in the image size, and so the mandlebrot is calculated almost instantly.
			*/
			/*int y = idx[0];
			int x = idx[1];*/

			// Work out the point in the complex plane that
			// corresponds to this pixel in the output image.
			Complex1 c{};
			c.x = left + (x * (right - left) / WIDTH);
			c.y = top + (y * (bottom - top) / HEIGHT);

			// Start off z at (0, 0).
			Complex1 z{};
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
				//tileValues[y][x] = 0x000000; // black
				arrView[t_idx] = 0x000000; // black
				//arrView[t_idx] = tileValues[y][x];
			}
			else
			{
				//float smoothedIndex = iterations - std::log(std::log(std::log(std::abs(z.real() * z.real() + z.imag() * z.imag())))) / std::log(2.0f);
				unsigned int col = paletteArrView[iterations];
				//tileValues[y][x] = col;

				//arrView[t_idx] = tileValues[y][x];

				arrView[t_idx] = col;// (col * iterations) / MAX_ITERATIONS;
			}
		});

	arrView.synchronize();

	//// HORIZONTAL BLUR
	//parallel_for_each(arrView.extent.tile<WIDTH, 1>(), [=](tiled_index<WIDTH, 1> t_idx) restrict(amp)
	//	{
	//		index<2> idx = t_idx.global;

	//		tile_static float horizontal_points[WIDTH];

	//		horizontal_points[idx[0]] = arrView[idx];

	//		t_idx.barrier.wait();

	//		int texturelocationX = idx[0] - (KERNEL_SIZE / 2);

	//		float pixelBlur = 0.0;

	//		for (int i = 0; i < KERNEL_SIZE; i++)
	//		{
	//			pixelBlur += horizontal_points[texturelocationX + i] * x.filter1D[i];
	//		}

	//		t_idx.barrier.wait();
	//		b[idx] = pixelBlur;
	//	});

	//b.synchronize();

	////swap and then process the vertical strips next 
	//std::swap(a, b)
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void runMultipleTimingsNoExplicitTile()
{
	int counter = 0;
	std::cout << "\n#### NO EXPLICIT TILING ###\n";
	timings << "No Explicit Tile,";		// Output to CSV.

	while (counter < 25)
	{
		// Start timing.
		the_clock::time_point start = the_clock::now();

		// This shows the whole set.	
		compute_mandelbrot_with_AMP(-2.0, 1.0, 1.125, -1.125, 16, 498);
		
		// Stop timing.
		the_clock::time_point end = the_clock::now();

		// Compute the difference between the two times in milliseconds.
		auto time_taken = duration_cast<milliseconds>(end - start).count();
		cout << "Computing the Mandelbrot set took: " << time_taken << " ms." << endl;

		timings << time_taken << ",";		// Output to CSV.

		++counter;
	}

	std::cout << '\n';
	//timings << '\n';
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void runMultipleTimingsExplicitTile()
{
	int counter = 0;
	std::cout << "#### EXPLICIT TILING ###\n";
	timings << "\nExplicit Tile,";

	while (counter < 25)
	{
		// Start timing
		the_clock::time_point start = the_clock::now();

		// This shows the whole set.	
		compute_mandelbrot_with_AMP_tiling(-2.0, 1.0, 1.125, -1.125, 16, 498);

		// Stop timing
		the_clock::time_point end = the_clock::now();

		// Compute the difference between the two times in milliseconds
		auto time_taken = duration_cast<milliseconds>(end - start).count();
		cout << "Computing the Mandelbrot set took: " << time_taken << " ms." << endl;

		timings << time_taken << ",";

		++counter;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void report_accelerator(const accelerator a)
{
	const std::wstring bs[2] = { L"false", L"true" };

	std::wcout << ": " << a.description << " "
		<< endl << "		device_path                       = " << a.device_path
		<< endl << "		version = " << (a.version >> 16) << '.' << (a.version & 0xFFFF)
		<< endl << "		dedicated_memory                  = " << std::setprecision(4) << float(a.dedicated_memory) / (1024.0f * 1024.0f) << " Mb"
		<< endl << "		has_display                       = " << bs[a.has_display]
		<< endl << "		supports debug                    = " << bs[a.is_debug]
		<< endl << "		is_emulated                       = " << bs[a.is_emulated]
		<< endl << "		supports_double_precision         = " << bs[a.supports_double_precision]
		<< endl << "		supports_limited_double_precision = " << bs[a.supports_limited_double_precision]
		<< endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// List and select the accelerator to use
void list_accelerators()
{
	// iterates over all accelerators and print characteristics
	for (int i = 0; i < accls.size(); i++)
	{
		accelerator a = accls[i];
		report_accelerator(a);
	}

	//http://www.danielmoth.com/Blog/Running-C-AMP-Kernels-On-The-CPU.aspx

	////Use default accelerator
	//accelerator a = accelerator(accelerator::default_accelerator);
	//accelerator::set_default(a.device_path);
	//std::wcout << " Using default acc = " << a.description << endl;

	//https://docs.microsoft.com/en-gb/windows/win32/direct3darticles/directx-warp?redirectedfrom=MSDN

	/*
	Recommended Application Types for WARP
	All applications that can use Direct3D can use WARP. This includes the following types of applications:

	Casual Games
	Existing Non-Gaming Applications
	Advanced Rendering Games
	Other Applications
	*/

	//Use Direct 3D WARP accelerator, this is an emulator, it is software on the CPU that emulates GPU hardware * This does not support double precision *
	/*accelerator a = accelerator(accelerator::direct3d_warp);
	accelerator::set_default(a.device_path);
	std::wcout << " Using Direct3D WARP acc = " << a.description << endl;*/

	//http://www.danielmoth.com/Blog/concurrencyaccelerator.aspx

	//Use Direct 3D REF accelerator, this is an emulator, it is software on the CPU that emulates GPU hardware* This is extremely slow and is only intended for debugging *
	/*
	represents the reference rasterizer emulator that simulates a direct3d device on the CPU (in a very slow manner).
	This emulator is available on systems with Visual Studio installed and is useful for debugging
	*/
	/*accelerator a = accelerator(accelerator::direct3d_ref);
	accelerator::set_default(a.device_path);
	std::wcout << " Using Direct3D REF acc = " << a.description << endl;*/

	/*
	Some great info here about different accelerators:
	//https://www.microsoftpressstore.com/articles/article.aspx?p=2201645
	*/

	//Use CPU accelerator - Does NOT support Debug mode.
	/*
	You cannot execute code on the CPU accelerator.
	ref: //https://docs.microsoft.com/en-us/cpp/parallel/amp/using-accelerator-and-accelerator-view-objects?view=msvc-160
	*/
	/*accelerator a = accelerator(accelerator::cpu_accelerator);
	accelerator::set_default(a.device_path);
	std::wcout << " Using CPU acc = " << a.description << endl;*/
} // list_accelerators

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// query if AMP accelerator exists on hardware
void query_AMP_support()
{
	std::vector<accelerator> accls = accelerator::get_all();

	if (accls.empty())
	{
		cout << "No accelerators found that are compatible with C++ AMP" << std::endl;
	}
	else
	{
		cout << "Accelerators found that are compatible with C++ AMP" << std::endl;
		list_accelerators();
	}
} // query_AMP_support

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void blur()
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////// IMPORTANT INFO RELATED TO THE WARM UP CALL BELOW /////////////////////////////

	/*
	 * Run these to cache the AMP runtime initialisation.
	 * This way the first timing using the GPU is not significantly slower than the rest.
	 * Otherwise the slower first time is caused by C++ AMP using JIT (Just in Time) lazy initialisation.
	 */

	 // ALWAYS RECORD TIMINGS AFTER THE AMP RUNTIME INTIALISATION HAS BEEN CACHED.

	 /*
	  * Make sure that you have called acceleartor_view.wait() as the parallel_for_each is
	  * an asynchronous call and so we need to make sure that ALL of the pervious uses during
	  * warmup calls are finihsed. Otherwise we may end up including some of the warmups in the timings.
	  */

///////////////////////////// IMPORTANT INFO RELATED TO THE WARM UP CALLS BELOW /////////////////////////////
void AMPWarmUp()
{
	compute_mandelbrot_with_AMP(-2.0, 1.0, 1.125, -1.125, 0, HEIGHT);
	compute_mandelbrot_with_AMP_tiling(-2.0, 1.0, 1.125, -1.125, 0, HEIGHT);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void setUpCSV()
{
	for (int i = 0; i < 25; ++i)
	{
		timings << "," << "Time " << (i + 1);
	}

	timings << '\n';
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	cout << "Please wait..." << endl;

	query_AMP_support();

	std::cout << "\n##############################################################\n";

	accelerator_view accelView = accelerator().default_view;
	accelerator::set_default(accelView.accelerator.default_accelerator);
	std::wcout << "\nUsing default acc = " << accelView.accelerator.description << '\n';

	std::cout << "\n##############################################################\n";

	cout << "Please wait..." << endl;

	setUpCSV();

	AMPWarmUp();
	runMultipleTimingsNoExplicitTile();
	runMultipleTimingsExplicitTile();
	
	// Write the final image that was generated to file for viewing.
	write_tga("output.tga");

	return 0;
}


//  http://warp.povusers.org/Mandelbrot/

/*
	Useful for getting coords to plot a specific mandlebrot fractle
	//https://sciencedemos.org.uk/mandelbrot.php

	Nice to play with
	//https://math.hws.edu/eck/js/mandelbrot/MB.html?ex=examples%2fspikey.xml
*/