/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#define GLOBAL __global 
#include "helper.hpp"

__kernel void GOLLinear(__global uchar* buffer1, __global uchar* buffer2)
{
	__global uchar* in = buffer1;
	__global uchar* out = buffer2;
	for(uint s = 0; s < NUM_STEPS; ++s)
	{
		//run simulation step
		for(uint x = 0; x < width; ++x)
		{
			for(uint y = 0; y < height; ++y)
			{
				updateCell(in, out, x, y);
			}
		}
		__global uchar* tmp = in;
		in = out;
		out = tmp;
	}
}


__kernel void GOLParallel(__global uchar* buffer1, __global uchar* buffer2)
{
	const size_t lid = get_local_id(0);
	__global uchar* in = buffer1;
	__global uchar* out = buffer2;
	for(uint s = 0; s < NUM_STEPS; ++s)
	{
		//run simulation step
		for(uint x = lid * PER_ITEM; x < (lid + 1) * PER_ITEM; ++x)
		{
			for(uint y = 0; y < height; ++y)
			{
				updateCell(in, out, x, y);
			}
		}
		barrier(CLK_GLOBAL_MEM_FENCE);
		__global uchar* tmp = in;
		in = out;
		out = tmp;
	}
}
