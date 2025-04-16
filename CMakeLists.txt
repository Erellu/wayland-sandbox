cmake_minimum_required(VERSION 3.16)
include(cmake/warning_setup.cmake)
include(cmake/code_gen.cmake)



project(wayland-sandbox LANGUAGES CXX)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

find_package(wayland_client 1.10.0 REQUIRED)
find_package(wayland_server 1.10.0 REQUIRED)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

gen_xdg_shell()
gen_zxdg_decoration()

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

    zxdg/decoration.hpp
    zxdg/decoration.cpp

    zxdg/generated/decoration-protocol.cpp
    zxdg/generated/decoration-client-protocol.hpp
    display.cpp
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
