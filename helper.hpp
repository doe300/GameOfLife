/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#ifndef HELPER_HPP_
#define HELPER_HPP_

#ifdef width
unsigned char countNeighbors(GLOBAL unsigned char* fields, unsigned x, unsigned y)
#else
unsigned char countNeighbors(GLOBAL unsigned char* fields, const unsigned width, const unsigned height, unsigned x, unsigned y)
#endif
{
	unsigned char neighbors = 0;
	if(x > 0)
	{
		if(y > 0)
			neighbors += fields[(x-1) * width + y-1];
		neighbors += fields[(x-1) * width + y];
		if(y < height - 1)
			neighbors += fields[(x-1) * width + y+1];
	}
	if(y > 0)
		neighbors += fields[x * width + y-1];
	if(y < height -1)
		neighbors += fields[x * width + y+1];
	if(x < width - 1)
	{
		if(y > 0)
			neighbors += fields[(x+1) * width + y-1];
		neighbors += fields[(x+1) * width + y];
		if(y < height - 1)
			neighbors += fields[(x+1) * width + y+1];
	}
	return neighbors;
}

#ifdef width
void updateCell(GLOBAL unsigned char* inputFields, GLOBAL unsigned char* outputFields, unsigned x, unsigned y)
#else
void updateCell(GLOBAL unsigned char* inputFields, GLOBAL unsigned char* outputFields, const unsigned width, const unsigned height, unsigned x, unsigned y)
#endif
{
	bool cellAlive = inputFields[x * width + y] == 1;
#ifdef width
	unsigned char neighbors = countNeighbors(inputFields, x, y);
#else
	unsigned char neighbors = countNeighbors(inputFields, width, height, x, y);
#endif
	//Any live cell with fewer than two live neighbours dies, as if caused by underpopulation.
	if(cellAlive && neighbors < 2)
		outputFields[x * width + y] = 0;
	//Any live cell with two or three live neighbours lives on to the next generation.
	else if(cellAlive && (neighbors == 2 || neighbors == 3))
		outputFields[x * width + y] = 1;
	//Any live cell with more than three live neighbours dies, as if by overpopulation.
	else if(cellAlive && neighbors > 3)
		outputFields[x * width + y] = 0;
	//Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
	else if(!cellAlive && neighbors == 3)
		outputFields[x * width + y] = 1;
	else
		outputFields[x * width + y] = 0;
}

#endif /* HELPER_HPP_ */
