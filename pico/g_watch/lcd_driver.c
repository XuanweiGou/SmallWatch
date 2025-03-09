#include "lcd_driver.h"
#include <string.h>

// GC9A01A初始化命令序列
static const uint8_t init_cmds[] = {
    0xEF, 0,
    0xEB, 1, 0x14,
    0xFE, 0,
    0xEF, 0,
    0xEB, 1, 0x14,
    0x84, 1, 0x40,
    0x85, 1, 0xFF,
    0x86, 1, 0xFF,
    0x87, 1, 0xFF,
    0x88, 1, 0x0A,
    0x89, 1, 0x21,
    0x8A, 1, 0x00,
    0x8B, 1, 0x80,
    0x8C, 1, 0x01,
    0x8D, 1, 0x01,
    0x8E, 1, 0xFF,
    0x8F, 1, 0xFF,
    LCD_CMD_COLMOD, 1, 0x05,  // 16-bit/pixel
    0x90, 4, 0x08, 0x08, 0x08, 0x08,
    0xBD, 1, 0x06,
    0xBC, 1, 0x00,
    0xFF, 3, 0x60, 0x01, 0x04,
    LCD_CMD_MADCTL, 1, 0x48,  // MX | BGR
    0x13, 0,
    0x10, 0
};

// 静态函数声明
static void lcd_write_byte(uint8_t data);
static void lcd_write_bytes(const uint8_t* data, size_t len);
static void lcd_reset(void);
static void lcd_init_pins(void);

// 初始化LCD
void lcd_init(void) {
    // 初始化引脚
    lcd_init_pins();
    
    // 初始化SPI
    spi_init(LCD_SPI_PORT, 40000000);  // 40MHz
    gpio_set_function(LCD_CLK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LCD_DIN_PIN, GPIO_FUNC_SPI);
    
    // 复位LCD
    lcd_reset();
    
    // 发送初始化命令序列
    const uint8_t *cmd = init_cmds;
    while (cmd < init_cmds + sizeof(init_cmds)) {
        lcd_write_cmd(*cmd++);
        uint8_t num_args = *cmd++;
        if (num_args > 0) {
            lcd_write_bytes(cmd, num_args);
            cmd += num_args;
        }
    }
    
    // 退出睡眠模式
    lcd_write_cmd(LCD_CMD_SLPOUT);
    sleep_ms(120);
    
    // 开启显示
    lcd_write_cmd(LCD_CMD_DISPON);
    sleep_ms(20);
    
    // 开启背光
    lcd_set_backlight(true);
}

// 初始化引脚
static void lcd_init_pins(void) {
    gpio_init(LCD_DC_PIN);
    gpio_init(LCD_CS_PIN);
    gpio_init(LCD_RST_PIN);
    gpio_init(LCD_BL_PIN);
    
    gpio_set_dir(LCD_DC_PIN, GPIO_OUT);
    gpio_set_dir(LCD_CS_PIN, GPIO_OUT);
    gpio_set_dir(LCD_RST_PIN, GPIO_OUT);
    gpio_set_dir(LCD_BL_PIN, GPIO_OUT);
    
    gpio_put(LCD_CS_PIN, 1);
    gpio_put(LCD_DC_PIN, 1);
    gpio_put(LCD_RST_PIN, 1);
    gpio_put(LCD_BL_PIN, 0);
}

// 复位LCD
static void lcd_reset(void) {
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(5);
    gpio_put(LCD_RST_PIN, 0);
    sleep_ms(15);
    gpio_put(LCD_RST_PIN, 1);
    sleep_ms(15);
}

// 写命令
void lcd_write_cmd(uint8_t cmd) {
    gpio_put(LCD_DC_PIN, 0);  // 命令模式
    gpio_put(LCD_CS_PIN, 0);  // 片选使能
    lcd_write_byte(cmd);
    gpio_put(LCD_CS_PIN, 1);  // 片选禁用
}

// 写数据
void lcd_write_data(uint8_t data) {
    gpio_put(LCD_DC_PIN, 1);  // 数据模式
    gpio_put(LCD_CS_PIN, 0);  // 片选使能
    lcd_write_byte(data);
    gpio_put(LCD_CS_PIN, 1);  // 片选禁用
}

// 写颜色数据
void lcd_write_color(uint16_t color) {
    gpio_put(LCD_DC_PIN, 1);  // 数据模式
    gpio_put(LCD_CS_PIN, 0);  // 片选使能
    lcd_write_byte(color >> 8);
    lcd_write_byte(color & 0xFF);
    gpio_put(LCD_CS_PIN, 1);  // 片选禁用
}

// 写入缓冲区数据
void lcd_write_buffer(const uint8_t* buffer, size_t size) {
    gpio_put(LCD_DC_PIN, 1);  // 数据模式
    gpio_put(LCD_CS_PIN, 0);  // 片选使能
    lcd_write_bytes(buffer, size);
    gpio_put(LCD_CS_PIN, 1);  // 片选禁用
}

// 设置显示窗口
void lcd_set_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
    lcd_write_cmd(LCD_CMD_CASET);
    lcd_write_data(x1 >> 8);
    lcd_write_data(x1 & 0xFF);
    lcd_write_data(x2 >> 8);
    lcd_write_data(x2 & 0xFF);
    
    lcd_write_cmd(LCD_CMD_RASET);
    lcd_write_data(y1 >> 8);
    lcd_write_data(y1 & 0xFF);
    lcd_write_data(y2 >> 8);
    lcd_write_data(y2 & 0xFF);
    
    lcd_write_cmd(LCD_CMD_RAMWR);
}

// 清屏
void lcd_clear(uint16_t color) {
    lcd_set_window(0, 0, LCD_WIDTH-1, LCD_HEIGHT-1);
    
    gpio_put(LCD_DC_PIN, 1);  // 数据模式
    gpio_put(LCD_CS_PIN, 0);  // 片选使能
    
    for(uint32_t i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
        lcd_write_byte(color >> 8);
        lcd_write_byte(color & 0xFF);
    }
    
    gpio_put(LCD_CS_PIN, 1);  // 片选禁用
}

// 开启显示
void lcd_display_on(void) {
    lcd_write_cmd(LCD_CMD_DISPON);
}

// 关闭显示
void lcd_display_off(void) {
    lcd_write_cmd(LCD_CMD_DISPOFF);
}

// 设置背光
void lcd_set_backlight(bool on) {
    gpio_put(LCD_BL_PIN, on);
}

// 底层SPI写入函数
static void lcd_write_byte(uint8_t data) {
    spi_write_blocking(LCD_SPI_PORT, &data, 1);
}

static void lcd_write_bytes(const uint8_t* data, size_t len) {
    spi_write_blocking(LCD_SPI_PORT, data, len);
} 