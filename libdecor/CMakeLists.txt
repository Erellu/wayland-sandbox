cmake_minimum_required(VERSION 3.16)
project(wayland-sandbox LANGUAGES C)


# find_package(wayland_client 1.10.0 REQUIRED)
# # find_package(dbus 1.0 REQUIRED)

# set(CMAKE_C_STANDARD 99)
# set(CMAKE_C_STANDARD_REQUIRED ON)

# include(CheckFunctionExists)

# set(meson_build_type "")

# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#     set(meson_build_type "debug")
# else()
#     set(meson_build_type "release")
# endif()

# execute_process(
#     WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
#     COMMAND meson --buildtype ${meson_build_type} -Dprefix=${CMAKE_INSTALL_PREFIX}/deps/libdecor
#     OUTPUT_VARIABLE FUBUKI_CODEGEN_STDOUT
#     RESULT_VARIABLE FUBUKI_CODE_GEN_SUCCESS
# )


file(READ ${CMAKE_CURRENT_LIST_DIR}/libdecor/config.h.meson LIBDECOR_CONFIG_H)
string(REPLACE "#mesondefine" "#cmakedefine" LIBDECOR_CONFIG_H "${LIBDECOR_CONFIG_H}")

    set(FIXED_INCLUDES "\#if defined(HAVE_MKOSTEMP)
    #include <stdlib.h>
#endif // defined(HAVE_MKOSTEMP)

#if defined(HAVE_POSIX_FALLOCATE)
    #include <fcntl.h>
#endif // defined(HAVE_POSIX_FALLOCATE)

#if defined(HAVE_MEMFD_CREATE)
    #include <sys/mman.h>
#endif // defined(HAVE_MEMFD_CREATE)

#if defined(HAVE_GETTID)
    #include <unistd.h>
#endif // defined(HAVE_GETTID)
        ")


include(CheckFunctionExists)
check_function_exists(mkostemp HAVE_MKOSTEMP)
check_function_exists(posix_fallocate HAVE_POSIX_FALLOCATE)
check_function_exists(memfd_create HAVE_MEMFD_CREATE)
check_function_exists(gettid HAVE_GETTID)
set(HAVE_XDG_SHELL_V6 TRUE) # Hopefully okay by now...

string(CONFIGURE "${LIBDECOR_CONFIG_H}" LIBDECOR_CONFIG_H)
set(LIBDECOR_CONFIG_H "${LIBDECOR_CONFIG_H} ${FIXED_INCLUDES}")

file(WRITE libdecor/src/config.h "${LIBDECOR_CONFIG_H}")

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMAND wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml libdecor/src/xdg-shell-protocol.c
    OUTPUT_VARIABLE FUBUKI_CODEGEN_STDOUT
    RESULT_VARIABLE FUBUKI_CODE_GEN_SUCCESS
)

if(FUBUKI_CODE_GEN_SUCCESS AND NOT FUBUKI_CODE_GEN_SUCCESS EQUAL 0)
    message(FATAL_ERROR "Codegen failed with\n*************************************\n ${FUBUKI_CODEGEN_STDOUT}\n*************************************")
endif()

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMAND wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml libdecor/src/xdg-shell-client-protocol.h
    OUTPUT_VARIABLE FUBUKI_CODEGEN_STDOUT
    RESULT_VARIABLE FUBUKI_CODE_GEN_SUCCESS
)

if(FUBUKI_CODE_GEN_SUCCESS AND NOT FUBUKI_CODE_GEN_SUCCESS EQUAL 0)
    message(FATAL_ERROR "Codegen failed with\n*************************************\n ${FUBUKI_CODEGEN_STDOUT}\n*************************************")
endif()

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMAND wayland-scanner private-code /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml libdecor/src/xdg-decoration-protocol.c
    OUTPUT_VARIABLE FUBUKI_CODEGEN_STDOUT
    RESULT_VARIABLE FUBUKI_CODE_GEN_SUCCESS
)

if(FUBUKI_CODE_GEN_SUCCESS AND NOT FUBUKI_CODE_GEN_SUCCESS EQUAL 0)
    message(FATAL_ERROR "Codegen failed with\n*************************************\n ${FUBUKI_CODEGEN_STDOUT}\n*************************************")
endif()

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    COMMAND wayland-scanner client-header /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml libdecor/src/xdg-decoration-client-protocol.h
    OUTPUT_VARIABLE FUBUKI_CODEGEN_STDOUT
    RESULT_VARIABLE FUBUKI_CODE_GEN_SUCCESS
)

if(FUBUKI_CODE_GEN_SUCCESS AND NOT FUBUKI_CODE_GEN_SUCCESS EQUAL 0)
    message(FATAL_ERROR "Codegen failed with\n*************************************\n ${FUBUKI_CODEGEN_STDOUT}\n*************************************")
endif()

add_library(decor
    STATIC
    libdecor/src/config.h
    libdecor/src/desktop-settings.c
    libdecor/src/desktop-settings.h
    libdecor/src/libdecor-fallback.c
    libdecor/src/libdecor-fallback.h
    libdecor/src/libdecor-plugin.h
    libdecor/src/libdecor.c
    libdecor/src/libdecor.h
    libdecor/src/os-compatibility.c
    libdecor/src/os-compatibility.h
    libdecor/src/xdg-shell-protocol.c
    libdecor/src/xdg-shell-client-protocol.h
    libdecor/src/xdg-decoration-protocol.c
    libdecor/src/xdg-decoration-client-protocol.h
    libdecor/src/plugins/cairo/libdecor-cairo.c
    libdecor/src/plugins/common/libdecor-cairo-blur.c
    libdecor/src/plugins/common/libdecor-cairo-blur.h
    libdecor/src/plugins/dummy/libdecor-dummy.c
    libdecor/src/utils.h
)

target_include_directories(
    decor
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/libdecor/src/>
)

target_link_libraries(decor PRIVATE ${dbus_LIBRARIES} ${CAIRO_LIBRARIES} ${PANGOCAIRO_LIBRARIES})
target_compile_definitions(decor PRIVATE _GNU_SOURCE)
# target_compile_definitions(decor PRIVATE -DLIBDECOR_PLUGIN_API_VERSION=1)

