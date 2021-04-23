#include "Filter.h"
#include "Mandlebrot.h"

/////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <amp.h>

/////////////////////////////////////////////////////////////////////////////////////////////

using namespace concurrency;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////

//GLOBALS
// Get all accelerators available to us and store in a vector so we can extract details.
std::vector<accelerator> accls = accelerator::get_all();
Mandlebrot mandlebrot;

/////////////////////////////////////////////////////////////////////////////////////////////

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

/////////////////////////////////////////////////////////////////////////////////////////////

// List and select the accelerator to use
void list_accelerators()
{
	// iterates over all accelerators and print characteristics
	for (int i = 0; i < accls.size(); i++)
	{
		accelerator a = accls[i];
		report_accelerator(a);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////

// Query if AMP accelerator exists on hardware.
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
}

/////////////////////////////////////////////////////////////////////////////////////////////

////////////////////// IMPORTANT INFO RELATED TO THE WARM UP CALL BELOW /////////////////////

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

////////////////////// IMPORTANT INFO RELATED TO THE WARM UP CALLS BELOW /////////////////////
void runAMPWarmUp()
{
	mandlebrot.compute_mandelbrot_with_AMP(-2.0, 1.0, 1.125, -1.125, 0, mandlebrot.getHeight());
	mandlebrot.compute_mandelbrot_with_AMP_tiling(-2.0, 1.0, 1.125, -1.125, 0, mandlebrot.getHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////

void createMandlebrot()
{
	mandlebrot.setUpCSV();
	mandlebrot.compute_mandelbrot_with_AMP_tiling(-2.0, 1.0, 1.125, -1.125, 0, mandlebrot.getHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////

void printAccelInUse()
{
	std::cout << "\n##############################################################\n";

	accelerator_view accelView = accelerator().default_view;
	accelerator::set_default(accelView.accelerator.default_accelerator);
	std::wcout << "\nUsing default acc = " << accelView.accelerator.description << '\n';

	std::cout << "\n##############################################################\n";

	cout << "Please wait..." << endl;
}

/////////////////////////////////////////////////////////////////////////////////////////////

void setUpAMP()
{
	query_AMP_support();
	printAccelInUse();
	runAMPWarmUp();
}

////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	cout << "Please wait..." << endl;

	setUpAMP();
	createMandlebrot();

	// Write the final image that was generated to file for viewing.
	mandlebrot.write_tga("output.tga");

	return 0;
}