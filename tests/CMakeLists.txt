list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find the installed gtest package
find_package(GTest REQUIRED)
find_package(SndFile REQUIRED)
find_package(FFTW3 REQUIRED)

# Add test executable
add_executable(PSKModulatorTest PSKModulatorTests.cpp)

# Link the test executable with the GTest libraries
target_link_libraries(PSKModulatorTest GTest::GTest GTest::Main FFTW3::fftw3 SndFile::sndfile)

# Enable C++17 standard
set_target_properties(PSKModulatorTest PROPERTIES CXX_STANDARD 17)

# Add test cases
include(GoogleTest)
gtest_discover_tests(PSKModulatorTest)