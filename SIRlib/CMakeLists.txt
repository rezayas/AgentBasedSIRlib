cmake_minimum_required(VERSION 3.3)
project (SIRlib)

# Define library version
set(SIRlib_VERSION_MAJOR 0 CACHE STRING "major version" FORCE)
set(SIRlib_VERSION_MINOR 1 CACHE STRING "minor version" FORCE)
set(SIRlib_VERSION ${SIRlib_VERSION_MAJOR}.${SIRlib_VERSION_MINOR} CACHE STRING "version" FORCE)

# Define install destinations
set(include_dest "include/SIRlib-${SIRlib_VERSION}")
set(main_lib_dest "lib/SIRlib-${SIRlib_VERSION}")
set(lib_dest ${main_lib_dest}/${CMAKE_BUILD_TYPE})

# Define options here

# Add subdirectories
add_subdirectory(src)

enable_testing()
add_subdirectory(tests)

install(FILES SIRlib-config.cmake DESTINATION ${main_lib_dest})
install(EXPORT SIRlib DESTINATION ${lib_dest})
