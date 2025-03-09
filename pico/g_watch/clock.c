#include "clock.h"
#include <math.h>
#include "hardware/rtc.h"
#include "pico/stdlib.h"

// 静态变量
static lv_obj_t *clock_canvas;
static uint32_t frame_count = 0;
static uint32_t last_step_time = 0;

// 颜色定义
#define COLOR_ROSE_GOLD_LIGHT lv_color_make(247, 232, 227)  // #f7e8e3
#define COLOR_ROSE_GOLD_MID   lv_color_make(242, 220, 212)  // #f2dcd4
#define COLOR_ROSE_GOLD_DARK  lv_color_make(232, 206, 199)  // #e8cec7
#define COLOR_SILVER_LIGHT    lv_color_make(232, 232, 232)  // #e8e8e8
#define COLOR_SILVER_MID      lv_color_make(153, 153, 153)  // #999999
#define COLOR_SILVER_DARK     lv_color_make(102, 102, 102)  // #666666

// 绘制三角形时标
static void draw_triangle_marker(lv_obj_t *canvas, float angle) {
    const float inner_radius = CLOCK_RADIUS * 0.75f;
    const float outer_radius = CLOCK_RADIUS * 0.95f;
    const float angle_width = 0.026f;
    
    lv_point_t points[3];
    
    // 计算三角形顶点
    points[0].x = CLOCK_CENTER_X + cosf(angle) * inner_radius;
    points[0].y = CLOCK_CENTER_Y + sinf(angle) * inner_radius;
    
    points[1].x = CLOCK_CENTER_X + cosf(angle - angle_width) * outer_radius;
    points[1].y = CLOCK_CENTER_Y + sinf(angle - angle_width) * outer_radius;
    
    points[2].x = CLOCK_CENTER_X + cosf(angle + angle_width) * outer_radius;
    points[2].y = CLOCK_CENTER_Y + sinf(angle + angle_width) * outer_radius;

    // 创建金属渐变效果
    lv_draw_mask_fade_param_t fade_mask_param;
    lv_area_t area = {
        .x1 = points[0].x - 5,
        .y1 = points[0].y - 5,
        .x2 = points[2].x + 5,
        .y2 = points[2].y + 5
    };

    // 绘制三角形
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = COLOR_SILVER_MID;
    rect_dsc.bg_grad_color = COLOR_SILVER_LIGHT;
    rect_dsc.bg_grad_dir = LV_GRAD_DIR_HOR;
    rect_dsc.radius = 0;
    rect_dsc.border_width = 1;
    rect_dsc.border_color = COLOR_SILVER_DARK;
    
    lv_draw_polygon(canvas, points, 3, &area, &rect_dsc);
}

// 绘制波纹效果
static void draw_ripples(lv_obj_t *canvas) {
    const uint8_t total_ripples = 4;
    
    for(uint8_t i = 0; i < total_ripples; i++) {
        float progress = i / (float)total_ripples;
        float radius = CLOCK_RADIUS * (0.45f + powf(progress, 0.6f) * 0.5f);
        
        // 计算波纹强度
        uint8_t opacity;
        if (progress < 0.25f) {
            opacity = (uint8_t)(0.04f * (1.0f - progress * 2.0f) * 255);
        } else {
            opacity = (uint8_t)(0.1f * expf(-2.5f * progress) * 255);
        }

        // 绘制波纹
        lv_draw_arc_dsc_t arc_dsc;
        lv_draw_arc_dsc_init(&arc_dsc);
        arc_dsc.color = lv_color_black();
        arc_dsc.width = 1;
        arc_dsc.opa = opacity;
        
        lv_draw_arc(canvas, CLOCK_CENTER_X, CLOCK_CENTER_Y, radius, 0, 360, &arc_dsc);
    }
}

