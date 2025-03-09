#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/rtc.h"
#include "pico/time.h"

// LVGL 头文件
#include "lvgl.h"
#include "src/display/lv_display.h"
#include "src/display/lv_display_private.h"
#include "src/core/lv_obj.h"
#include "src/draw/lv_draw.h"

// 本地头文件
#include "lcd_driver.h"
#include "clock.h"

#define DISP_BUF_SIZE (LCD_WIDTH * 10)

// 显示缓冲区
static lv_color_t buf1[DISP_BUF_SIZE];
static lv_disp_t * disp;

// 创建表盘样式
static lv_style_t style_clock;
static lv_obj_t *clock_obj;
static lv_obj_t *date_label;

// 添加指针样式
static lv_style_t style_hour_hand;
static lv_style_t style_min_hand;
static lv_style_t style_sec_hand;
static lv_obj_t *hour_hand;
static lv_obj_t *min_hand;
static lv_obj_t *sec_hand;

// 创建金属质感渐变
static void create_metallic_style(lv_style_t *style) {
    lv_style_init(style);
    
    // 设置背景渐变
    lv_style_set_bg_color(style, lv_color_hex(0xf7e8e3));  // 玫瑰金色
    lv_style_set_bg_grad_color(style, lv_color_hex(0xe8cec7));
    lv_style_set_bg_grad_dir(style, LV_GRAD_DIR_NONE);
    
    // 添加边框效果
    lv_style_set_border_width(style, 2);
    lv_style_set_border_color(style, lv_color_hex(0xd4b5ac));
    lv_style_set_radius(style, LV_RADIUS_CIRCLE);
    
    // 添加阴影效果
    lv_style_set_shadow_width(style, 20);
    lv_style_set_shadow_color(style, lv_color_hex(0x666666));
    lv_style_set_shadow_opa(style, LV_OPA_30);
}

// 显示刷新回调
static void disp_flush(lv_display_t * disp_drv, const lv_area_t * area, uint8_t * px_map)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    lcd_set_window(area->x1, area->y1, area->x2, area->y2);
    lcd_write_pixels((uint16_t*)px_map, w * h);

    lv_display_flush_ready(disp_drv);
}

// 添加波纹效果
static void draw_ripple_effect(lv_obj_t *obj, lv_draw_ctx_t *draw_ctx) {
    lv_area_t area;
    lv_obj_get_coords(obj, &area);
    int32_t radius = (area.x2 - area.x1) / 2;
    int32_t cx = area.x1 + radius;
    int32_t cy = area.y1 + radius;

    // 绘制4层波纹
    for(int i = 0; i < 4; i++) {
        float progress = i / 4.0f;
        uint32_t current_radius = radius * (0.45f + progress * 0.5f);
        
        lv_draw_arc_dsc_t arc_dsc;
        lv_draw_arc_dsc_init(&arc_dsc);
        arc_dsc.color = lv_color_hex(0x666666);
        arc_dsc.width = 2;
        arc_dsc.opa = (1.0f - progress) * 50;  // 渐变透明度
        
        lv_draw_arc(draw_ctx, &arc_dsc, cx, cy, current_radius, 0, 360);
    }
}

// 更新绘制表盘函数
static void draw_clock_face(lv_obj_t *obj, lv_event_t *e) {
    lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
    
    // 绘制波纹效果
    draw_ripple_effect(obj, draw_ctx);
    
    // 绘制品牌名称
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = lv_color_hex(0xb76e5d);
    label_dsc.font = &lv_font_montserrat_16;  // 使用合适的字体
    
    lv_point_t label_pos;
    label_pos.x = obj->coords.x1 + (obj->coords.x2 - obj->coords.x1) / 2;
    label_pos.y = obj->coords.y1 + (obj->coords.y2 - obj->coords.y1) / 3;
    
    lv_draw_label(draw_ctx, &label_dsc, &label_pos, "YongqiGou", NULL);

    // 绘制刻度 (保持现有代码)
    lv_draw_ctx_t *draw_ctx = lv_event_get_draw_ctx(e);
    lv_area_t area;
    lv_obj_get_coords(obj, &area);
    
    // 绘制主刻度
    for(int i = 0; i < 12; i++) {
        float angle = i * 30 * M_PI / 180;
        int32_t x1 = area.x1 + (area.x2 - area.x1) / 2 + sin(angle) * 100;
        int32_t y1 = area.y1 + (area.y2 - area.y1) / 2 - cos(angle) * 100;
        int32_t x2 = area.x1 + (area.x2 - area.x1) / 2 + sin(angle) * 110;
        int32_t y2 = area.y1 + (area.y2 - area.y1) / 2 - cos(angle) * 110;
        
        lv_draw_line_dsc_t line_dsc;
        lv_draw_line_dsc_init(&line_dsc);
        line_dsc.color = lv_color_hex(0x666666);
        line_dsc.width = 3;
        lv_draw_line(draw_ctx, &line_dsc, x1, y1, x2, y2);
    }
}

