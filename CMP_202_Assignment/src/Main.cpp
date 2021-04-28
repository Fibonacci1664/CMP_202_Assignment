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
	mandle->compute_mandelbrot_with_AMP(-2.0, 1.0, 1.125, -1.125, 0, HEIGHT);
}

/////////////////////////////////////////////////////////////////////////////////////////////

void createMandlebrot(Mandlebrot* mandle)
{
	mandle->setUpCSV();
	mandle->runMultipleTimings();
}

/////////////////////////////////////////////////////////////////////////////////////////////

void setUpAMP(MyAMP* theAMP)
{
	theAMP->query_AMP_support();
	theAMP->printAccelInUse();
}

////////////////////////////////////////////////////////////////////////////////////////////

void imagePrefs(int& size)
{
	std::cout << "Please enter size of image to generate, (256 - 1024):> ";
	std::cin >> size;

	while (size < 256 || size > 1024)
	{
		std::cout << "Please enter size of image to generate, (256 - 1024):> ";
		std::cin >> size;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char* argv[])
{
	MyAMP ampObj;
	int size = 0;

	//imagePrefs(size);

	//Mandlebrot mandlebrot(size);
	Mandlebrot mandlebrot;

	setUpAMP(&ampObj);
	runAMPWarmUp(&mandlebrot);

	std::cout << "Please wait while the image is generated..." << '\n';

	createMandlebrot(&mandlebrot);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////