/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#ifndef GAMEOFLIFE_HPP_
#define GAMEOFLIFE_HPP_

#include <vector>
#include <memory.h>
#include <iostream>

using Field = unsigned char;

struct Grid
{
	const unsigned width;
	const unsigned height;

	std::vector<Field> fields;

	Grid(const unsigned width, const unsigned height) : width(width), height(height)
	{
		fields.resize(width * height, 0);
	}

	const Field& at(unsigned x, unsigned y) const
	{
		return fields.at(x * width + y);
	}

	Field& at(unsigned x, unsigned y)
	{
		return fields.at(x * width + y);
	}

	void clear()
	{
		fields.clear();
		fields.resize(width * height, 0);
	}

	void seed(const unsigned numCells);

	void print(std::ostream& s) const;
};


//TODO parallel execution
void setupCPU(const Grid& grid, unsigned numSteps);
void setupGPU(const Grid& grid, unsigned numSteps);
void simulateCPULinear(const Grid& grid, Grid& output, unsigned numSteps);
void simulateGPULinear(const Grid& grid, Grid& output, unsigned numSteps);
void simulateCPUParallel(const Grid& grid, Grid& output, unsigned numSteps);
void simulateGPUParallel(const Grid& grid, Grid& output, unsigned numSteps);

#endif /* GAMEOFLIFE_HPP_ */
