# Require C++14 compilation
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# add_library(SIRlib ${CMAKE_CURRENT_SOURCE_DIR}/../src)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)

add_executable (Test
                tests-main.cpp
                tests-SIRSimulation.cpp
                tests-CSVImport.cpp
                TestB.cpp
                TestCDE.cpp
                TestGJ.cpp
                TestLMN.cpp
                TestPRTUW.cpp)

target_link_libraries(Test Catch SimulationLib StatisticalDistributionsLib SIRlib)

add_test(NAME MyTest COMMAND Test WORKING_DIRECTORY <${CMAKE_CURRENT_SOURCE_DIR}>)
