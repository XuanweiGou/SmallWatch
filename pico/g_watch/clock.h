#ifndef CLOCK_H
#define CLOCK_H

#include "lvgl.h"

// 表盘尺寸常量
#define CLOCK_RADIUS 120
#define CLOCK_CENTER_X 120
#define CLOCK_CENTER_Y 120

// 颜色常量
#define COLOR_ROSE_GOLD_LIGHT lv_color_make(247, 232, 227)  // #f7e8e3
#define COLOR_ROSE_GOLD_MID   lv_color_make(242, 220, 212)  // #f2dcd4
#define COLOR_ROSE_GOLD_DARK  lv_color_make(232, 206, 199)  // #e8cec7

// 金属银色系列
#define COLOR_SILVER_LIGHT    lv_color_make(232, 232, 232)  // #e8e8e8
#define COLOR_SILVER_MID      lv_color_make(153, 153, 153)  // #999999
#define COLOR_SILVER_DARK     lv_color_make(102, 102, 102)  // #666666

// 初始化时钟
void init_clock(void);

// 更新时钟显示
void update_clock(void);

// 绘制表盘
void draw_clock_face(void);

// 绘制指针
void draw_hands(uint8_t hours, uint8_t minutes, uint8_t seconds, uint16_t milliseconds);

// 绘制日期窗口
void draw_date_window(void);

#endif // CLOCK_H 