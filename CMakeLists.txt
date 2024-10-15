cmake_minimum_required(VERSION 3.10)
project(MILSTD110C)

# Set C++17 standard
set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(include)
include_directories(include/encoder)
include_directories(include/modulation)
include_directories(include/utils)

# Add subdirectories for organization
enable_testing()
add_subdirectory(tests)

# Set source files
set(SOURCES main.cpp)

# Link with libsndfile
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
find_package(SndFile REQUIRED)
find_package(FFTW3 REQUIRED)

# Add executable
add_executable(MILSTD110C ${SOURCES})

# Link executable with libsndfile library
target_link_libraries(MILSTD110C SndFile::sndfile)
target_link_libraries(MILSTD110C FFTW3::fftw3)

# Debug and Release Build Types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Add options for a Debug build
if(CMAKE_BUILD_TYPE MATCHES Debug)
    message(STATUS "Building with Debug flags")
    target_compile_definitions(MILSTD110C PRIVATE DEBUG)
    target_compile_options(MILSTD110C PRIVATE -g -O0) # Add debugging symbols, no optimization
endif()

# Add options for a Release build
if(CMAKE_BUILD_TYPE MATCHES Release)
    message(STATUS "Building with Release flags")
    target_compile_options(MILSTD110C PRIVATE -O3) # Optimize for performance
endif()
