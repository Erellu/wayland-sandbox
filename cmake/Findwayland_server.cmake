# https://github.com/rpavlik/cmake-modules/blob/main/FindWayland.cmake

# - Try to Find Wayland
#
# Copyright 2015, Vlad Zagorodniy
#
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# SPDX-License-Identifier: BSL-1.0
#
# Will be defined:
# WAYLAND_FOUND
# WAYLAND_SERVER_FOUND
# WAYLAND_SERVER_INCLUDE_DIR
# WAYLAND_SERVER_LIBRARIES
#

find_package(PkgConfig)
pkg_check_modules(PKG_WAYLAND_SERVER REQUIRED wayland-server)

if (NOT PKG_WAYLAND_SERVER_FOUND)
    message(FATAL_ERROR "No wayland-server")
endif(NOT PKG_WAYLAND_SERVER_FOUND)

find_path(wayland_server_INCLUDE_DIR wayland-server.h ${PKG_WAYLAND_SERVER_INCLUDE_DIRS})
find_library(wayland_server_LIBRARIES NAMES wayland-server PATHS ${PKG_WAYLAND_SERVER_LIBRARY_DIRS})
set(wayland_server_FOUND TRUE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(wayland_server DEFAULT_MSG wayland_server_INCLUDE_DIR wayland_server_LIBRARIES)
mark_as_advanced(wayland_server_INCLUDE_DIR wayland_server_LIBRARIES)
