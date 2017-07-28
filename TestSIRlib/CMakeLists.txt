cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(TestSIRlib)

find_package(Boost REQUIRED)
find_package(SIRlib REQUIRED)
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(SerialSIRsim run-SIRsim-serial.cpp SIRSimRunner.cpp)
add_executable(ParallelSIRsim run-SIRsim-parallel.cpp SIRSimRunner.cpp)

target_link_libraries(SerialSIRsim PUBLIC SimulationLib StatisticalDistributionsLib SIRlib)
target_link_libraries(ParallelSIRsim PUBLIC SimulationLib StatisticalDistributionsLib SIRlib)