// 创建指针样式
static void create_hand_styles(void) {
    // 时针样式
    lv_style_init(&style_hour_hand);
    lv_style_set_line_width(&style_hour_hand, 4);
    lv_style_set_line_color(&style_hour_hand, lv_color_hex(0x666666));
    lv_style_set_line_rounded(&style_hour_hand, true);
    
    // 分针样式
    lv_style_init(&style_min_hand);
    lv_style_set_line_width(&style_min_hand, 3);
    lv_style_set_line_color(&style_min_hand, lv_color_hex(0x888888));
    lv_style_set_line_rounded(&style_min_hand, true);
    
    // 秒针样式
    lv_style_init(&style_sec_hand);
    lv_style_set_line_width(&style_sec_hand, 2);
    lv_style_set_line_color(&style_sec_hand, lv_color_hex(0xb76e5d));
    lv_style_set_line_rounded(&style_sec_hand, true);
}

// 创建日期窗口
static void create_date_window(void) {
    date_label = lv_label_create(clock_obj);
    
    static lv_style_t style_date;
    lv_style_init(&style_date);
    lv_style_set_bg_color(&style_date, lv_color_white());
    lv_style_set_border_color(&style_date, lv_color_hex(0xd4b5ac));
    lv_style_set_border_width(&style_date, 1);
    lv_style_set_pad_all(&style_date, 2);
    lv_style_set_text_color(&style_date, lv_color_hex(0xd4b5ac));
    
    lv_obj_add_style(date_label, &style_date, 0);
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 40);
}

// 更新时间处理函数
static void update_time(lv_timer_t * timer) {
    datetime_t t;
    rtc_get_datetime(&t);
    
    // 计算指针角度
    int32_t hour_angle = (t.hour % 12 + t.min / 60.0f) * 30;
    int32_t min_angle = t.min * 6;
    int32_t sec_angle = t.sec * 6;
    
    // 更新指针位置
    lv_obj_set_style_transform_angle(hour_hand, hour_angle * 10, 0);
    lv_obj_set_style_transform_angle(min_hand, min_angle * 10, 0);
    lv_obj_set_style_transform_angle(sec_hand, sec_angle * 10, 0);
    
    // 更新日期显示
    char date_str[32];
    snprintf(date_str, sizeof(date_str), "%s %02d", 
             t.month < 13 ? month_names[t.month-1] : "???", 
             t.day);
    lv_label_set_text(date_label, date_str);
}

// LVGL 初始化
static void lvgl_init(void)
{
    lv_init();

    // 创建显示设备
    disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    
    // 设置显示缓冲区
    lv_display_set_buffers(disp, buf1, NULL, DISP_BUF_SIZE * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    // 设置刷新回调
    lv_display_set_flush_cb(disp, disp_flush);

    // 创建时钟对象
    clock_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(clock_obj, 240, 240);
    lv_obj_center(clock_obj);
    
    // 应用金属质感样式
    create_metallic_style(&style_clock);
    lv_obj_add_style(clock_obj, &style_clock, 0);
    
    // 添加表盘绘制事件
    lv_obj_add_event_cb(clock_obj, draw_clock_face, LV_EVENT_DRAW_MAIN, NULL);

    // 创建指针
    create_hand_styles();
    
    hour_hand = lv_line_create(clock_obj);
    min_hand = lv_line_create(clock_obj);
    sec_hand = lv_line_create(clock_obj);
    
    lv_obj_add_style(hour_hand, &style_hour_hand, 0);
    lv_obj_add_style(min_hand, &style_min_hand, 0);
    lv_obj_add_style(sec_hand, &style_sec_hand, 0);
    
    // 创建日期窗口
    create_date_window();
    
    // 创建定时器更新时间
    lv_timer_create(update_time, 1000, NULL);
}

// 主函数
int main()
{
    stdio_init_all();
    lcd_init();
    lvgl_init();

    while (1) {
        lv_timer_handler();
        sleep_ms(5);
    }

    return 0;
} 