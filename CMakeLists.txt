cmake_minimum_required(VERSION 3.12)

set(NAME pico-explorer-clock)

include(pico_sdk_import.cmake)
include(pimoroni_pico_import.cmake)

# Gooey boilerplate
project(${NAME} C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

# Add your source files
add_executable(${NAME}
    src/main.cpp
)

# Include required libraries
include(common/pimoroni_i2c)
include(common/pimoroni_bus)
include(libraries/pico_explorer/pico_explorer)
include(drivers/button/button)

# Link required libraries
target_link_libraries(
    ${NAME}
    pico_stdlib
    pico_explorer
    pico_graphics
    st7789
    button
    pico_time
)

# create map/bin/hex file etc.
pico_add_extra_outputs(${NAME})

# Set up files for the release packages
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${NAME}.uf2
    ${CMAKE_CURRENT_LIST_DIR}/README.md
    DESTINATION .
)

set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
set(CPACK_GENERATOR "ZIP" "TGZ")
include(CPack)

# Disable USB serial and enable UART serial
pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 1)
