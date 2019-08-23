cmake_minimum_required(VERSION 3.3)
project(SimulationTool)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(ROOT REQUIRED)
find_package(Boost REQUIRED filesystem system)

include_directories(${ROOT_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS} include)

add_executable(SimulationTool SimulationTool.cxx SimulationTool.h)
target_link_libraries(SimulationTool ${ROOT_LIBRARIES} ${Boost_LIBRARIES})