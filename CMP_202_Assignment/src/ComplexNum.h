#pragma once
#include <amp.h>
#include <amp_math.h>

/////////////////////////////////////////////////////////////////////////////////////////////

using namespace concurrency;

/////////////////////////////////////////////////////////////////////////////////////////////

// Using our own Complex number structure and definitions as the Complex type is not available in the Concurrency namespace.
struct ComplexNum
{
	float x;
	float y;
};

/////////////////////////////////////////////////////////////////////////////////////////////

// Struct helper function.
ComplexNum c_add(ComplexNum c1, ComplexNum c2) restrict(cpu, amp) // restrict keyword - able to execute this function on the GPU and CPU
{
	ComplexNum tmp;
	float a = c1.x;
	float b = c1.y;
	float c = c2.x;
	float d = c2.y;
	tmp.x = a + c;
	tmp.y = b + d;

	return tmp;
}

/////////////////////////////////////////////////////////////////////////////////////////////

// Struct helper function.
float c_abs(ComplexNum c) restrict(cpu, amp)
{
	return concurrency::fast_math::sqrt(c.x * c.x + c.y * c.y);
}

/////////////////////////////////////////////////////////////////////////////////////////////

// Struct helper function.
ComplexNum c_mul(ComplexNum c1, ComplexNum c2) restrict(cpu, amp)
{
	ComplexNum tmp;
	float a = c1.x;
	float b = c1.y;
	float c = c2.x;
	float d = c2.y;
	tmp.x = a * c - b * d;
	tmp.y = b * c + a * d;

	return tmp;
}

/////////////////////////////////////////////////////////////////////////////////////////////