// 绘制金属指针
static void draw_metallic_hand(lv_obj_t *canvas, float angle, float length, float width) {
    float cos_angle = cosf(angle);
    float sin_angle = sinf(angle);
    
    // 计算指针形状点
    lv_point_t points[7];  // 使用7个点创建更平滑的形状
    
    // 尾部点
    points[0].x = CLOCK_CENTER_X - width/2 * cos_angle;
    points[0].y = CLOCK_CENTER_Y - width/2 * sin_angle;
    
    // 中间控制点
    points[1].x = CLOCK_CENTER_X - width/3 * cos_angle - width/4 * sin_angle;
    points[1].y = CLOCK_CENTER_Y - width/3 * sin_angle + width/4 * cos_angle;
    
    points[2].x = CLOCK_CENTER_X - width/6 * cos_angle - width/2 * sin_angle;
    points[2].y = CLOCK_CENTER_Y - width/6 * sin_angle + width/2 * cos_angle;
    
    // 尖端
    points[3].x = CLOCK_CENTER_X + length * sin_angle;
    points[3].y = CLOCK_CENTER_Y - length * cos_angle;
    
    // 对称的另一侧控制点
    points[4].x = CLOCK_CENTER_X - width/6 * cos_angle + width/2 * sin_angle;
    points[4].y = CLOCK_CENTER_Y - width/6 * sin_angle - width/2 * cos_angle;
    
    points[5].x = CLOCK_CENTER_X - width/3 * cos_angle + width/4 * sin_angle;
    points[5].y = CLOCK_CENTER_Y - width/3 * sin_angle - width/4 * cos_angle;
    
    points[6].x = points[0].x;
    points[6].y = points[0].y;

    // 绘制指针
    lv_draw_polygon_dsc_t poly_dsc;
    lv_draw_polygon_dsc_init(&poly_dsc);
    poly_dsc.bg_color = COLOR_SILVER_DARK;
    poly_dsc.bg_grad_color = COLOR_SILVER_LIGHT;
    poly_dsc.bg_grad_dir = LV_GRAD_DIR_HOR;
    
    lv_area_t area = {
        .x1 = CLOCK_CENTER_X - length,
        .y1 = CLOCK_CENTER_Y - length,
        .x2 = CLOCK_CENTER_X + length,
        .y2 = CLOCK_CENTER_Y + length
    };
    
    lv_draw_polygon(canvas, points, 7, &area, &poly_dsc);
}

// 绘制表盘
void draw_clock_face(void) {
    // 清空画布
    lv_canvas_fill_bg(clock_canvas, COLOR_ROSE_GOLD_LIGHT, LV_OPA_COVER);
    
    // 绘制波纹
    draw_ripples(clock_canvas);
    
    // 绘制时标
    for(uint8_t i = 0; i < 12; i++) {
        float angle = i * M_PI / 6 - M_PI / 2;
        draw_triangle_marker(clock_canvas, angle);
    }
    
    // 绘制品牌名
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = COLOR_ROSE_GOLD_DARK;
    label_dsc.font = &lv_font_montserrat_16;
    
    lv_point_t pos = {
        .x = CLOCK_CENTER_X,
        .y = CLOCK_CENTER_Y - 30
    };
    
    lv_draw_label(clock_canvas, &pos, NULL, "YongqiGou", &label_dsc);
}

