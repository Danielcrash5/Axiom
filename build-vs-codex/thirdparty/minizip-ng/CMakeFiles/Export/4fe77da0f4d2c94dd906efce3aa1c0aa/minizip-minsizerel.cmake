#----------------------------------------------------------------
# Generated CMake target import file for configuration "MinSizeRel".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "MINIZIP::minizip" for configuration "MinSizeRel"
set_property(TARGET MINIZIP::minizip APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(MINIZIP::minizip PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "C"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/minizip.lib"
  )

list(APPEND _cmake_import_check_targets MINIZIP::minizip )
list(APPEND _cmake_import_check_files_for_MINIZIP::minizip "${_IMPORT_PREFIX}/lib/minizip.lib" )

# Import target "MINIZIP::ppmd" for configuration "MinSizeRel"
set_property(TARGET MINIZIP::ppmd APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(MINIZIP::ppmd PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_MINSIZEREL "C"
  IMPORTED_LOCATION_MINSIZEREL "${_IMPORT_PREFIX}/lib/ppmd.lib"
  )

list(APPEND _cmake_import_check_targets MINIZIP::ppmd )
list(APPEND _cmake_import_check_files_for_MINIZIP::ppmd "${_IMPORT_PREFIX}/lib/ppmd.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
