#include "lv_drv_conf.h"        // 首先包含驱动配置
#include "libs/SDL2/include/SDL.h"     // 从 libs 目录开始的完整路径
#include "libs/lvgl/lvgl.h"            // 从 libs 目录开始的完整路径
#include "libs/lv_drivers/display/monitor.h"  // 从 libs 目录开始的完整路径
#include "libs/lv_drivers/indev/mouse.h"      // 从 libs 目录开始的完整路径
#include <time.h>
#include <math.h>

// 使用与实际项目相同的显示尺寸
#define LCD_WIDTH 240
#define LCD_HEIGHT 240

// 全局变量
static lv_obj_t *g_hour_hand;
static lv_obj_t *g_min_hand;
static lv_obj_t *g_sec_hand;
static lv_obj_t *g_date_label;
static int32_t g_last_sec_angle = 0;
static int32_t g_last_min_angle = 0;
static int32_t g_last_hour_angle = 0;

// 鼠标读取回调
static void mouse_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    // 获取鼠标状态
    int32_t x, y;
    bool left_button;
    
    SDL_GetMouseState(&x, &y);
    left_button = SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT);
    
    // 更新数据
    data->point.x = x;
    data->point.y = y;
    data->state = left_button ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}

// LVGL 9.x 版本的显示缓冲区
static lv_display_t * disp;              // 显示设备
static lv_color_t buf1[LCD_WIDTH * 20];  // 颜色缓冲区

// 显示刷新回调
static void display_flush_cb(lv_display_t * disp_drv, const lv_area_t * area, lv_color_t * px_map)
{
    monitor_flush(disp_drv, area, px_map);
}

// 动画回调函数
static void hand_animation_cb(void * var, int32_t value)
{
    lv_obj_t * hand = (lv_obj_t *)var;
    lv_obj_set_style_transform_angle(hand, value, 0);
}

// 时间更新定时器回调
static void update_time(lv_timer_t * timer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    // 计算角度
    int32_t hour_angle = (t->tm_hour % 12 + t->tm_min / 60.0) * 30 * 10;  // 30度每小时，需要乘以10
    int32_t min_angle = t->tm_min * 6 * 10;  // 6度每分钟，需要乘以10
    int32_t sec_angle = t->tm_sec * 6 * 10;  // 6度每秒，需要乘以10

    // 创建动画
    if (sec_angle != g_last_sec_angle) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, g_sec_hand);
        lv_anim_set_exec_cb(&a, hand_animation_cb);
        lv_anim_set_values(&a, g_last_sec_angle, sec_angle);
        lv_anim_set_time(&a, 200);  // 200ms
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_start(&a);
        g_last_sec_angle = sec_angle;
    }

    if (min_angle != g_last_min_angle) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, g_min_hand);
        lv_anim_set_exec_cb(&a, hand_animation_cb);
        lv_anim_set_values(&a, g_last_min_angle, min_angle);
        lv_anim_set_time(&a, 300);  // 300ms
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_start(&a);
        g_last_min_angle = min_angle;
    }

    if (hour_angle != g_last_hour_angle) {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, g_hour_hand);
        lv_anim_set_exec_cb(&a, hand_animation_cb);
        lv_anim_set_values(&a, g_last_hour_angle, hour_angle);
        lv_anim_set_time(&a, 400);  // 400ms
        lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
        lv_anim_start(&a);
        g_last_hour_angle = hour_angle;
    }
    
    // 更新日期
    char date_str[32];
    snprintf(date_str, sizeof(date_str), "%s %02d", 
             t->tm_mon == 0 ? "JAN" : t->tm_mon == 1 ? "FEB" : t->tm_mon == 2 ? "MAR" :
             t->tm_mon == 3 ? "APR" : t->tm_mon == 4 ? "MAY" : t->tm_mon == 5 ? "JUN" :
             t->tm_mon == 6 ? "JUL" : t->tm_mon == 7 ? "AUG" : t->tm_mon == 8 ? "SEP" :
             t->tm_mon == 9 ? "OCT" : t->tm_mon == 10 ? "NOV" : "DEC", 
             t->tm_mday);
    lv_label_set_text(g_date_label, date_str);
}

