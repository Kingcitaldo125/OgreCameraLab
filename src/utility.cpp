#include <stdafx.h>
#include <utility.h>

float ssurge::rand_float(float min, float max)
{
	double v;

	v = (max - min) * (double)rand() / RAND_MAX;

	return (float)v;
}