# Require C++14 compilation
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# include_directories (${TEST_SOURCE_DIR}/src
                     # )

add_executable (Test
                tests-main.cpp
                tests-EventQueue.cpp
                tests-EventQueueWithState.cpp
                tests-SIRSimulation.cpp)

target_link_libraries (Test
                       Catch)

add_test (NAME MyTest COMMAND Test WORKING_DIRECTORY <${CMAKE_CURRENT_SOURCE_DIR}>)
