#include "Filter.h"
#include "Mandlebrot.h"
#include "MyAMP.h"

/////////////////////////////////////////////////////////////////////////////////////////////

#include <iostream>

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
void runAMPWarmUp(Mandlebrot* mandle)
{
	mandle->compute_mandelbrot_with_AMP(-2.0, 1.0, 1.125, -1.125, 0, mandle->getHeight());
	mandle->compute_mandelbrot_with_AMP_tiling(-2.0, 1.0, 1.125, -1.125, 0, mandle->getHeight());
}

/////////////////////////////////////////////////////////////////////////////////////////////

void createMandlebrot(Mandlebrot* mandle)
{
	mandle->setUpCSV();
	mandle->runMultipleTimingsNoExplicitTile();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void setUpAMP(MyAMP* theAMP)
{
	theAMP->query_AMP_support();
	theAMP->printAccelInUse();
}

////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	Mandlebrot mandlebrot;
	MyAMP ampObj;

	std::cout << "Please wait..." << '\n';

	setUpAMP(&ampObj);
	//runAMPWarmUp(&mandlebrot);
	createMandlebrot(&mandlebrot);

	// Write both images that were generated to file for viewing.
	//mandlebrot.write_tga("original_image.tga", false);
	//mandlebrot.write_tga("blurred_image.tga", true);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////