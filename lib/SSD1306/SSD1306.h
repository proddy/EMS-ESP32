/*!
 * This is part of for Adafruit's SSD1306 library for monochrome
 * OLED displays: http://www.adafruit.com/category/63_98
 *
 * This (trimmed) build supports I2C only. SPI code has been removed.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries, with
 * contributions from the open source community.
 *
 * BSD license, all text above, and the splash screen header file,
 * must be included in any redistribution.
 *
 */

#ifndef _SSD1306_H_
#define _SSD1306_H_

#include <Adafruit_GFX.h>
#include <Wire.h>

// Color values used with drawPixel() and friends
#define SSD1306_BLACK 0   ///< Draw 'off' pixels
#define SSD1306_WHITE 1   ///< Draw 'on' pixels
#define SSD1306_INVERSE 2 ///< Invert pixels

// SSD1306 command bytes (see datasheet)
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR 0x22
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETSTARTLINE 0x40

// Charge pump / vcc source for begin()
#define SSD1306_EXTERNALVCC 0x01  ///< External display voltage source
#define SSD1306_SWITCHCAPVCC 0x02 ///< Gen. display voltage from 3.3V

// Scrolling commands
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3

/*!
    @brief  Class that stores state and functions for interacting with
            SSD1306 OLED displays over I2C.
*/
class SSD1306 : public Adafruit_GFX {
  public:
    SSD1306(uint8_t w, uint8_t h, TwoWire * twi = &Wire, int8_t rst_pin = -1, uint32_t clkDuring = 400000UL, uint32_t clkAfter = 100000UL);

    ~SSD1306(void);

    bool         begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, uint8_t i2caddr = 0, bool reset = true);
    void         display(void);
    void         clearDisplay(void);
    void         invertDisplay(bool i);
    void         dim(bool dim);
    void         drawPixel(int16_t x, int16_t y, uint16_t color);
    virtual void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
    virtual void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
    void         startscrollright(uint8_t start, uint8_t stop);
    void         startscrollleft(uint8_t start, uint8_t stop);
    void         stopscroll(void);
    void         ssd1306_command(uint8_t c);
    bool         getPixel(int16_t x, int16_t y);
    uint8_t *    getBuffer(void);

  protected:
    void drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color);
    void drawFastVLineInternal(int16_t x, int16_t y, int16_t h, uint16_t color);
    void ssd1306_command1(uint8_t c);
    void ssd1306_commandList(const uint8_t * c, uint8_t n);

    TwoWire * wire;    ///< Initialized during construction. See Wire.cpp, Wire.h
    uint8_t * buffer;  ///< Display buffer, allocated in begin().
    int8_t    i2caddr; ///< I2C address, set in begin().
    int8_t    vccstate;
    int8_t    rstPin;     ///< Display reset pin assignment, or -1 if unused.
    uint32_t  wireClk;    ///< Wire speed during SSD1306 transfers
    uint32_t  restoreClk; ///< Wire speed to restore after transfers
    uint8_t   contrast;   ///< normal contrast setting for this device
};

#endif // _SSD1306_H_
