#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "MINIZIP::minizip" for configuration "Debug"
set_property(TARGET MINIZIP::minizip APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(MINIZIP::minizip PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/minizip.lib"
  )

list(APPEND _cmake_import_check_targets MINIZIP::minizip )
list(APPEND _cmake_import_check_files_for_MINIZIP::minizip "${_IMPORT_PREFIX}/lib/minizip.lib" )

# Import target "MINIZIP::ppmd" for configuration "Debug"
set_property(TARGET MINIZIP::ppmd APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(MINIZIP::ppmd PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/ppmd.lib"
  )

list(APPEND _cmake_import_check_targets MINIZIP::ppmd )
list(APPEND _cmake_import_check_files_for_MINIZIP::ppmd "${_IMPORT_PREFIX}/lib/ppmd.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
