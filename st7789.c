//#include "fonts.h"
#include <stdio.h>
#include <math.h>
#include <st7789.h>
#include "lvgl.h"

// Display dimensions
static int16_t _width = ST7789_WIDTH;
static int16_t _height = ST7789_HEIGHT;
static uint8_t _rotation = 0;

// Text cursor
static TextCursor_t cursor = {0, 0, ST77XX_WHITE, ST77XX_BLACK, 1, true};

// Hardware control macros
#define TFT_RST_LOW()   HAL_GPIO_WritePin(TFT_RST_PORT, TFT_RST_PIN, GPIO_PIN_RESET)
#define TFT_RST_HIGH()  HAL_GPIO_WritePin(TFT_RST_PORT, TFT_RST_PIN, GPIO_PIN_SET)
#define TFT_DC_LOW()    HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_RESET)
#define TFT_DC_HIGH()   HAL_GPIO_WritePin(TFT_DC_PORT, TFT_DC_PIN, GPIO_PIN_SET)
#define TFT_CS_LOW()    HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_RESET)
#define TFT_CS_HIGH()   HAL_GPIO_WritePin(TFT_CS_PORT, TFT_CS_PIN, GPIO_PIN_SET)


#define ST7789_Y_OFFSET 20   // or 40 depending on your panel
// Low-level SPI functions
static void TFT_WriteCommand(uint8_t cmd) {
    TFT_DC_LOW();
    TFT_CS_LOW();
    HAL_SPI_Transmit(&TFT_SPI_PORT, &cmd, 1, HAL_MAX_DELAY);
    TFT_CS_HIGH();
}

static void TFT_WriteData(uint8_t data) {
    TFT_DC_HIGH();
    TFT_CS_LOW();
    HAL_SPI_Transmit(&TFT_SPI_PORT, &data, 1, HAL_MAX_DELAY);
    TFT_CS_HIGH();
}

static void TFT_WriteDataMultiple(uint8_t* data, uint32_t len) {
    TFT_DC_HIGH();
    TFT_CS_LOW();
    HAL_SPI_Transmit(&TFT_SPI_PORT, data, len, HAL_MAX_DELAY);
    TFT_CS_HIGH();
}


static void TFT_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // Apply vertical offset so (0,0) logical is mapped into the visible area
    y0 += ST7789_Y_OFFSET;
    y1 += ST7789_Y_OFFSET;

    TFT_WriteCommand(ST7789_CASET);
    TFT_WriteData(x0 >> 8);
    TFT_WriteData(x0 & 0xFF);
    TFT_WriteData(x1 >> 8);
    TFT_WriteData(x1 & 0xFF);

    TFT_WriteCommand(ST7789_RASET);
    TFT_WriteData(y0 >> 8);
    TFT_WriteData(y0 & 0xFF);
    TFT_WriteData(y1 >> 8);
    TFT_WriteData(y1 & 0xFF);

    TFT_WriteCommand(ST7789_RAMWR);
}


// Initialize display - Adafruit style
void TFT_Init(uint16_t width, uint16_t height) {
    _width = width;
    _height = height;

    TFT_CS_HIGH();
    TFT_RST_HIGH();
    HAL_Delay(10);
    TFT_RST_LOW();
    HAL_Delay(10);
    TFT_RST_HIGH();
    HAL_Delay(120);

    //Serial_Println("TFT_SWRESET");
      TFT_WriteCommand(ST7789_SWRESET);
      HAL_Delay(150);

     // Serial_Println("TFT_SLPOUT");
      TFT_WriteCommand(ST7789_SLPOUT);
      HAL_Delay(120);

     // Serial_Println("TFT_COLMOD 16-bit");
      TFT_WriteCommand(ST7789_COLMOD);
      TFT_WriteData(0x55);

    //  Serial_Println("TFT_MADCTL");
      TFT_WriteCommand(ST7789_MADCTL);
      TFT_WriteData(0x00);

    TFT_WriteCommand(ST7789_INVON);
    HAL_Delay(10);

    TFT_WriteCommand(ST7789_NORON);
    HAL_Delay(10);

    TFT_WriteCommand(ST7789_DISPON);
    HAL_Delay(120);
}

