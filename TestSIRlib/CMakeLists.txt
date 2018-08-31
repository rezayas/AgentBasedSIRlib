cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(TestSIRlib)

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(SIRlib REQUIRED)
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(SerialSIRsim run-SIRsim-serial.cpp SIRSimRunner.cpp)
add_executable(ParallelSIRsim run-SIRsim-parallel.cpp SIRSimRunner.cpp)

target_link_libraries(SerialSIRsim PUBLIC SimulationLib StatisticalDistributionsLib SIRlib Threads::Threads)
target_link_libraries(ParallelSIRsim PUBLIC SimulationLib StatisticalDistributionsLib SIRlib Threads::Threads)
