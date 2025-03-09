#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

// 显示设置
#define LV_HOR_RES_MAX          240
#define LV_VER_RES_MAX          240
#define LV_COLOR_DEPTH          16
#define LV_COLOR_16_SWAP        0

// 内存设置
#define LV_MEM_CUSTOM           0
#define LV_MEM_SIZE            (32U * 1024U)
#define LV_MEM_ATTR
#define LV_MEM_ADR             0

// HAL设置
#define LV_TICK_CUSTOM         0
#define LV_DPI_DEF             130
#define LV_DISP_DEF_REFR_PERIOD 30

// 功能配置
#define LV_USE_PERF_MONITOR    0
#define LV_USE_MEM_MONITOR     0
#define LV_USE_LOG             0
#define LV_USE_ASSERT_NULL     1
#define LV_USE_ASSERT_MEM      1
#define LV_USE_ASSERT_STR      1
#define LV_USE_ASSERT_OBJ      1
#define LV_USE_ASSERT_STYLE    1

// 字体设置
#define LV_FONT_MONTSERRAT_12  1
#define LV_FONT_MONTSERRAT_14  1
#define LV_FONT_MONTSERRAT_16  1
#define LV_FONT_DEFAULT        &lv_font_montserrat_14

#endif // LV_CONF_H 