void TFT_SetRotation(uint8_t rotation) {
    _rotation = rotation % 4;
    TFT_WriteCommand(ST7789_MADCTL);

    switch (_rotation) {
        case ROTATION_0:
            TFT_WriteData(0x00);
            _width = ST7789_WIDTH;
            _height = ST7789_HEIGHT;
            break;
        case ROTATION_90:
            TFT_WriteData(0x60);
            _width = ST7789_HEIGHT;
            _height = ST7789_WIDTH;
            break;
        case ROTATION_180:
            TFT_WriteData(0xC0);
            _width = ST7789_WIDTH;
            _height = ST7789_HEIGHT;
            break;
        case ROTATION_270:
            TFT_WriteData(0xA0);
            _width = ST7789_HEIGHT;
            _height = ST7789_WIDTH;
            break;
    }
}

void TFT_FillScreen(uint16_t color) {
    TFT_FillRect(0, 0, _width, _height, color);
}

void TFT_DrawPixel(int16_t x, int16_t y, uint16_t color) {
    if (x < 0 || x >= _width || y < 0 || y >= _height) return;

    TFT_SetWindow(x, y, x, y);

    uint8_t data[2];
    data[0] = color >> 8;
    data[1] = color & 0xFF;

    TFT_WriteDataMultiple(data, 2);
}

void TFT_FillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    // Bounds checking
    if (x >= _width || y >= _height || w <= 0 || h <= 0) return;
    if (x < 0) { w += x; x = 0; }
    if (y < 0) { h += y; y = 0; }
    if (x + w > _width) w = _width - x;
    if (y + h > _height) h = _height - y;

    TFT_SetWindow(x, y, x + w - 1, y + h - 1);

    uint32_t totalPixels = (uint32_t)w * (uint32_t)h;

    // OPTIMIZED: Use buffer to reduce SPI calls
    uint16_t pixelBuffer[64];  // 128 bytes buffer
    for (int i = 0; i < 64; i++) {
        pixelBuffer[i] = color;
    }

    TFT_DC_HIGH();
    TFT_CS_LOW();

    // Send in chunks
    while (totalPixels >= 64) {
        HAL_SPI_Transmit(&TFT_SPI_PORT, (uint8_t *)pixelBuffer, 128, HAL_MAX_DELAY);
        totalPixels -= 64;
    }

    // Send remaining pixels
    if (totalPixels > 0) {
        HAL_SPI_Transmit(&TFT_SPI_PORT, (uint8_t *)pixelBuffer, totalPixels * 2, HAL_MAX_DELAY);
    }

    TFT_CS_HIGH();
}


void TFT_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);

    if (steep) {
        int16_t temp = x0; x0 = y0; y0 = temp;
        temp = x1; x1 = y1; y1 = temp;
    }

    if (x0 > x1) {
        int16_t temp = x0; x0 = x1; x1 = temp;
        temp = y0; y0 = y1; y1 = temp;
    }

    int16_t dx = x1 - x0;
    int16_t dy = abs(y1 - y0);
    int16_t err = dx / 2;
    int16_t ystep = (y0 < y1) ? 1 : -1;

    for (; x0 <= x1; x0++) {
        if (steep) {
            TFT_DrawPixel(y0, x0, color);
        } else {
            TFT_DrawPixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void TFT_DrawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    TFT_FillRect(x, y, 1, h, color);
}

void TFT_DrawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    TFT_FillRect(x, y, w, 1, color);
}

void TFT_DrawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
    TFT_DrawFastHLine(x, y, w, color);
    TFT_DrawFastHLine(x, y + h - 1, w, color);
    TFT_DrawFastVLine(x, y, h, color);
    TFT_DrawFastVLine(x + w - 1, y, h, color);
}

void TFT_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    TFT_DrawPixel(x0, y0 + r, color);
    TFT_DrawPixel(x0, y0 - r, color);
    TFT_DrawPixel(x0 + r, y0, color);
    TFT_DrawPixel(x0 - r, y0, color);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        TFT_DrawPixel(x0 + x, y0 + y, color);
        TFT_DrawPixel(x0 - x, y0 + y, color);
        TFT_DrawPixel(x0 + x, y0 - y, color);
        TFT_DrawPixel(x0 - x, y0 - y, color);
        TFT_DrawPixel(x0 + y, y0 + x, color);
        TFT_DrawPixel(x0 - y, y0 + x, color);
        TFT_DrawPixel(x0 + y, y0 - x, color);
        TFT_DrawPixel(x0 - y, y0 - x, color);
    }
}

