#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* 基本配置 */
#define LV_USE_DEV_VERSION 0
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0
#define LV_COLOR_SCREEN_TRANSP 0

/* 内存管理 */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (64U * 1024U)
#define LV_MEM_POOL_INCLUDE <stdlib.h>
#define LV_MEM_POOL_ALLOC malloc
#define LV_MEM_POOL_FREE free

/* HAL 设置 */
#define LV_TICK_CUSTOM 0
#define LV_DPI_DEF 130

/* 绘图功能 */
#define LV_USE_DRAW_SW 1
#define LV_USE_DRAW_SDL 1
#define LV_USE_GPU_SDL 1

/* 文件系统 */
#define LV_USE_FS_IF 1
#define LV_FS_STDIO_LETTER 'A'

/* 其他功能 */
#define LV_USE_LOG 1
#define LV_USE_PERF_MONITOR 1
#define LV_USE_MEM_MONITOR 1
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MEM 1
#define LV_USE_ASSERT_STR 1
#define LV_USE_ASSERT_OBJ 1
#define LV_USE_ASSERT_STYLE 1

/* 小部件 */
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_BTN 1
#define LV_USE_LABEL 1
#define LV_USE_LINE 1
#define LV_USE_TABLE 1
#define LV_USE_CHECKBOX 1
#define LV_USE_IMGBTN 1
#define LV_USE_SLIDER 1

/* 主题 */
#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0
#define LV_THEME_DEFAULT_GROW 1

#endif /* LV_CONF_H */ 