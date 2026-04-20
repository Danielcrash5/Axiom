# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "F:/projects/Axiom/thirdparty/minizip-ng/third_party/ppmd")
  file(MAKE_DIRECTORY "F:/projects/Axiom/thirdparty/minizip-ng/third_party/ppmd")
endif()
file(MAKE_DIRECTORY
  "F:/projects/Axiom/build-vs-codex/_deps/ppmd-build"
  "F:/projects/Axiom/build-vs-codex/_deps/ppmd-subbuild/ppmd-populate-prefix"
  "F:/projects/Axiom/build-vs-codex/_deps/ppmd-subbuild/ppmd-populate-prefix/tmp"
  "F:/projects/Axiom/build-vs-codex/_deps/ppmd-subbuild/ppmd-populate-prefix/src/ppmd-populate-stamp"
  "F:/projects/Axiom/build-vs-codex/_deps/ppmd-subbuild/ppmd-populate-prefix/src"
  "F:/projects/Axiom/build-vs-codex/_deps/ppmd-subbuild/ppmd-populate-prefix/src/ppmd-populate-stamp"
)

set(configSubDirs Debug)
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "F:/projects/Axiom/build-vs-codex/_deps/ppmd-subbuild/ppmd-populate-prefix/src/ppmd-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "F:/projects/Axiom/build-vs-codex/_deps/ppmd-subbuild/ppmd-populate-prefix/src/ppmd-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
