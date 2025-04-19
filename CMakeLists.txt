cmake_minimum_required(VERSION 3.18)
include(cmake/warning_setup.cmake)
include(cmake/code_gen.cmake)

#------------------------------------------------------------------------------
# CMake options

set(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING "Build type (Debug, Release, RelWithDebInfo, MinSizeRel)")
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;Release;RelWithDebInfo;MinSizeRel")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" AND NOT CMAKE_BUILD_TYPE STREQUAL "Release" AND NOT CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo" AND NOT CMAKE_BUILD_TYPE STREQUAL "MinSizeRel" )
    message(FATAL_ERROR "Invalid value for CMAKE_BUILD_TYPE: '${CMAKE_BUILD_TYPE}' (required one of 'Debug', 'Release', 'RelWithDebInfo', 'MinSizeRel')")
endif()

#------------------------------------------------------------------------------

project(wayland-sandbox LANGUAGES CXX C)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# find_package(harfbuzz REQUIRED)
# message(FATAL_ERROR ${HarfBuzz_LIBRARIES})

find_package(wayland_client 1.10.0 REQUIRED)
# find_package(dbus 1.0 REQUIRED)
# find_package(Glib REQUIRED)
# find_package(Cairo REQUIRED)
# find_package(Pango REQUIRED)
# find_package(PangoCairo REQUIRED)


# find_package(wayland_server 1.10.0 REQUIRED)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

gen_xdg_shell()
gen_zxdg_decoration()

add_executable(wayland-sandbox
    main.cpp

    display.hpp
    display.cpp

    decoration.hpp

    file_descriptor.hpp
    file_descriptor.cpp

    registry.hpp

    scoped_mmap.hpp

    screen.hpp
    screen.cpp

    screen_properties.hpp

    shm_pool.hpp
    shm_pool.cpp
    shm_buffer.hpp
    shm_buffer.cpp

    test.hpp
    test.cpp

    types.hpp

    window.hpp
    window.cpp

    window_info.hpp

    xdg/surface.hpp
    xdg/surface.cpp

    xdg/toplevel.hpp
    xdg/toplevel.cpp

    xdg/wm_base.hpp
    xdg/wm_base.cpp

    xdg/generated/shell-protocol.cpp
    xdg/generated/shell-client-protocol.hpp

    zxdg/decoration.hpp
    zxdg/decoration.cpp

    zxdg/generated/decoration-protocol.cpp
    zxdg/generated/decoration-client-protocol.hpp
    seat.hpp
    keyboard.hpp
    pointer.hpp

    # decor/context.hpp
    # decor/context.cpp
    # decor/frame.hpp
    # decor/frame.cpp
)

#------------------------------------------------------------------------------
# Build libdecor
# Unfortunately, libdecor provided by apt doesn't have libdecor_frame_set_user_data, which is required to handle move assignment and move construction
# of wrapper objects. So we're forced to build it ourselves
#find_package(libdecor_0 REQUIRED)
# FIXME: doesn't work

# set(meson_build_type "")

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     set(meson_build_type "debug")
# else()
#     set(meson_build_type "release")
# endif()

# make_directory(${CMAKE_CURRENT_BINARY_DIR}/libdecor/install)

# set(libdecor_src_dir ${CMAKE_CURRENT_LIST_DIR}/libdecor/libdecor)
# set(meson_build_dir "${CMAKE_CURRENT_BINARY_DIR}/libdecor/build")
# set(meson_prefix "-Dprefix=${CMAKE_INSTALL_PREFIX}/deps/libdecor")
# set(meson_options "-Dgtk=disabled")


# execute_process(
#     WORKING_DIRECTORY ${libdecor_src_dir}
#     COMMAND meson ${meson_build_dir}
#             --buildtype ${meson_build_type}
#             ${meson_prefix}
#             ${meson_options}
#     COMMAND meson compile
#             \-C ${meson_build_dir}
#     COMMAND meson install
#             \-C ${meson_build_dir}
#     OUTPUT_VARIABLE FUBUKI_LIBDECOR_MESON_STDOUT
#     RESULT_VARIABLE FUBUKI_LIBDECOR_SUCCESS
#     COMMAND_ECHO STDERR
# )

# if(FUBUKI_LIBDECOR_SUCCESS AND NOT FUBUKI_LIBDECOR_SUCCESS EQUAL 0)
#     message(FATAL_ERROR "Failed to build libdecor. Process exited with code ${FUBUKI_LIBDECOR_SUCCESS}:\n*************************************\n ${FUBUKI_LIBDECOR_MESON_STDOUT}\n*************************************")
# endif()

# if(FUBUKI_LIBDECOR_SUCCESS AND NOT FUBUKI_LIBDECOR_SUCCESS EQUAL 0)
#     message(FATAL_ERROR "Failed to install libdecor. Process exited with code  ${FUBUKI_LIBDECOR_SUCCESS}:\n*************************************\n ${FUBUKI_LIBDECOR_MESON_STDOUT}\n*************************************")
# endif()

# add_library(decor SHARED IMPORTED)
# set_target_properties(decor PROPERTIES IMPORTED_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/libdecor/src/libdecor-0.so)

#------------------------------------------------------------------------------

include(CheckFunctionExists)
check_function_exists(memfd_create FUBUKI_HAS_MEMFD_CREATE)
if (FUBUKI_HAS_MEMFD_CREATE)
    target_compile_definitions(wayland-sandbox PRIVATE FUBUKI_HAS_MEMFD_CREATE)
endif()

# target_compile_definitions(wayland-sandbox PRIVATE _POSIX_C_SOURCE=200112L)
target_link_libraries(wayland-sandbox PRIVATE ${wayland_client_LIBRARIES} )
target_link_libraries(wayland-sandbox PRIVATE rt)
# target_link_libraries(wayland-sandbox PRIVATE decor)
target_compile_options(wayland-sandbox PRIVATE ${FUBUKI_WARNINGS})

include(GNUInstallDirs)
install(TARGETS wayland-sandbox
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
