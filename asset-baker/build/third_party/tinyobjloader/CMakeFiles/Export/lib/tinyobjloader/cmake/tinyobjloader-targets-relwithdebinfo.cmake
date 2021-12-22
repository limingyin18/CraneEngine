#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "tinyobjloader::tinyobjloader" for configuration "RelWithDebInfo"
set_property(TARGET tinyobjloader::tinyobjloader APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(tinyobjloader::tinyobjloader PROPERTIES
  IMPORTED_IMPLIB_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/tinyobjloader.lib"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/bin/tinyobjloader.dll"
  )

list(APPEND _IMPORT_CHECK_TARGETS tinyobjloader::tinyobjloader )
list(APPEND _IMPORT_CHECK_FILES_FOR_tinyobjloader::tinyobjloader "${_IMPORT_PREFIX}/lib/tinyobjloader.lib" "${_IMPORT_PREFIX}/bin/tinyobjloader.dll" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
