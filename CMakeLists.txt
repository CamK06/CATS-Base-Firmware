# Set minimum required version of CMake
cmake_minimum_required(VERSION 3.12)

# Include build functions from Pico SDK
include($ENV{PICO_SDK_PATH}/external/pico_sdk_import.cmake)

if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  set(GIT_COMMIT "unknown")
endif()
add_definitions("-DGIT_COMMIT=\"${GIT_COMMIT}\"")

add_definitions("-DUSBD_VID=0xdead")
add_definitions("-DUSBD_PID=0xbeef")
add_definitions("-DUSBD_PRODUCT=\"CATS Transceiver\"")
add_definitions("-DUSBD_MANUFACTURER=\"VE3KCN\"")

# Set name of project (as PROJECT_NAME) and C/C   standards
project(catsbase C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Creates a pico-sdk subdirectory in our project for the libraries
pico_sdk_init()

file(GLOB SOURCE src/*.c src/drivers/**/*.c)

# Tell CMake where to find the executable source file
add_executable(${PROJECT_NAME} 
    ${SOURCE}
)

# Create map/bin/hex/uf2 files
pico_add_extra_outputs(${PROJECT_NAME})

# Ensure tinyusb can see tusb_config.h
target_include_directories(catsbase PUBLIC src src/drivers/common)

# Link to pico_stdlib (gpio, time, etc. functions)
target_link_libraries(${PROJECT_NAME} 
    PUBLIC pico_stdlib pico_unique_id hardware_flash hardware_sync hardware_irq hardware_spi
)

# Enable usb output, disable uart output
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)