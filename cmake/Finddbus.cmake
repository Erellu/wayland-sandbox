# - Try to Find libdecor-0
# Will be defined:
# dbus_FOUND
# dbus_INCLUDE_DIR
# dbus_LIBRARIES
#

find_package(PkgConfig)
pkg_check_modules(PKG_dbus REQUIRED dbus-1)

if (NOT PKG_dbus_FOUND)
    message(FATAL_ERROR "No dbus")
endif(NOT PKG_dbus_FOUND)

find_path(dbus_INCLUDE_DIR  dbus-1.0/dbus/dbus.h ${PKG_DBUS_INCLUDE_DIRS})
find_library(dbus_LIBRARIES NAMES dbus-1 PATHS ${PKG_DBUS_LIBRARY_DIRS})
set(dbus_FOUND TRUE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dbus DEFAULT_MSG dbus_INCLUDE_DIR dbus_LIBRARIES)
mark_as_advanced(dbus_INCLUDE_DIR dbus_LIBRARIES)

