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
# WAYLAND_CLIENT_FOUND
# WAYLAND_SERVER_FOUND
# WAYLAND_CLIENT_INCLUDE_DIR
# WAYLAND_SERVER_INCLUDE_DIR
# WAYLAND_CLIENT_LIBRARIES
# WAYLAND_SERVER_LIBRARIES
#

find_package(PkgConfig)
pkg_check_modules(PKG_WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(PKG_WAYLAND_SERVER REQUIRED wayland-server)

if (NOT PKG_WAYLAND_CLIENT_FOUND)
    message(FATAL_ERROR "No wayland-client")
endif(NOT PKG_WAYLAND_CLIENT_FOUND)

if (NOT PKG_WAYLAND_SERVER_FOUND)
    message(FATAL_ERROR "No wayland-server")
endif(NOT PKG_WAYLAND_SERVER_FOUND)

# find include paths
find_path(WAYLAND_CLIENT_INCLUDE_DIR wayland-client.h ${PKG_WAYLAND_CLIENT_INCLUDE_DIRS})
find_path(WAYLAND_SERVER_INCLUDE_DIR wayland-server.h ${PKG_WAYLAND_SERVER_INCLUDE_DIRS})

# find libs
find_library(WAYLAND_CLIENT_LIBRARIES NAMES wayland-client PATHS ${PKG_WAYLAND_CLIENT_LIBRARY_DIRS})
find_library(WAYLAND_SERVER_LIBRARIES NAMES wayland-server PATHS ${PKG_WAYLAND_SERVER_LIBRARY_DIRS})

# set _FOUND vars
set(WAYLAND_CLIENT_FOUND TRUE)
set(WAYLAND_SERVER_FOUND TRUE)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(WAYLAND DEFAULT_MSG WAYLAND_CLIENT_INCLUDE_DIR WAYLAND_CLIENT_LIBRARIES
    WAYLAND_SERVER_INCLUDE_DIR WAYLAND_SERVER_LIBRARIES)
mark_as_advanced(WAYLAND_CLIENT_INCLUDE_DIR WAYLAND_CLIENT_LIBRARIES
    WAYLAND_SERVER_INCLUDE_DIR WAYLAND_SERVER_LIBRARIES)