// 绘制日期窗口
void draw_date_window(void) {
    datetime_t t;
    rtc_get_datetime(&t);
    
    static const char* months[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                                 "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"};
    
    char date[3];
    snprintf(date, sizeof(date), "%02d", t.day);
    
    // 计算窗口尺寸和位置
    const float window_width = CLOCK_RADIUS * 0.3f * 1.1f;
    const float window_height = CLOCK_RADIUS * 0.12f * 1.1f;
    const float y_pos = CLOCK_CENTER_Y + CLOCK_RADIUS * 0.35f + 20;
    
    // 计算Dec和01的宽度
    const float dec_width = window_width * 0.67f;
    const float date_width = window_width * 0.33f;
    const float spacing = 3;  // 3像素间距
    
    // 绘制月份框
    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.bg_color = lv_color_white();
    rect_dsc.border_color = COLOR_ROSE_GOLD_DARK;
    rect_dsc.border_width = 1;
    
    lv_area_t month_area = {
        .x1 = CLOCK_CENTER_X - (date_width + spacing) / 2 - dec_width - 1,
        .y1 = y_pos - window_height/2,
        .x2 = CLOCK_CENTER_X - (date_width + spacing) / 2 - 1,
        .y2 = y_pos + window_height/2
    };
    
    lv_draw_rect(clock_canvas, &month_area, &rect_dsc);
    
    // 绘制日期框
    lv_area_t date_area = {
        .x1 = CLOCK_CENTER_X + (dec_width + spacing) / 2,
        .y1 = y_pos - window_height/2,
        .x2 = CLOCK_CENTER_X + (dec_width + spacing) / 2 + date_width,
        .y2 = y_pos + window_height/2
    };
    
    lv_draw_rect(clock_canvas, &date_area, &rect_dsc);
    
    // 绘制文本
    lv_draw_label_dsc_t label_dsc;
    lv_draw_label_dsc_init(&label_dsc);
    label_dsc.color = COLOR_ROSE_GOLD_DARK;
    
    // 月份文本
    label_dsc.font = &lv_font_montserrat_12;
    lv_point_t month_pos = {
        .x = (month_area.x1 + month_area.x2) / 2,
        .y = y_pos + 1
    };
    lv_draw_label(clock_canvas, &month_pos, &month_area, months[t.month-1], &label_dsc);
    
    // 日期文本
    label_dsc.font = &lv_font_montserrat_11;
    lv_point_t date_pos = {
        .x = (date_area.x1 + date_area.x2) / 2,
        .y = y_pos + 0.5f
    };
    lv_draw_label(clock_canvas, &date_pos, &date_area, date, &label_dsc);
}

// 绘制指针
void draw_hands(void) {
    datetime_t t;
    rtc_get_datetime(&t);
    
    // 计算角度
    float hour_angle = (t.hour % 12 + t.min / 60.0f) * M_PI / 6 - M_PI / 2;
    float min_angle = (t.min + t.sec / 60.0f) * M_PI / 30 - M_PI / 2;
    float sec_angle = t.sec * M_PI / 30 - M_PI / 2;
    
    // 绘制指针
    draw_metallic_hand(clock_canvas, hour_angle, CLOCK_RADIUS * 0.45f, 6);
    draw_metallic_hand(clock_canvas, min_angle, CLOCK_RADIUS * 0.65f, 4);
    draw_metallic_hand(clock_canvas, sec_angle, CLOCK_RADIUS * 0.8f, 2);
    
    // 绘制中心圆点
    lv_draw_arc_dsc_t center_dsc;
    lv_draw_arc_dsc_init(&center_dsc);
    center_dsc.color = COLOR_SILVER_DARK;
    lv_draw_arc(clock_canvas, CLOCK_CENTER_X, CLOCK_CENTER_Y, 4, 0, 360, &center_dsc);
}

// 初始化时钟
void init_clock(void) {
    // 创建画布
    static lv_color_t buf[LCD_WIDTH * LCD_HEIGHT];
    clock_canvas = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(clock_canvas, buf, LCD_WIDTH, LCD_HEIGHT, LV_IMG_CF_TRUE_COLOR);
    lv_obj_center(clock_canvas);
    
    // 创建定时器
    lv_timer_create(update_clock, 1000/60, NULL);  // 60fps
}

// 更新函数
static void update_clock(lv_timer_t *timer) {
    frame_count++;
    
    // 每3帧更新一次显示
    if (frame_count % 3 == 0) {
        draw_clock_face();
        draw_date_window();
        draw_hands();
    }
}