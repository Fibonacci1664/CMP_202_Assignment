#pragma once

/////////////////////////////////////////////////////////////////////////////////////////////

const int KERNEL_SIZE = 7;

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

	float filter[KERNEL_SIZE] = { 0.000904706,	0.003157733,	0.00668492,		0.008583607,	0.00668492,		0.003157733,	0.000904706 };

};

/////////////////////////////////////////////////////////////////////////////////////////////