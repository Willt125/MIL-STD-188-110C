find_path(SNDFILE_INCLUDE_DIR sndfile.h)
find_library(SNDFILE_LIBRARY NAMES sndfile libsndfile)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SndFile DEFAULT_MSG SNDFILE_LIBRARY SNDFILE_INCLUDE_DIR)

if(SNDFILE_FOUND)
  set(SNDFILE_LIBRARIES ${SNDFILE_LIBRARY})
  set(SNDFILE_INCLUDE_DIRS ${SNDFILE_INCLUDE_DIR})

  mark_as_advanced(SNDFILE_LIBRARY SNDFILE_INCLUDE_DIR)

  add_library(SndFile::sndfile UNKNOWN IMPORTED)
  set_target_properties(SndFile::sndfile PROPERTIES
    IMPORTED_LOCATION "${SNDFILE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${SNDFILE_INCLUDE_DIR}"
  )
endif()