/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "GameOfLife.hpp"
#undef width
#undef height
#define GLOBAL
#include "helper.hpp"
#include <thread>

void setupCPU(const Grid& grid, unsigned numSteps)
{

}

void simulateCPULinear(const Grid& grid, Grid& output, unsigned numSteps)
{
	std::vector<Field> grid1(grid.fields);
	std::vector<Field> grid2(grid.fields);

	std::vector<Field>* before = &grid1;
	std::vector<Field>* after = &grid2;
	for(unsigned i = 0; i < numSteps; ++i)
	{
		//run simulation step
		for(std::size_t x = 0; x < grid.width; ++x)
		{
			for(std::size_t y = 0; y < grid.height; ++y)
			{
				updateCell(before->data(), after->data(), grid.width, grid.height, x, y);
			}
		}

		//switch source and destination
		auto tmp = before;
		before = after;
		after = tmp;
	}

	memcpy(output.fields.data(), before->data(), grid.width * grid.height);
}

void simulateCPUParallel(const Grid& grid, Grid& output, unsigned numSteps)
{
	unsigned int numThreads = std::thread::hardware_concurrency();
	std::vector<std::thread> threads;
	threads.resize(numThreads);

	std::vector<Field> grid1(grid.fields);
	std::vector<Field> grid2(grid.fields);

	std::vector<Field>* before = &grid1;
	std::vector<Field>* after = &grid2;
	for(unsigned i = 0; i < numSteps; ++i)
	{
		//run simulation step
		const std::function<void(unsigned, unsigned)> runner = [&grid, &before, &after](unsigned id, unsigned size) -> void{
			for(std::size_t x = id * size; x < (id+1)*size; ++x)
			{
				for(std::size_t y = 0; y < grid.height; ++y)
				{
					updateCell(before->data(), after->data(), grid.width, grid.height, x, y);
				}
			}
		};
		for(unsigned t = 0; t < numThreads; ++t)
		{
			threads[t] = std::thread(runner, t, grid.width / numThreads);
		}
		for(unsigned t = 0; t < numThreads; ++t)
		{
			threads.at(t).join();
		}

		//switch source and destination
		auto tmp = before;
		before = after;
		after = tmp;
	}

	memcpy(output.fields.data(), before->data(), grid.width * grid.height);
}
