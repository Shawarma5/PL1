# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")


include(pico_sdk_import.cmake)


project(rover C CXX ASM)

pico_sdk_init()

add_executable(rover
        rover.cpp
        overcurrent.cpp
        tape.cpp
)

pico_set_program_name(rover "rover")
pico_set_program_version(rover "0.2")


pico_enable_stdio_uart(rover 0)
pico_enable_stdio_usb(rover 1)


target_link_libraries(rover
        pico_stdlib
        hardware_pwm
        hardware_irq
        hardware_clocks
)

target_include_directories(rover PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(rover)
