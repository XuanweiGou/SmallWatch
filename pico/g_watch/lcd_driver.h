#ifndef LCD_DRIVER_H
#define LCD_DRIVER_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"

// LCD分辨率定义
#define LCD_WIDTH   240
#define LCD_HEIGHT  240

// LCD控制引脚定义
#define LCD_SPI_PORT    spi1
#define LCD_DC_PIN      8   // 数据/命令选择引脚
#define LCD_CS_PIN      9   // 片选引脚
#define LCD_CLK_PIN     10  // SPI时钟引脚
#define LCD_DIN_PIN     11  // SPI数据输入引脚
#define LCD_RST_PIN     12  // 复位引脚
#define LCD_BL_PIN      13  // 背光控制引脚

// GC9A01A LCD控制器命令
#define LCD_CMD_NOP        0x00
#define LCD_CMD_SWRESET    0x01  // 软件复位
#define LCD_CMD_SLPIN      0x10  // 睡眠模式
#define LCD_CMD_SLPOUT     0x11  // 退出睡眠模式
#define LCD_CMD_PTLON      0x12  // 部分显示模式
#define LCD_CMD_NORON      0x13  // 正常显示模式
#define LCD_CMD_INVOFF     0x20  // 关闭显示反转
#define LCD_CMD_INVON      0x21  // 开启显示反转
#define LCD_CMD_DISPOFF    0x28  // 关闭显示
#define LCD_CMD_DISPON     0x29  // 开启显示
#define LCD_CMD_CASET      0x2A  // 列地址设置
#define LCD_CMD_RASET      0x2B  // 行地址设置
#define LCD_CMD_RAMWR      0x2C  // 内存写入
#define LCD_CMD_RAMRD      0x2E  // 内存读取
#define LCD_CMD_MADCTL     0x36  // 内存数据访问控制
#define LCD_CMD_COLMOD     0x3A  // 接口像素格式

// 函数声明
// 初始化LCD
void lcd_init(void);

// 设置显示窗口
void lcd_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

// 写命令
void lcd_write_cmd(uint8_t cmd);

// 写数据
void lcd_write_data(uint8_t data);

// 写颜色数据
void lcd_write_color(uint16_t color);

// 写入缓冲区数据
void lcd_write_buffer(const uint8_t* buffer, size_t size);

// 清屏
void lcd_clear(uint16_t color);

// 开启显示
void lcd_display_on(void);

// 关闭显示
void lcd_display_off(void);

// 设置背光
void lcd_set_backlight(bool on);

#endif // LCD_DRIVER_H 