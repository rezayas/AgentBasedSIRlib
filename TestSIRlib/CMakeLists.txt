cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(TestSIRlib)

find_package(SIRlib REQUIRED)
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(SIRsim run-SIRsim.cpp)

target_link_libraries(SIRsim PUBLIC SimulationLib StatisticalDistributionsLib SIRlib)
# target_link_libraries(SIRsim PUBLIC SIRlib)
