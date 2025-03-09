cmake_minimum_required(VERSION 3.13)

# 设置工具链路径和配置
set(ARM_TOOLCHAIN_DIR "D:/GNUArm")
set(PICO_TOOLCHAIN_PATH ${ARM_TOOLCHAIN_DIR})

# 设置编译器
set(CMAKE_C_COMPILER "${ARM_TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc.exe")
set(CMAKE_CXX_COMPILER "${ARM_TOOLCHAIN_DIR}/bin/arm-none-eabi-g++.exe")
set(CMAKE_ASM_COMPILER "${ARM_TOOLCHAIN_DIR}/bin/arm-none-eabi-gcc.exe")

# 导入Pico SDK
include(pico_sdk_import.cmake)

# 设置Pico平台
set(PICO_PLATFORM rp2040)
set(PICO_BOARD pico)
set(PICO_COMPILER pico_arm_gcc)

# 项目名称和语言设置
project(yongqigou_watch C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# 初始化Pico SDK
pico_sdk_init()

# LVGL配置
set(LV_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h")

# 设置LVGL源文件
file(GLOB_RECURSE LVGL_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/lvgl/src/*.c"
)

# 创建LVGL库
add_library(lvgl STATIC ${LVGL_SOURCES})

# 配置LVGL
target_compile_definitions(lvgl PUBLIC
    LV_CONF_INCLUDE_SIMPLE
    LV_CONF_PATH="${LV_CONF_PATH}"
    LV_USE_DRAW_SW=1
    LV_DRAW_SW_ASM=0
    LV_USE_DRAW_HW=0
    LV_USE_GPU=0
)

# 设置LVGL包含目录
target_include_directories(lvgl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/src
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/src/display
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/src/misc
)

# 添加源文件
add_executable(${PROJECT_NAME}
    main.c
    clock.c
    lcd_driver.c
)

# 添加头文件路径
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl
    ${CMAKE_CURRENT_SOURCE_DIR}/lvgl/src
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    hardware_spi
    hardware_rtc
    lvgl
    pico_time
    pico_float
    m
)

# 启用USB输出
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

# 创建UF2文件
pico_add_extra_outputs(${PROJECT_NAME}) 