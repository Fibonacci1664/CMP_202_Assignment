#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////

const int KERNEL_SIZE = 21;

/////////////////////////////////////////////////////////////////////////////////////////////

struct Filter
{
	// For blur
	/*float filter[KERNEL_SIZE][KERNEL_SIZE] = {			{0.000904706,	0.003157733,	0.00668492,		0.008583607,	0.00668492,		0.003157733,	0.000904706},
														{0.003157733,	0.01102157,		0.023332663,	0.029959733,	0.023332663,	0.01102157,		0.003157733},
														{0.00668492,	0.023332663,	0.049395249,	0.063424755,	0.049395249,	0.023332663,	0.00668492},
														{0.008583607,	0.029959733,	0.063424755,	0.081438997,	0.063424755,	0.029959733,	0.008583607},
														{0.00668492,	0.023332663,	0.049395249,	0.063424755,	0.049395249,	0.023332663,	0.00668492},
														{0.003157733,	0.01102157,		0.023332663,	0.029959733,	0.023332663,	0.01102157,		0.003157733},
														{0.000904706,	0.003157733,	0.00668492,		0.008583607,	0.00668492,		0.003157733,	0.000904706} };*/

	// These numbers were so small that they were essentially returning zero for the final output image resulting in a final image of just all black.
	//float filter[KERNEL_SIZE] = { 0.000904706,	0.003157733,	0.00668492,		0.008583607,	0.00668492,		0.003157733,	0.000904706 };
	
	// SD 2, filter size 21
	float filter[KERNEL_SIZE] = {	0.0415884f, 0.0432451f, 0.0447924f, 0.0462143f, 0.0474954f, 0.0486217f, 0.0495807f,
									0.0503615f, 0.0509551f, 0.0513548f, 0.051581f, 0.0513548f, 0.0509551f, 0.0503615f,
									0.0495807f, 0.0486217f, 0.0474954f, 0.0462143f, 0.0447924f, 0.0432451f, 0.0415884f };
	
	// SD of 1.21 filter size 7
	//float filter[KERNEL_SIZE] = {0.030078323f, 0.104983664f, 0.2222250419f, 0.285375187f, 0.2222250419f, 0.104983664f, 0.030078323f };
};

/////////////////////////////////////////////////////////////////////////////////////////////