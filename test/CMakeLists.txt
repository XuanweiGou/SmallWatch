cmake_minimum_required(VERSION 3.10)
project(lvgl_simulator)

# SDL2 路径
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/libs/SDL2")
set(SDL2_INCLUDE_DIRS "${SDL2_DIR}/include")
set(SDL2_LIBRARIES "${SDL2_DIR}/lib/x64/SDL2.lib")

# 设置 LVGL 配置文件路径
set(LV_CONF_PATH "${CMAKE_SOURCE_DIR}/../pico/yongqigou_watch/lv_conf.h")
add_definitions(-DLV_CONF_INCLUDE_SIMPLE)
add_definitions(-DLV_CONF_PATH="${LV_CONF_PATH}")

# 添加 LVGL
add_subdirectory(libs/lvgl)

# 创建可执行文件
add_executable(simulator 
    "${CMAKE_SOURCE_DIR}/simulator.c"
)

# 包含目录
target_include_directories(simulator PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../pico/yongqigou_watch
    ${SDL2_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/libs
    ${CMAKE_SOURCE_DIR}/libs/lvgl
    ${CMAKE_SOURCE_DIR}/libs/lv_drivers
)

# 链接库
target_link_libraries(simulator PRIVATE
    lvgl
    ${SDL2_LIBRARIES}
)

# 复制 SDL2.dll 到输出目录
add_custom_command(TARGET simulator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${SDL2_DIR}/lib/x64/SDL2.dll"
    $<TARGET_FILE_DIR:simulator>
)

# 添加 lv_drivers 配置
target_compile_definitions(simulator PRIVATE
    MONITOR_SDL=1
    USE_SDL=1
    LV_USE_SDL=1
) 