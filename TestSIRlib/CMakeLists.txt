cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(TestSIRlib)

find_package(SIRlib REQUIRED)
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(SerialSIRsim run-SIRsim-serial.cpp SIRSimRunner.cpp)
add_executable(ParallelSIRsim run-SIRsim-parallel.cpp SIRSimRunner.cpp)

target_link_libraries(SerialSIRsim PUBLIC SimulationLib StatisticalDistributionsLib SIRlib)
target_link_libraries(ParallelSIRsim PUBLIC SimulationLib StatisticalDistributionsLib SIRlib)

# Allow cmake to link python
find_package(PythonLibs REQUIRED)

add_library(SIRsimPyModule SHARED SIRsimPyModule.cpp SIRSimRunner.cpp)
target_link_libraries(SIRsimPyModule PUBLIC SimulationLib StatisticalDistributionsLib SIRlib ${PYTHON_LIBRARIES})
include_directories(${PYTHON_INCLUDE_DIRS})


find_program(PYTHON "python")

if (PYTHON)
    set(SETUP_PY_IN "${CMAKE_CURRENT_SOURCE_DIR}/setup.py.in")
    set(SETUP_PY    "${CMAKE_CURRENT_BINARY_DIR}/setup.py")
    set(DEPS        SIRlib StatisticalDistributionsLib SimulationLib)
    set(OUTPUT      "${CMAKE_CURRENT_BINARY_DIR}/build/timestamp")

    configure_file(${SETUP_PY_IN} ${SETUP_PY})

    add_custom_command(OUTPUT ${OUTPUT}
                       COMMAND ${PYTHON} ${SETUP_PY} build
                       COMMAND ${CMAKE_COMMAND} -E touch ${OUTPUT}
                       DEPENDS ${DEPS})

    add_custom_target(target ALL DEPENDS ${OUTPUT})

    install(CODE "execute_process(COMMAND ${PYTHON} ${SETUP_PY} install)")
endif()
