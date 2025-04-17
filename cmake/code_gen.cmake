function(gen_xdg_shell)
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/xdg/generated/shell-protocol.cpp")
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
endfunction()

function(gen_zxdg_decoration)
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/zxdg/generated/decoration-protocol.cpp")
        execute_process(
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            COMMAND wayland-scanner private-code /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml zxdg/generated/decoration-protocol.cpp
            OUTPUT_VARIABLE FUBUKI_CODEGEN_STDOUT
            RESULT_VARIABLE FUBUKI_CODE_GEN_SUCCESS
        )

        if(FUBUKI_CODE_GEN_SUCCESS AND NOT FUBUKI_CODE_GEN_SUCCESS EQUAL 0)
            message(FATAL_ERROR "Codegen failed with\n*************************************\n ${FUBUKI_CODEGEN_STDOUT}\n*************************************")
        endif()
    endif()

    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/xdgz/generated/decoration-client-protocol.hpp")
        execute_process(
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
            COMMAND wayland-scanner client-header /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml zxdg/generated/decoration-client-protocol.hpp
            OUTPUT_VARIABLE FUBUKI_CODEGEN_STDOUT
            RESULT_VARIABLE FUBUKI_CODE_GEN_SUCCESS
        )

        if(FUBUKI_CODE_GEN_SUCCESS AND NOT FUBUKI_CODE_GEN_SUCCESS EQUAL 0)
            message(FATAL_ERROR "Codegen failed with\n*************************************\n ${FUBUKI_CODEGEN_STDOUT}\n*************************************")
        endif()
    endif()
endfunction()