// 初始化硬件抽象层
static void hal_init(void) {
    // 初始化 SDL 显示
    monitor_init();
    
    // 创建显示设备
    disp = lv_display_create(LCD_WIDTH, LCD_HEIGHT);
    
    // 设置显示缓冲区 - 增加缓冲区大小以提高性能
    lv_display_set_buffers(disp, buf1, NULL, 
                          LCD_WIDTH * 20 * sizeof(lv_color_t),  // 增加缓冲区大小
                          LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    // 设置刷新回调
    lv_display_set_flush_cb(disp, display_flush_cb);
    
    // 初始化鼠标
    mouse_init();
}

// 创建时钟界面
static void create_clock_ui(void) {
    // 创建时钟对象
    lv_obj_t * clock_obj = lv_obj_create(lv_scr_act());
    lv_obj_set_size(clock_obj, LCD_WIDTH, LCD_HEIGHT);
    lv_obj_center(clock_obj);
    
    // 设置背景样式
    static lv_style_t style_clock;
    lv_style_init(&style_clock);
    lv_style_set_bg_color(&style_clock, lv_color_hex(0xf7e8e3));  // 玫瑰金色
    lv_style_set_bg_grad_color(&style_clock, lv_color_hex(0xe8cec7));  // 深玫瑰金
    lv_style_set_bg_grad_dir(&style_clock, LV_GRAD_DIR_VER);
    lv_style_set_radius(&style_clock, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&style_clock, 0);
    lv_style_set_shadow_width(&style_clock, 20);
    lv_style_set_shadow_color(&style_clock, lv_color_hex(0x333333));
    lv_style_set_shadow_opa(&style_clock, LV_OPA_30);
    lv_obj_add_style(clock_obj, &style_clock, 0);

    // 创建内圈装饰
    lv_obj_t * inner_circle = lv_obj_create(clock_obj);
    static lv_style_t style_inner;
    lv_style_init(&style_inner);
    lv_style_set_radius(&style_inner, LV_RADIUS_CIRCLE);
    lv_style_set_bg_color(&style_inner, lv_color_hex(0xffffff));
    lv_style_set_bg_grad_color(&style_inner, lv_color_hex(0xf7e8e3));
    lv_style_set_bg_grad_dir(&style_inner, LV_GRAD_DIR_VER);
    lv_style_set_bg_opa(&style_inner, LV_OPA_50);
    lv_style_set_border_width(&style_inner, 0);
    lv_obj_add_style(inner_circle, &style_inner, 0);
    lv_obj_set_size(inner_circle, LCD_WIDTH - 40, LCD_HEIGHT - 40);
    lv_obj_center(inner_circle);

    // 创建表盘刻度
    for(int i = 0; i < 12; i++) {
        lv_obj_t * tick = lv_obj_create(clock_obj);
        
        // 设置刻度样式
        static lv_style_t style_tick;
        lv_style_init(&style_tick);
        lv_style_set_bg_color(&style_tick, lv_color_hex(0x333333));
        lv_style_set_bg_grad_color(&style_tick, lv_color_hex(0x666666));
        lv_style_set_bg_grad_dir(&style_tick, LV_GRAD_DIR_VER);
        lv_style_set_shadow_width(&style_tick, 3);
        lv_style_set_shadow_color(&style_tick, lv_color_hex(0x000000));
        lv_style_set_shadow_opa(&style_tick, LV_OPA_30);
        
        // 计算刻度位置
        float angle = i * 30.0f;  // 360/12 = 30度
        float rad = angle * 3.14159f / 180.0f;
        int32_t x = LCD_WIDTH/2 + (int32_t)(sinf(rad) * (LCD_WIDTH/2 - 20));
        int32_t y = LCD_HEIGHT/2 - (int32_t)(cosf(rad) * (LCD_HEIGHT/2 - 20));
        
        // 设置刻度大小和位置
        lv_obj_set_size(tick, 4, 15);
        lv_obj_center(tick);
        lv_obj_set_pos(tick, x - 2, y - 7);
        lv_obj_set_style_transform_angle(tick, angle * 10, 0);
        
        lv_obj_add_style(tick, &style_tick, 0);
    }

    // 创建品牌名称标签
    lv_obj_t * brand_label = lv_label_create(clock_obj);
    static lv_style_t style_brand;
    lv_style_init(&style_brand);
    lv_style_set_text_color(&style_brand, lv_color_hex(0xb76e5d));
    lv_style_set_text_font(&style_brand, &lv_font_default);
    lv_style_set_text_opa(&style_brand, LV_OPA_80);
    lv_obj_add_style(brand_label, &style_brand, 0);
    lv_label_set_text(brand_label, "YongqiGou");
    lv_obj_align(brand_label, LV_ALIGN_CENTER, 0, -30);

    // 修改日期窗口，添加阴影和渐变
    lv_obj_t * date_box = lv_obj_create(clock_obj);
    static lv_style_t style_date;
    lv_style_init(&style_date);
    lv_style_set_bg_color(&style_date, lv_color_white());
    lv_style_set_bg_grad_color(&style_date, lv_color_hex(0xf8f8f8));
    lv_style_set_bg_grad_dir(&style_date, LV_GRAD_DIR_VER);
    lv_style_set_border_color(&style_date, lv_color_hex(0xcccccc));
    lv_style_set_border_width(&style_date, 1);
    lv_style_set_shadow_width(&style_date, 5);
    lv_style_set_shadow_color(&style_date, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_date, LV_OPA_20);
    lv_obj_add_style(date_box, &style_date, 0);
    lv_obj_set_size(date_box, 60, 20);
    lv_obj_align(date_box, LV_ALIGN_CENTER, 0, 40);

    // 添加日期标签
    g_date_label = lv_label_create(date_box);
    static lv_style_t style_date_text;
    lv_style_init(&style_date_text);
    lv_style_set_text_color(&style_date_text, lv_color_hex(0x333333));
    lv_obj_add_style(g_date_label, &style_date_text, 0);
    lv_obj_center(g_date_label);
    lv_label_set_text(g_date_label, "DEC 01");

    // 创建时针、分针、秒针
    static lv_point_t hour_hand_points[] = {{0,8}, {4,0}, {0,-40}, {-4,0}};
    static lv_point_t min_hand_points[] = {{0,8}, {3,0}, {0,-70}, {-3,0}};
    static lv_point_t sec_hand_points[] = {{0,10}, {2,0}, {0,-80}, {-2,0}};

    lv_obj_t * hour_hand = lv_line_create(clock_obj);
    lv_obj_t * min_hand = lv_line_create(clock_obj);
    lv_obj_t * sec_hand = lv_line_create(clock_obj);

    static lv_style_t style_hour_hand;
    static lv_style_t style_min_hand;
    static lv_style_t style_sec_hand;

    lv_style_init(&style_hour_hand);
    lv_style_init(&style_min_hand);
    lv_style_init(&style_sec_hand);

    // 时针样式
    lv_style_set_line_width(&style_hour_hand, 4);
    lv_style_set_line_color(&style_hour_hand, lv_color_hex(0x333333));
    lv_style_set_line_rounded(&style_hour_hand, true);
    lv_style_set_shadow_width(&style_hour_hand, 5);
    lv_style_set_shadow_color(&style_hour_hand, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_hour_hand, LV_OPA_30);

    // 分针样式
    lv_style_set_line_width(&style_min_hand, 3);
    lv_style_set_line_color(&style_min_hand, lv_color_hex(0x666666));
    lv_style_set_line_rounded(&style_min_hand, true);
    lv_style_set_shadow_width(&style_min_hand, 4);
    lv_style_set_shadow_color(&style_min_hand, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_min_hand, LV_OPA_20);

    // 秒针样式
    lv_style_set_line_width(&style_sec_hand, 2);
    lv_style_set_line_color(&style_sec_hand, lv_color_hex(0xff0000));
    lv_style_set_line_rounded(&style_sec_hand, true);
    lv_style_set_shadow_width(&style_sec_hand, 3);
    lv_style_set_shadow_color(&style_sec_hand, lv_color_hex(0x000000));
    lv_style_set_shadow_opa(&style_sec_hand, LV_OPA_20);

    // 设置指针的旋转中心点
    lv_style_set_transform_pivot_x(&style_hour_hand, 0);
    lv_style_set_transform_pivot_y(&style_hour_hand, 0);
    lv_style_set_transform_pivot_x(&style_min_hand, 0);
    lv_style_set_transform_pivot_y(&style_min_hand, 0);
    lv_style_set_transform_pivot_x(&style_sec_hand, 0);
    lv_style_set_transform_pivot_y(&style_sec_hand, 0);

    lv_obj_add_style(hour_hand, &style_hour_hand, 0);
    lv_obj_add_style(min_hand, &style_min_hand, 0);
    lv_obj_add_style(sec_hand, &style_sec_hand, 0);

    lv_line_set_points(hour_hand, hour_hand_points, 4);
    lv_line_set_points(min_hand, min_hand_points, 4);
    lv_line_set_points(sec_hand, sec_hand_points, 4);

    lv_obj_center(hour_hand);
    lv_obj_center(min_hand);
    lv_obj_center(sec_hand);

    // 设置指针的旋转原点
    lv_obj_set_style_transform_pivot_x(hour_hand, LCD_WIDTH/2, 0);
    lv_obj_set_style_transform_pivot_y(hour_hand, LCD_HEIGHT/2, 0);
    lv_obj_set_style_transform_pivot_x(min_hand, LCD_WIDTH/2, 0);
    lv_obj_set_style_transform_pivot_y(min_hand, LCD_HEIGHT/2, 0);
    lv_obj_set_style_transform_pivot_x(sec_hand, LCD_WIDTH/2, 0);
    lv_obj_set_style_transform_pivot_y(sec_hand, LCD_HEIGHT/2, 0);

    // 创建中心点装饰
    // 外圈
    lv_obj_t * center_ring = lv_obj_create(clock_obj);
    static lv_style_t style_center_ring;
    lv_style_init(&style_center_ring);
    lv_style_set_bg_color(&style_center_ring, lv_color_hex(0x333333));
    lv_style_set_bg_grad_color(&style_center_ring, lv_color_hex(0x666666));
    lv_style_set_bg_grad_dir(&style_center_ring, LV_GRAD_DIR_VER);
    lv_style_set_radius(&style_center_ring, LV_RADIUS_CIRCLE);
    lv_style_set_border_width(&style_center_ring, 1);
    lv_style_set_border_color(&style_center_ring, lv_color_hex(0x999999));
    lv_obj_add_style(center_ring, &style_center_ring, 0);
    lv_obj_set_size(center_ring, 12, 12);
    lv_obj_center(center_ring);

    // 内圈
    lv_obj_t * center_dot = lv_obj_create(clock_obj);
    static lv_style_t style_center;
    lv_style_init(&style_center);
    lv_style_set_bg_color(&style_center, lv_color_hex(0x000000));
    lv_style_set_radius(&style_center, LV_RADIUS_CIRCLE);
    lv_obj_add_style(center_dot, &style_center, 0);
    lv_obj_set_size(center_dot, 6, 6);
    lv_obj_center(center_dot);

    // 保存指针对象到全局变量
    g_hour_hand = hour_hand;
    g_min_hand = min_hand;
    g_sec_hand = sec_hand;
}

int main(void) {
    // 初始化 SDL
    SDL_Init(SDL_INIT_VIDEO);
    
    // 初始化 LVGL
    lv_init();
    
    // 初始化硬件抽象层
    hal_init();
    
    // 创建时钟界面
    create_clock_ui();
    
    // 创建时间更新定时器 - 减少更新频率以优化性能
    lv_timer_create(update_time, 100, NULL);  // 每100ms更新一次
    
    // 主循环
    while(1) {
        lv_timer_handler();
        SDL_Delay(10);  // 增加延时以减少CPU使用率
        
        // 处理 SDL 事件
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                goto cleanup;
            }
        }
    }

cleanup:
    SDL_Quit();
    return 0;
} 