void TFT_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    TFT_DrawFastVLine(x0, y0 - r, 2 * r + 1, color);
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        TFT_DrawFastVLine(x0 + x, y0 - y, 2 * y + 1, color);
        TFT_DrawFastVLine(x0 - x, y0 - y, 2 * y + 1, color);
        TFT_DrawFastVLine(x0 + y, y0 - x, 2 * x + 1, color);
        TFT_DrawFastVLine(x0 - y, y0 - x, 2 * x + 1, color);
    }
}

void TFT_DrawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    TFT_DrawLine(x0, y0, x1, y1, color);
    TFT_DrawLine(x1, y1, x2, y2, color);
    TFT_DrawLine(x2, y2, x0, y0, color);
}

void TFT_FillTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        int16_t temp = y0; y0 = y1; y1 = temp;
        temp = x0; x0 = x1; x1 = temp;
    }
    if (y1 > y2) {
        int16_t temp = y2; y2 = y1; y1 = temp;
        temp = x2; x2 = x1; x1 = temp;
    }
    if (y0 > y1) {
        int16_t temp = y0; y0 = y1; y1 = temp;
        temp = x0; x0 = x1; x1 = temp;
    }

    if (y0 == y2) {
        a = b = x0;
        if (x1 < a) a = x1;
        else if (x1 > b) b = x1;
        if (x2 < a) a = x2;
        else if (x2 > b) b = x2;
        TFT_DrawFastHLine(a, y0, b - a + 1, color);
        return;
    }

    int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0,
            dx12 = x2 - x1, dy12 = y2 - y1;
    int32_t sa = 0, sb = 0;

    last = (y1 == y2) ? y1 : y1 - 1;

    for (y = y0; y <= last; y++) {
        a = x0 + sa / dy01;
        b = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        if (a > b) {
            int16_t temp = a; a = b; b = temp;
        }
        TFT_DrawFastHLine(a, y, b - a + 1, color);
    }

    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for (; y <= y2; y++) {
        a = x1 + sa / dy12;
        b = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        if (a > b) {
            int16_t temp = a; a = b; b = temp;
        }
        TFT_DrawFastHLine(a, y, b - a + 1, color);
    }
}

void TFT_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    TFT_DrawFastHLine(x + r, y, w - 2 * r, color);
    TFT_DrawFastHLine(x + r, y + h - 1, w - 2 * r, color);
    TFT_DrawFastVLine(x, y + r, h - 2 * r, color);
    TFT_DrawFastVLine(x + w - 1, y + r, h - 2 * r, color);

    // Draw corners
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x1 = 0;
    int16_t y1 = r;

    while (x1 < y1) {
        if (f >= 0) {
            y1--;
            ddF_y += 2;
            f += ddF_y;
        }
        x1++;
        ddF_x += 2;
        f += ddF_x;

        TFT_DrawPixel(x + r + x1, y + r - y1, color);
        TFT_DrawPixel(x + r - y1, y + r + x1, color);
        TFT_DrawPixel(x + w - r - 1 - x1, y + r - y1, color);
        TFT_DrawPixel(x + w - r - 1 + y1, y + r + x1, color);
        TFT_DrawPixel(x + r + x1, y + h - r - 1 + y1, color);
        TFT_DrawPixel(x + r - y1, y + h - r - 1 - x1, color);
        TFT_DrawPixel(x + w - r - 1 - x1, y + h - r - 1 + y1, color);
        TFT_DrawPixel(x + w - r - 1 + y1, y + h - r - 1 - x1, color);
    }
}

void TFT_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color) {
    TFT_FillRect(x + r, y, w - 2 * r, h, color);

    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x1 = 0;
    int16_t y1 = r;

    while (x1 < y1) {
        if (f >= 0) {
            y1--;
            ddF_y += 2;
            f += ddF_y;
        }
        x1++;
        ddF_x += 2;
        f += ddF_x;

        TFT_DrawFastVLine(x + r - y1, y + r - x1, 2 * x1 + 1 + h - 2 * r, color);
        TFT_DrawFastVLine(x + r + y1, y + r - x1, 2 * x1 + 1 + h - 2 * r, color);
        TFT_DrawFastVLine(x + w - r - 1 - y1, y + r - x1, 2 * x1 + 1 + h - 2 * r, color);
        TFT_DrawFastVLine(x + w - r - 1 + y1, y + r - x1, 2 * x1 + 1 + h - 2 * r, color);
    }
}

