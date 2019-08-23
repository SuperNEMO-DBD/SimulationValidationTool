cmake_minimum_required(VERSION 3.3)
project(SimulationTool)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(ROOT REQUIRED)

include_directories(. ${ROOT_INCLUDE_DIRS})

add_executable(SimulationTool SimulationTool.cxx SimulationTool.h getopt_pp.cpp getopt_pp.h)
target_link_libraries(SimulationTool ${ROOT_LIBRARIES})
