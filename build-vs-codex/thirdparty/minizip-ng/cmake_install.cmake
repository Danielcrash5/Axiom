# Install script for directory: F:/projects/Axiom/thirdparty/minizip-ng

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files/Axiom")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/Debug/minizip.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/Release/minizip.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/MinSizeRel/minizip.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/RelWithDebInfo/minizip.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/Debug/ppmd.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/Release/ppmd.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/MinSizeRel/ppmd.lib")
  elseif(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/RelWithDebInfo/ppmd.lib")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip.cmake"
         "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/CMakeFiles/Export/4fe77da0f4d2c94dd906efce3aa1c0aa/minizip.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip/minizip.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/CMakeFiles/Export/4fe77da0f4d2c94dd906efce3aa1c0aa/minizip.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/CMakeFiles/Export/4fe77da0f4d2c94dd906efce3aa1c0aa/minizip-debug.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/CMakeFiles/Export/4fe77da0f4d2c94dd906efce3aa1c0aa/minizip-minsizerel.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/CMakeFiles/Export/4fe77da0f4d2c94dd906efce3aa1c0aa/minizip-relwithdebinfo.cmake")
  endif()
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/CMakeFiles/Export/4fe77da0f4d2c94dd906efce3aa1c0aa/minizip-release.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/minizip" TYPE FILE FILES
    "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/minizip-config-version.cmake"
    "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/minizip-config.cmake"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/minizip" TYPE FILE FILES
    "F:/projects/Axiom/thirdparty/minizip-ng/mz.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_os.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_crypt.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_strm.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_strm_buf.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_strm_mem.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_strm_split.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_strm_os.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_zip.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_zip_rw.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_strm_ppmd.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_strm_pkcrypt.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/mz_strm_wzaes.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/compat/ioapi.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/compat/unzip.h"
    "F:/projects/Axiom/thirdparty/minizip-ng/compat/zip.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/minizip.pc")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "F:/projects/Axiom/build-vs-codex/thirdparty/minizip-ng/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
