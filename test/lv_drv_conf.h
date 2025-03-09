#ifndef LV_DRV_CONF_H
#define LV_DRV_CONF_H

#include "lv_conf.h"

/* SDL 显示器驱动 */
#ifndef USE_SDL
#  define USE_SDL         1
#endif

#if USE_SDL
/* SDL 缓冲区大小 */
#  define SDL_HOR_RES     LCD_WIDTH
#  define SDL_VER_RES     LCD_HEIGHT

/* 鼠标支持 */
#  define SDL_MOUSE_SUPPORT     1
#endif

#endif /* LV_DRV_CONF_H */ 