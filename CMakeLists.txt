cmake_minimum_required(VERSION 3.16)
include(cmake/warning_setup.cmake)

project(wayland-sandbox LANGUAGES CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(wayland_client 1.10.0 REQUIRED)
find_package(wayland_server 1.10.0 REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/xdg/generated/shell-protocol.cpp")
    message(WARNING "GENERATING cpp")
    execute_process(
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        COMMAND wayland-scanner private-code /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg/generated/shell-protocol.cpp
        OUTPUT_VARIABLE FUBUKI_CODEGEN_STDOUT
        RESULT_VARIABLE FUBUKI_CODE_GEN_SUCCESS
    )

    if(FUBUKI_CODE_GEN_SUCCESS AND NOT FUBUKI_CODE_GEN_SUCCESS EQUAL 0)
        message(FATAL_ERROR "Codegen failed with\n*************************************\n ${FUBUKI_CODEGEN_STDOUT}\n*************************************")
    endif()
endif()

if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/xdg/generated/shell-client-protocol.hpp")
    execute_process(
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        COMMAND wayland-scanner client-header /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml xdg/generated/shell-client-protocol.hpp
        OUTPUT_VARIABLE FUBUKI_CODEGEN_STDOUT
        RESULT_VARIABLE FUBUKI_CODE_GEN_SUCCESS
    )

    if(FUBUKI_CODE_GEN_SUCCESS AND NOT FUBUKI_CODE_GEN_SUCCESS EQUAL 0)
        message(FATAL_ERROR "Codegen failed with\n*************************************\n ${FUBUKI_CODEGEN_STDOUT}\n*************************************")
    endif()
endif()


add_executable(wayland-sandbox
    main.cpp

    display.hpp

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
)

include(CheckFunctionExists)
check_function_exists(memfd_create FUBUKI_HAS_MEMFD_CREATE)
if (FUBUKI_HAS_MEMFD_CREATE)
    target_compile_definitions(wayland-sandbox PRIVATE FUBUKI_HAS_MEMFD_CREATE)
endif()

# target_compile_definitions(wayland-sandbox PRIVATE _POSIX_C_SOURCE=200112L)
target_link_libraries(wayland-sandbox PRIVATE ${wayland_client_LIBRARIES})
target_link_libraries(wayland-sandbox PRIVATE rt)
target_compile_options(wayland-sandbox PRIVATE ${FUBUKI_WARNINGS})

include(GNUInstallDirs)
install(TARGETS wayland-sandbox
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
