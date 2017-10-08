/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "GameOfLife.hpp"
#include <CL/cl.h>
#include <stdexcept>
#include <fstream>
#include <functional>

static cl_context context = nullptr;
static cl_command_queue queue = nullptr;
static cl_program program = nullptr;
static cl_kernel kernelLinear = nullptr;
static cl_kernel kernelParallel = nullptr;
static cl_int errorCode = CL_SUCCESS;
static cl_mem buffer1 = nullptr, buffer2 = nullptr;

static void checkError(cl_int returnCode, const std::function<std::string()>& messageSupplier = []() -> std::string{return "";})
{
	if(returnCode < 0)
		throw std::runtime_error((std::to_string(returnCode) + ": ") + messageSupplier());
}

void setupGPU(const Grid& grid, unsigned numSteps)
{
	cl_platform_id platform = nullptr;
	checkError(clGetPlatformIDs(1, &platform, nullptr));
	cl_device_id device = nullptr;
	checkError(clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr));
	context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &errorCode);
	checkError(errorCode);
	queue = clCreateCommandQueue(context, device, 0L, &errorCode);
	checkError(errorCode);

	std::string code;
	{
		std::ifstream f("./GOL.cl");
		std::string tmp;
		while (std::getline(f, tmp))
		{
			code.append(tmp).append("\n");
		}
	}

	auto codeLines = code.data();
	auto codeLength = code.length();
	program = clCreateProgramWithSource(context, 1, &codeLines, &codeLength, &errorCode);
	checkError(errorCode);

	checkError(clBuildProgram(program, 1, &device, (std::string("-Dwidth=") + std::to_string(grid.width) + std::string(" -Dheight=") + std::to_string(grid.height) + std::string(" -DPER_ITEM=") + std::to_string(grid.width / 8) + std::string(" -DNUM_STEPS=") + std::to_string(numSteps)).data(), nullptr, nullptr),
			[program, device]() -> std::string
	{
		size_t len = 0;
		char tmp[4096];
		checkError(clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 4096, tmp, &len));
		return std::string(tmp, len);
	});

	kernelLinear = clCreateKernel(program, "GOLLinear", &errorCode);
	checkError(errorCode);
	kernelParallel = clCreateKernel(program, "GOLParallel", &errorCode);
	checkError(errorCode);

	buffer1 = clCreateBuffer(context, CL_MEM_READ_WRITE, grid.width * grid.height, nullptr, &errorCode);
	checkError(errorCode);
	buffer2 = clCreateBuffer(context, CL_MEM_READ_WRITE, grid.width * grid.height, nullptr, &errorCode);
	checkError(errorCode);

	//so they are removed after the second (both functions) release
	checkError(clRetainMemObject(buffer1));
	checkError(clRetainMemObject(buffer2));
}

void simulateGPULinear(const Grid& grid, Grid& output, unsigned numSteps)
{
	void* inputPtr = clEnqueueMapBuffer(queue, buffer1, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, grid.width * grid.height, 0, nullptr, nullptr, &errorCode);
	checkError(errorCode);
	void* outputPtr = clEnqueueMapBuffer(queue, buffer2, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, grid.width * grid.height, 0, nullptr, nullptr, &errorCode);
	checkError(errorCode);

	memcpy(inputPtr, grid.fields.data(), grid.width * grid.height);
	memset(outputPtr, 0, grid.width * grid.height);

	checkError(clSetKernelArg(kernelLinear, 0, sizeof(cl_mem), &buffer1));
	checkError(clSetKernelArg(kernelLinear, 1, sizeof(cl_mem), &buffer2));

	cl_event event;
	checkError(clEnqueueTask(queue, kernelLinear, 0, nullptr, &event));
	checkError(clWaitForEvents(1, &event));

	memcpy(output.fields.data(), numSteps % 2 == 1 ? outputPtr : inputPtr, grid.width * grid.height);

	checkError(clEnqueueUnmapMemObject(queue, buffer1, inputPtr, 0, nullptr, nullptr));
	checkError(clEnqueueUnmapMemObject(queue, buffer2, outputPtr, 0, nullptr, nullptr));

	checkError(clReleaseMemObject(buffer1));
	checkError(clReleaseMemObject(buffer2));
}

void simulateGPUParallel(const Grid& grid, Grid& output, unsigned numSteps)
{
	void* inputPtr = clEnqueueMapBuffer(queue, buffer1, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, grid.width * grid.height, 0, nullptr, nullptr, &errorCode);
	checkError(errorCode);
	void* outputPtr = clEnqueueMapBuffer(queue, buffer2, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0, grid.width * grid.height, 0, nullptr, nullptr, &errorCode);
	checkError(errorCode);

	memcpy(inputPtr, grid.fields.data(), grid.width * grid.height);
	memset(outputPtr, 0, grid.width * grid.height);

	checkError(clSetKernelArg(kernelParallel, 0, sizeof(cl_mem), &buffer1));
	checkError(clSetKernelArg(kernelParallel, 1, sizeof(cl_mem), &buffer2));

	cl_event event;
	size_t offsets[3] = {0, 0, 0};
	size_t sizes[3] = {8, 1, 1};
	checkError(clEnqueueNDRangeKernel(queue, kernelParallel, 1, offsets, sizes, sizes, 0, nullptr, &event));
	checkError(clWaitForEvents(1, &event));

	memcpy(output.fields.data(), numSteps % 2 == 1 ? outputPtr : inputPtr, grid.width * grid.height);

	checkError(clEnqueueUnmapMemObject(queue, buffer1, inputPtr, 0, nullptr, nullptr));
	checkError(clEnqueueUnmapMemObject(queue, buffer2, outputPtr, 0, nullptr, nullptr));

	checkError(clReleaseMemObject(buffer1));
	checkError(clReleaseMemObject(buffer2));
}



