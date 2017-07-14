cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(run-SIRlib)

find_package(SIRlib REQUIRED)
find_package(SimulationLib REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(src run-SIRlib.cpp)
add_executable(run-SIRlib ${src})
target_link_libraries(run-SIRlib PUBLIC SIRlib)
