/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "GameOfLife.hpp"

#include <random>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <functional>

void Grid::seed(const unsigned numCells)
{
//	static std::mt19937 generator(std::chrono::system_clock::now().time_since_epoch().count());
//	for(unsigned i = 0; i < numCells; ++i)
//	{
//		double relX = (double)generator() / (double)generator.max();
//		double relY = (double)generator() / (double)generator.max();
//		at(relX * width, relY * height) = 1;
//	}

	//block -> stays
	at(1, 1) = 1;
	at(1, 2) = 1;
	at(2, 1) = 1;
	at(2, 2) = 1;

	//beehive -> stays
	at(12, 12) = 1;
	at(13, 11) = 1;
	at(14, 11) = 1;
	at(15, 12) = 1;
	at(13, 13) = 1;
	at(14, 13) = 1;

	//tub -> stays
	at(28, 28) = 1;
	at(29, 27) = 1;
	at(30, 28) = 1;
	at(29, 29) = 1;

	//blinker -> oscillates
	at(1, 7) = 1;
	at(1, 8) = 1;
	at(1, 9) = 1;

	//beacon -> oscillates
	at(20, 5) = 1;
	at(21, 5) = 1;
	at(20, 6) = 1;
	at(21, 6) = 1;
	at(22, 7) = 1;
	at(22, 8) = 1;
	at(23, 7) = 1;
	at(23, 8) = 1;
}

void Grid::print(std::ostream& s) const
{
	s << std::endl;
	for(unsigned y = 0; y < height; ++y)
	{
		s << y << ':' << '\t';
		for(unsigned x = 0; x < width; ++x)
		{
			s << (at(x, y) ? 'x' : ' ');
		}
		s << std::endl;
	}
}

static void profile(const std::function<void()>& func, const std::string& name)
{
	auto start = std::chrono::high_resolution_clock::now();
	func();
	auto end = std::chrono::high_resolution_clock::now();
	std::cout << std::endl;
	std::cout << name << ": " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() << " us" << std::endl;
}

int main(int argc, char **argv)
{
	if(argc < 4)
	{
		std::cout << "Usage: [program] CPU|GPU width height [cells] [steps]" << std::endl;
		exit(0);
	}

	std::string version = argv[1];

	auto width = std::atol(argv[2]);
	auto height = std::atol(argv[3]);

	Grid grid(width, height);
	Grid output(width, height);

	if(argc >= 5)
		grid.seed(std::atol(argv[4]));
	else
		grid.seed(width);

	int steps = 1024;
	if(argc >= 6)
		steps = std::atol(argv[5]);

	std::cout << "Before:" << std::endl;
	grid.print(std::cout);
	std::cout << std::endl;

	if(version.find("GPU") != std::string::npos)
	{
		std::cout << "Running on GPU with a " << grid.width << " * " << grid.height << " grid and " << steps << " steps" << std::endl;
		profile([&grid, steps]() -> void {setupGPU(grid, steps);}, "SetupGPU");
		profile([&grid, &steps, &output]() -> void {simulateGPULinear(grid, output, steps);}, "SimulateGPU (linear)");
		output.print(std::cout);
		profile([&grid, &steps, &output]() -> void {simulateGPUParallel(grid, output, steps);}, "SimulateGPU (parallel)");
		output.print(std::cout);
	}
	if(version.find("CPU") != std::string::npos)
	{
		std::cout << "Running on CPU with a " << grid.width << " * " << grid.height << " grid and " << steps << " steps" << std::endl;
		profile([&grid,steps]() -> void {setupCPU(grid,steps);}, "SetupCPU");
		profile([&grid, &steps, &output]() -> void{simulateCPULinear(grid, output, steps);}, "SimulateCPU (linear)");
		output.print(std::cout);
		profile([&grid, &steps, &output]() -> void{simulateCPUParallel(grid, output, steps);}, "SimulateCPU (parallel)");
		output.print(std::cout);
	}

	return 0;
}
