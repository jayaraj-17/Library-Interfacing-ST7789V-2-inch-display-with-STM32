#ifndef ST7789_ADAFRUIT_H
#define ST7789_ADAFRUIT_H

#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// Display dimensions
#define ST7789_WIDTH  240
#define ST7789_HEIGHT 320

// Pin definitions - MODIFY ACCORDING TO YOUR CONNECTIONS
#define TFT_RST_PIN     GPIO_PIN_5
#define TFT_RST_PORT    GPIOB
#define TFT_DC_PIN      GPIO_PIN_10
#define TFT_DC_PORT     GPIOA
#define TFT_CS_PIN      GPIO_PIN_9
#define TFT_CS_PORT     GPIOB

// SPI Handle
extern SPI_HandleTypeDef hspi1;
#define TFT_SPI_PORT hspi1

// Color definitions (RGB565 format) - Adafruit compatible
#define ST77XX_BLACK       0x0000
#define ST77XX_WHITE       0xFFFF
#define ST77XX_RED         0xF800
#define ST77XX_GREEN       0x07E0
#define ST77XX_BLUE        0x001F
#define ST77XX_CYAN        0x07FF
#define ST77XX_MAGENTA     0xF81F
#define ST77XX_YELLOW      0xFFE0
#define ST77XX_ORANGE      0xFC00
#define ST77XX_PURPLE      0x8010
#define ST77XX_PINK        0xFE19
#define ST77XX_LIGHTGREY   0xC618
#define ST77XX_DARKGREY    0x7BEF

// Display commands
#define ST7789_NOP        0x00
#define ST7789_SWRESET    0x01
#define ST7789_SLPOUT     0x11
#define ST7789_NORON      0x13
#define ST7789_INVOFF     0x20
#define ST7789_INVON      0x21
#define ST7789_DISPOFF    0x28
#define ST7789_DISPON     0x29
#define ST7789_CASET      0x2A
#define ST7789_RASET      0x2B
#define ST7789_RAMWR      0x2C
#define ST7789_MADCTL     0x36
#define ST7789_COLMOD     0x3A

// Rotation values
#define ROTATION_0   0
#define ROTATION_90  1
#define ROTATION_180 2
#define ROTATION_270 3

// Text cursor structure
typedef struct {
    int16_t x;
    int16_t y;
    uint16_t textcolor;
    uint16_t textbgcolor;
    uint8_t textsize;
    bool wrap;
} TextCursor_t;

// Function prototypes - Adafruit GFX style
void TFT_Init(uint16_t width, uint16_t height);
void TFT_SetRotation(uint8_t rotation);

// Drawing functions
void TFT_FillScreen(uint16_t color);
void TFT_DrawPixel(int16_t x, int16_t y, uint16_t color);
void TFT_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void TFT_DrawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void TFT_DrawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void TFT_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void TFT_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void TFT_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void TFT_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);
void TFT_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void TFT_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);
void TFT_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void TFT_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);

// Text functions
void TFT_SetCursor(int16_t x, int16_t y);
void TFT_SetTextColor(uint16_t color);
void TFT_SetTextColorBg(uint16_t color, uint16_t bg);
void TFT_SetTextSize(uint8_t size);
void TFT_SetTextWrap(bool wrap);
void TFT_Print(const char* str);
void TFT_Println(const char* str);
void TFT_PrintInt(int32_t num);
void TFT_PrintFloat(float num, uint8_t decimals);

// Color conversion
uint16_t TFT_Color565(uint8_t r, uint8_t g, uint8_t b);

// Utility
int16_t TFT_Width(void);
int16_t TFT_Height(void);

#endif // ST7789_ADAFRUIT_H
