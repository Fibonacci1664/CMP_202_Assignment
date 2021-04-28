#include "MyAMP.h"

/////////////////////////////////////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////////////////////

using namespace concurrency;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////

// GLOBALS
// Get all accelerators available to us and store in a vector so we can extract details.
std::vector<accelerator> accls = accelerator::get_all();

/////////////////////////////////////////////////////////////////////////////////////////////

// CONSTRUCTOR / DESTRUCTOR
MyAMP::MyAMP()
{

}

MyAMP::~MyAMP()
{

}

/////////////////////////////////////////////////////////////////////////////////////////////

void MyAMP::report_accelerator(const accelerator a)
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

// List and select the accelerator to use.
void MyAMP::list_accelerators()
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
void MyAMP::query_AMP_support()
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

void MyAMP::printAccelInUse()
{
	std::cout << "\n##############################################################\n";

	accelerator_view accelView = accelerator().default_view;
	accelerator::set_default(accelView.accelerator.default_accelerator);
	std::wcout << "\nUsing default acc = " << accelView.accelerator.description << '\n';

	std::cout << "\n##############################################################\n";
}

/////////////////////////////////////////////////////////////////////////////////////////////