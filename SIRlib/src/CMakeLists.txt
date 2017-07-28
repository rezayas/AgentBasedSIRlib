# Set headers
set(header_path "${SIRlib_SOURCE_DIR}/include/SIRlib")
set(header ${header_path}/Individual.h
		   ${header_path}/SIRlib.h)

# Set source files
set(src SIRlib.cpp)

# Require C++14 compilation
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include SimulationLib
find_package(Boost REQUIRED)
find_package(SimulationLib REQUIRED)
find_package(StatisticalDistributionsLib REQUIRED)

# Configure config.hpp.in
# configure_file("config.hpp.in" "${CMAKE_CURRENT_BINARY_DIR}/config_impl.hpp")

# Define library target
add_library(SIRlib ${header} ${src})

include_directories(${Boost_INCLUDE_DIRS})

target_include_directories(SIRlib PUBLIC
  $<BUILD_INTERFACE:${SIRlib_SOURCE_DIR}/include> # for headers when building
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for config_impl.hpp when building
  $<INSTALL_INTERFACE:${include_dest}> # for client in install mode
  $<INSTALL_INTERFACE:${lib_dest}>) # for config_impl.hpp in install mode

target_link_libraries(SIRlib PUBLIC SimulationLib StatisticalDistributionsLib)

install(TARGETS SIRlib EXPORT SIRlib DESTINATION "${lib_dest}")
install(FILES ${header} DESTINATION "${include_dest}")
# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/config_impl.hpp DESTINATION "${include_dest}")
