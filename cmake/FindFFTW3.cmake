# FindFFTW3.cmake
# This file is used by CMake to locate the FFTW3 library on the system.
# It sets the FFTW3_INCLUDE_DIRS and FFTW3_LIBRARIES variables.

# Find the include directory for FFTW3
find_path(FFTW3_INCLUDE_DIR fftw3.h
  HINTS
    ${FFTW3_DIR}/include
    /usr/include
    /usr/local/include
    /opt/local/include
)

# Find the library for FFTW3
find_library(FFTW3_LIBRARY fftw3
  HINTS
    ${FFTW3_DIR}/lib
    /usr/lib
    /usr/local/lib
    /opt/local/lib
)

# Find the multi-threaded FFTW3 library, if available
find_library(FFTW3_THREADS_LIBRARY fftw3_threads
  HINTS
    ${FFTW3_DIR}/lib
    /usr/lib
    /usr/local/lib
    /opt/local/lib
)

# Check if the FFTW3 library was found
if(FFTW3_INCLUDE_DIR AND FFTW3_LIBRARY)
  set(FFTW3_FOUND TRUE)

  # Create the FFTW3 imported target
  add_library(FFTW3::fftw3 UNKNOWN IMPORTED)
  set_target_properties(FFTW3::fftw3 PROPERTIES
    IMPORTED_LOCATION ${FFTW3_LIBRARY}
    INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIR}
  )

  # Create the FFTW3 Threads imported target, if found
  if(FFTW3_THREADS_LIBRARY)
    add_library(FFTW3::fftw3_threads UNKNOWN IMPORTED)
    set_target_properties(FFTW3::fftw3_threads PROPERTIES
      IMPORTED_LOCATION ${FFTW3_THREADS_LIBRARY}
      INTERFACE_INCLUDE_DIRECTORIES ${FFTW3_INCLUDE_DIR}
    )
  endif()

  message(STATUS "Found FFTW3: ${FFTW3_LIBRARY}")

else()
  set(FFTW3_FOUND FALSE)
  message(STATUS "FFTW3 not found.")
endif()

# Mark variables as advanced to hide from the cache
mark_as_advanced(FFTW3_INCLUDE_DIR FFTW3_LIBRARY FFTW3_THREADS_LIBRARY)
