# - Try to Find libdecor-0
#
# Will be defined:
# LIBDECOR_0_FOUND
# LIBDECOR_0_INCLUDE_DIR
# LIBDECOR_0_LIBRARIES
#

find_package(PkgConfig)
pkg_check_modules(PKG_LIBDECOR_0 REQUIRED libdecor-0)

if (NOT PKG_LIBDECOR_0_FOUND)
    message(FATAL_ERROR "No libdecor")
endif(NOT PKG_LIBDECOR_0_FOUND)

find_path(libdecor_0_INCLUDE_DIR libdecor.h ${PKG_LIBDECOR_0_INCLUDE_DIRS})
find_library(libdecor_0_LIBRARIES NAMES "libdecor-0.so" PATHS ${PKG_LIBDECOR_0_LIBRARY_DIRS})
set(libdecor_0_FOUND TRUE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(libdecor_0 DEFAULT_MSG libdecor_0_INCLUDE_DIR libdecor_0_LIBRARIES)
mark_as_advanced(libdecor_0_INCLUDE_DIR libdecor_0_LIBRARIES)
