cmake_minimum_required (VERSION 3.1)

# Option to enable/disable debug-output
option(BUILD_DEBUG "Build with debugging symbols. Otherwise build for performance" ON)

project (GameOfLife)

find_package( OpenCL REQUIRED )

if(BUILD_DEBUG)
	LIST(APPEND CMAKE_CXX_FLAGS "-std=c++11 -O3 -Wall -Wextra -pthread")
	SET(CMAKE_BUILD_TYPE 		Debug)
else()
	LIST(APPEND CMAKE_CXX_FLAGS "-std=c++11 -g3 -Wall -Wextra -pthread")
	SET(CMAKE_BUILD_TYPE 		Release)
endif()
include_directories( ${OpenCL_INCLUDE_DIRS} )

add_executable(GameOfLife GameOfLife.cpp CPU.cpp GPU.cpp)
target_link_libraries(GameOfLife ${OpenCL_LIBRARIES})
target_link_libraries(GameOfLife pthread)