// Text functions
void TFT_SetCursor(int16_t x, int16_t y) {
    cursor.x = x;
    cursor.y = y;
}

void TFT_SetTextColor(uint16_t color) {
    cursor.textcolor = color;
    cursor.textbgcolor = color;
}

void TFT_SetTextColorBg(uint16_t color, uint16_t bg) {
    cursor.textcolor = color;
    cursor.textbgcolor = bg;
}

void TFT_SetTextSize(uint8_t size) {
    cursor.textsize = (size > 0) ? size : 1;
}

void TFT_SetTextWrap(bool wrap) {
    cursor.wrap = wrap;
}


void TFT_Print(const char* str) {
    while (*str) {
        if (*str == '\n') {
            cursor.y += 8 * cursor.textsize;
            cursor.x = 0;
        } else if (*str == '\r') {
            // Skip
        } else {
            TFT_DrawChar(cursor.x, cursor.y, *str, cursor.textcolor, cursor.textbgcolor, cursor.textsize);
            cursor.x += 6 * cursor.textsize;
            if (cursor.wrap && (cursor.x + 6 * cursor.textsize > _width)) {
                cursor.x = 0;
                cursor.y += 8 * cursor.textsize;
            }
        }
        str++;
    }
}

void TFT_Println(const char* str) {
    TFT_Print(str);
    cursor.y += 8 * cursor.textsize;
    cursor.x = 0;
}

void TFT_PrintInt(int32_t num) {
    char buffer[12];
    sprintf(buffer, "%ld", num);
    TFT_Print(buffer);
}

void TFT_PrintFloat(float num, uint8_t decimals) {
    char buffer[20];
    char format[10];
    sprintf(format, "%%.%df", decimals);
    sprintf(buffer, format, num);
    TFT_Print(buffer);
}

uint16_t TFT_Color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

int16_t TFT_Width(void) {
    return _width;
}

int16_t TFT_Height(void) {
    return _height;
}

//////////////////////////////////

void TFT_WritePixels(uint16_t *data, uint32_t len) {
    TFT_DC_HIGH();
    TFT_CS_LOW();
    HAL_SPI_Transmit(&TFT_SPI_PORT, (uint8_t *)data, len * 2, HAL_MAX_DELAY);
    TFT_CS_HIGH();
}


// LVGL display flush callback
void st7789_lv_flush(lv_disp_drv_t *disp_drv,
                     const lv_area_t *area,
                     lv_color_t *color_p)
{
    int32_t x1 = area->x1;
    int32_t y1 = area->y1;
    int32_t x2 = area->x2;
    int32_t y2 = area->y2;

    // Clip to screen bounds (safety check)
    if (x2 < 0 || y2 < 0 || x1 >= _width || y1 >= _height) {
        lv_disp_flush_ready(disp_drv);
        return;
    }
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= _width) x2 = _width - 1;
    if (y2 >= _height) y2 = _height - 1;

    // drawing window
    TFT_SetWindow(x1, y1, x2, y2);

    // Calculating pixel count
    uint32_t w = x2 - x1 + 1;
    uint32_t h = y2 - y1 + 1;
    uint32_t total = w * h;

    // Sending pixel data (LVGL uses RGB565, same as ST7789)
    TFT_DC_HIGH();
    TFT_CS_LOW();
    HAL_SPI_Transmit(&TFT_SPI_PORT,
                     (uint8_t *)color_p,
                     total * 2,  // 2 bytes per pixel
                     HAL_MAX_DELAY);
    TFT_CS_HIGH();

    // Tell LVGL we're done
    lv_disp_flush_ready(disp_drv);
}

/* ================= LVGL BUFFER ================= */

static lv_disp_draw_buf_t draw_buf;

/* 20 lines buffer (safe for STM32WB55 RAM) */
static lv_color_t buf[240 * 20];


void LVGL_Display_Init(void)
{
    lv_init();

    /* draw buffer */
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 240 * 20);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = 240;
    disp_drv.ver_res = 280;
    disp_drv.flush_cb = st7789_lv_flush;
    disp_drv.draw_buf = &draw_buf;

    lv_disp_drv_register(&disp_drv);
}

