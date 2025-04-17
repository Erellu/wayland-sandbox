# FindPango.cmake
# <https://github.com/nemequ/gnome-cmake>
#
# CMake support for Pango.
#
# License:
#
#   Copyright (c) 2016 Evan Nemerson <evan@nemerson.com>
#
#   Permission is hereby granted, free of charge, to any person
#   obtaining a copy of this software and associated documentation
#   files (the "Software"), to deal in the Software without
#   restriction, including without limitation the rights to use, copy,
#   modify, merge, publish, distribute, sublicense, and/or sell copies
#   of the Software, and to permit persons to whom the Software is
#   furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be
#   included in all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
#   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
#   HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
#   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
#   DEALINGS IN THE SOFTWARE.


find_package(PkgConfig)

set(Pango_DEPS
  GLib HarfBuzz)

if(PKG_CONFIG_FOUND)
  pkg_search_module(Pango_PKG pango)
endif()

find_library(Pango_LIBRARIES pango-1.0 HINTS ${Pango_PKG_LIBRARY_DIRS})

if(Pango_LIBRARIES AND NOT Pango_FOUND)
  add_library(pango-1.0 SHARED IMPORTED)
  set_property(TARGET pango-1.0 PROPERTY IMPORTED_LOCATION "${Pango_LIBRARIES}")
  set_property(TARGET pango-1.0 PROPERTY INTERFACE_COMPILE_OPTIONS "${Pango_PKG_CFLAGS_OTHER}")

  find_path(Pango_INCLUDE_DIR "pango.h"
            HINTS ${Pango_PKG_INCLUDE_DIRS}
            PATH_SUFFIXES pango)

  if(Pango_INCLUDE_DIR)
    file(STRINGS "${Pango_INCLUDE_DIR}/pango/pango-features.h" Pango_MAJOR_VERSION REGEX "^#define PANGO_VERSION_MAJOR +\\(?([0-9]+)\\)?$")
    string(REGEX REPLACE "^#define PANGO_VERSION_MAJOR \\(?([0-9]+)\\)?" "\\1" Pango_MAJOR_VERSION "${Pango_MAJOR_VERSION}")
    file(STRINGS "${Pango_INCLUDE_DIR}/pango/pango-features.h" Pango_MINOR_VERSION REGEX "^#define PANGO_VERSION_MINOR +\\(?([0-9]+)\\)?$")
    string(REGEX REPLACE "^#define PANGO_VERSION_MINOR \\(?([0-9]+)\\)?" "\\1" Pango_MINOR_VERSION "${Pango_MINOR_VERSION}")
    file(STRINGS "${Pango_INCLUDE_DIR}/pango/pango-features.h" Pango_MICRO_VERSION REGEX "^#define PANGO_VERSION_MICRO +\\(?([0-9]+)\\)?$")
    string(REGEX REPLACE "^#define PANGO_VERSION_MICRO \\(?([0-9]+)\\)?" "\\1" Pango_MICRO_VERSION "${Pango_MICRO_VERSION}")
    set(Pango_VERSION "${Pango_MAJOR_VERSION}.${Pango_MINOR_VERSION}.${Pango_MICRO_VERSION}")
    unset(Pango_MAJOR_VERSION)
    unset(Pango_MINOR_VERSION)
    unset(Pango_MICRO_VERSION)

    list(APPEND Pango_INCLUDE_DIRS ${Pango_INCLUDE_DIR})
    set_property(TARGET pango-1.0 PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${Pango_INCLUDE_DIR}")
  endif()
endif()
# message(FATAL_ERROR ">>>>>>>>>>>>>>>>> ${Pango_INCLUDE_DIRS} <<<<<<<<<<<<<<<<")

set(Pango_DEPS_FOUND_VARS)

foreach(pango_dep ${Pango_DEPS})

  # find_package(${pango_dep})

  # list(APPEND Pango_DEPS_FOUND_VARS "${pango_dep}_FOUND")
  # list(APPEND Pango_INCLUDE_DIRS ${${pango_dep}_INCLUDE_DIRS})

  set_property (TARGET "pango-1.0" APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${${pango_dep}_LIBRARIES}")
  message(WARNING "${${pango_dep}_LIBRARIES}")
endforeach(pango_dep)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Pango
    REQUIRED_VARS
      Pango_LIBRARIES
      Pango_INCLUDE_DIRS
      ${Pango_DEPS_FOUND_VARS}
    VERSION_VAR
      Pango_VERSION)

unset(Pango_DEPS_FOUND_VARS)

# find_package(PkgConfig)
# pkg_check_modules(PC_PANGO QUIET pango)

# find_path(PANGO_INCLUDE_DIR
#           NAMES pango/pango.h
#           HINTS ${PC_PANGO_INCLUDE_DIRS}
#           PATH_SUFFIXES pango-1.0)

# find_library(PANGO_LIBRARY
#              NAMES pango-1.0
#              HINTS ${PC_PANGO_LIBRARY_DIRS}
# )

# set(PANGO_LIBRARIES ${PANGO_LIBRARY})

# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args(Pango DEFAULT_MSG  PANGO_LIBRARIES PANGO_INCLUDE_DIR)

# mark_as_advanced(PANGO_INCLUDE_DIR PANGO_LIBRARIES PANGO_LIBRARY)
