/*!
 * @file SSD1306.cpp
 *
 * @mainpage Arduino library for monochrome OLEDs based on SSD1306 drivers.
 *
 * @section intro_sec Introduction
 *
 * This is documentation for Adafruit's SSD1306 library for monochrome
 * OLED displays. This (trimmed) build supports I2C only; SPI code has
 * been removed.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section dependencies Dependencies
 *
 * This library depends on <a
 * href="https://github.com/adafruit/Adafruit-GFX-Library"> Adafruit_GFX</a>
 * being present on your system. Please make sure you have installed the latest
 * version before using this library.
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries, with
 * contributions from the open source community.
 *
 * @section license License
 *
 * BSD license, all text above, and the splash screen included below,
 * must be included in any redistribution.
 *
 */

#include "SSD1306.h"
#include "splash.h"
#include <Adafruit_GFX.h>

// SOME DEFINES AND STATIC VARIABLES USED INTERNALLY -----------------------

#if defined(I2C_BUFFER_LENGTH)
#define WIRE_MAX min(256, I2C_BUFFER_LENGTH) ///< Particle or similar Wire lib
#elif defined(BUFFER_LENGTH)
#define WIRE_MAX min(256, BUFFER_LENGTH) ///< AVR or similar Wire lib
#elif defined(SERIAL_BUFFER_SIZE)
#define WIRE_MAX min(255, SERIAL_BUFFER_SIZE - 1) ///< Newer Wire uses RingBuffer
#else
#define WIRE_MAX 32 ///< Use common Arduino core default
#endif

#define ssd1306_swap(a, b) (((a) ^= (b)), ((b) ^= (a)), ((a) ^= (b))) ///< No-temp-var swap operation

#define WIRE_WRITE wire->write ///< Wire write function

// Bump Wire to wireClk during transfers, restore to restoreClk afterwards so
// other (slower) devices on the same bus still work.
#define TRANSACTION_START wire->setClock(wireClk);
#define TRANSACTION_END wire->setClock(restoreClk);

// CONSTRUCTORS, DESTRUCTOR ------------------------------------------------

/*!
    @brief  Constructor for I2C-interfaced SSD1306 displays.
    @param  w         Display width in pixels
    @param  h         Display height in pixels
    @param  twi       Pointer to an existing TwoWire instance (e.g. &Wire).
    @param  rst_pin   Reset pin, or -1 if not used.
    @param  clkDuring Wire clock (Hz) during SSD1306 transfers. Default 400 kHz.
    @param  clkAfter  Wire clock (Hz) to restore after transfers. Default 100 kHz.
    @note   Call begin() before use -- buffer allocation happens there.
*/
SSD1306::SSD1306(uint8_t w, uint8_t h, TwoWire * twi, int8_t rst_pin, uint32_t clkDuring, uint32_t clkAfter)
    : Adafruit_GFX(w, h)
    , wire(twi ? twi : &Wire)
    , buffer(NULL)
    , rstPin(rst_pin)
    , wireClk(clkDuring)
    , restoreClk(clkAfter) {
}

/*!
    @brief  Destructor for SSD1306 object.
*/
SSD1306::~SSD1306(void) {
    if (buffer) {
        free(buffer);
        buffer = NULL;
    }
}

// LOW-LEVEL UTILS ---------------------------------------------------------

/*!
    @brief  Issue a single command to the SSD1306. Protected; callers should
            bracket sequences with TRANSACTION_START / TRANSACTION_END.
*/
void SSD1306::ssd1306_command1(uint8_t c) {
    wire->beginTransmission(i2caddr);
    WIRE_WRITE((uint8_t)0x00); // Co = 0, D/C = 0
    WIRE_WRITE(c);
    wire->endTransmission();
}

/*!
    @brief  Issue a list of commands to the SSD1306.
    @param  c   pointer to list of commands
    @param  n   number of commands in the list
*/
void SSD1306::ssd1306_commandList(const uint8_t * c, uint8_t n) {
    wire->beginTransmission(i2caddr);
    WIRE_WRITE((uint8_t)0x00); // Co = 0, D/C = 0
    uint16_t bytesOut = 1;
    while (n--) {
        if (bytesOut >= WIRE_MAX) {
            wire->endTransmission();
            wire->beginTransmission(i2caddr);
            WIRE_WRITE((uint8_t)0x00); // Co = 0, D/C = 0
            bytesOut = 1;
        }
        WIRE_WRITE(pgm_read_byte(c++));
        bytesOut++;
    }
    wire->endTransmission();
}

/*!
    @brief  Issue a single low-level command directly to the SSD1306 display,
            bypassing the library. Wraps the command in a transaction.
*/
void SSD1306::ssd1306_command(uint8_t c) {
    TRANSACTION_START
    ssd1306_command1(c);
    TRANSACTION_END
}

// ALLOCATE & INIT DISPLAY -------------------------------------------------

/*!
    @brief  Allocate RAM for image buffer, initialize peripherals and pins.
    @param  vcs           SSD1306_SWITCHCAPVCC or SSD1306_EXTERNALVCC.
    @param  addr          I2C address (0 for default: 0x3C on 128x32, else 0x3D).
    @param  reset         If true and rstPin is valid, perform a hard reset.
    @param  periphBegin   If true, call wire->begin(); set false if the sketch
                          has already done so (e.g. custom SDA/SCL pins).
    @return true on successful allocation/init, false otherwise.
    @note   MUST call this function before any drawing or updates!
*/
bool SSD1306::begin(uint8_t vcs, uint8_t addr, bool reset) {
    if ((!buffer) && !(buffer = (uint8_t *)malloc(WIDTH * ((HEIGHT + 7) / 8))))
        return false;

    clearDisplay();

    // show splash
    drawBitmap((WIDTH - splash_width) / 2, (HEIGHT - splash_height) / 2, splash_data, splash_width, splash_height, 1);

    vccstate = vcs;

    // If I2C address is unspecified, use default
    // (0x3C for 32-pixel-tall displays, 0x3D for all others).
    i2caddr = addr ? addr : ((HEIGHT == 32) ? 0x3C : 0x3D);

    // Reset SSD1306 if requested and reset pin specified in constructor
    if (reset && (rstPin >= 0)) {
        pinMode(rstPin, OUTPUT);
        digitalWrite(rstPin, HIGH);
        delay(1);                   // VDD goes high at start, pause for 1 ms
        digitalWrite(rstPin, LOW);  // Bring reset low
        delay(10);                  // Wait 10 ms
        digitalWrite(rstPin, HIGH); // Bring out of reset
    }

    TRANSACTION_START

    // Init sequence
    static const uint8_t PROGMEM init1[] = {SSD1306_DISPLAYOFF,         // 0xAE
                                            SSD1306_SETDISPLAYCLOCKDIV, // 0xD5
                                            0x80,                       // the suggested ratio 0x80
                                            SSD1306_SETMULTIPLEX};      // 0xA8
    ssd1306_commandList(init1, sizeof(init1));
    ssd1306_command1(HEIGHT - 1);

    static const uint8_t PROGMEM init2[] = {SSD1306_SETDISPLAYOFFSET,   // 0xD3
                                            0x0,                        // no offset
                                            SSD1306_SETSTARTLINE | 0x0, // line #0
                                            SSD1306_CHARGEPUMP};        // 0x8D
    ssd1306_commandList(init2, sizeof(init2));

    ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x10 : 0x14);

    static const uint8_t PROGMEM init3[] = {SSD1306_MEMORYMODE, // 0x20
                                            0x00,               // 0x0 act like ks0108
                                            SSD1306_SEGREMAP | 0x1,
                                            SSD1306_COMSCANDEC};
    ssd1306_commandList(init3, sizeof(init3));

    uint8_t comPins = 0x02;
    contrast        = 0x8F;

    ssd1306_command1(SSD1306_SETCOMPINS);
    ssd1306_command1(comPins);
    ssd1306_command1(SSD1306_SETCONTRAST);
    ssd1306_command1(contrast);

    ssd1306_command1(SSD1306_SETPRECHARGE); // 0xd9
    ssd1306_command1((vccstate == SSD1306_EXTERNALVCC) ? 0x22 : 0xF1);
    static const uint8_t PROGMEM init5[] = {SSD1306_SETVCOMDETECT, // 0xDB
                                            0x40,
                                            SSD1306_DISPLAYALLON_RESUME, // 0xA4
                                            SSD1306_NORMALDISPLAY,       // 0xA6
                                            SSD1306_DEACTIVATE_SCROLL,
                                            SSD1306_DISPLAYON}; // Main screen turn on
    ssd1306_commandList(init5, sizeof(init5));

    TRANSACTION_END

    return true; // Success
}

// DRAWING FUNCTIONS -------------------------------------------------------

/*!
    @brief  Set/clear/invert a single pixel.
*/
void SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
        // Pixel is in-bounds. Rotate coordinates if needed.
        switch (getRotation()) {
        case 1:
            ssd1306_swap(x, y);
            x = WIDTH - x - 1;
            break;
        case 2:
            x = WIDTH - x - 1;
            y = HEIGHT - y - 1;
            break;
        case 3:
            ssd1306_swap(x, y);
            y = HEIGHT - y - 1;
            break;
        }
        switch (color) {
        case SSD1306_WHITE:
            buffer[x + (y / 8) * WIDTH] |= (1 << (y & 7));
            break;
        case SSD1306_BLACK:
            buffer[x + (y / 8) * WIDTH] &= ~(1 << (y & 7));
            break;
        case SSD1306_INVERSE:
            buffer[x + (y / 8) * WIDTH] ^= (1 << (y & 7));
            break;
        }
    }
}

/*!
    @brief  Clear contents of display buffer (set all pixels to off).
*/
void SSD1306::clearDisplay(void) {
    memset(buffer, 0, WIDTH * ((HEIGHT + 7) / 8));
}

/*!
    @brief  Draw a horizontal line.
*/
void SSD1306::drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
    bool bSwap = false;
    switch (rotation) {
    case 1:
        // 90 degree rotation, swap x & y for rotation, then invert x
        bSwap = true;
        ssd1306_swap(x, y);
        x = WIDTH - x - 1;
        break;
    case 2:
        // 180 degree rotation, invert x and y, then shift y around for height.
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        x -= (w - 1);
        break;
    case 3:
        // 270 degree rotation, swap x & y for rotation,
        // then invert y and adjust y for w (not to become h)
        bSwap = true;
        ssd1306_swap(x, y);
        y = HEIGHT - y - 1;
        y -= (w - 1);
        break;
    }

    if (bSwap)
        drawFastVLineInternal(x, y, w, color);
    else
        drawFastHLineInternal(x, y, w, color);
}

void SSD1306::drawFastHLineInternal(int16_t x, int16_t y, int16_t w, uint16_t color) {
    if ((y >= 0) && (y < HEIGHT)) { // Y coord in bounds?
        if (x < 0) {                // Clip left
            w += x;
            x = 0;
        }
        if ((x + w) > WIDTH) { // Clip right
            w = (WIDTH - x);
        }
        if (w > 0) { // Proceed only if width is positive
            uint8_t *pBuf = &buffer[(y / 8) * WIDTH + x], mask = 1 << (y & 7);
            switch (color) {
            case SSD1306_WHITE:
                while (w--) {
                    *pBuf++ |= mask;
                };
                break;
            case SSD1306_BLACK:
                mask = ~mask;
                while (w--) {
                    *pBuf++ &= mask;
                };
                break;
            case SSD1306_INVERSE:
                while (w--) {
                    *pBuf++ ^= mask;
                };
                break;
            }
        }
    }
}

/*!
    @brief  Draw a vertical line.
*/
void SSD1306::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
    bool bSwap = false;
    switch (rotation) {
    case 1:
        // 90 degree rotation, swap x & y for rotation,
        // then invert x and adjust x for h (now to become w)
        bSwap = true;
        ssd1306_swap(x, y);
        x = WIDTH - x - 1;
        x -= (h - 1);
        break;
    case 2:
        // 180 degree rotation, invert x and y, then shift y around for height.
        x = WIDTH - x - 1;
        y = HEIGHT - y - 1;
        y -= (h - 1);
        break;
    case 3:
        // 270 degree rotation, swap x & y for rotation, then invert y
        bSwap = true;
        ssd1306_swap(x, y);
        y = HEIGHT - y - 1;
        break;
    }

    if (bSwap)
        drawFastHLineInternal(x, y, h, color);
    else
        drawFastVLineInternal(x, y, h, color);
}

void SSD1306::drawFastVLineInternal(int16_t x, int16_t __y, int16_t __h, uint16_t color) {
    if ((x >= 0) && (x < WIDTH)) { // X coord in bounds?
        if (__y < 0) {             // Clip top
            __h += __y;
            __y = 0;
        }
        if ((__y + __h) > HEIGHT) { // Clip bottom
            __h = (HEIGHT - __y);
        }
        if (__h > 0) { // Proceed only if height is now positive
            // this display doesn't need ints for coordinates,
            // use local byte registers for faster juggling
            uint8_t   y = __y, h = __h;
            uint8_t * pBuf = &buffer[(y / 8) * WIDTH + x];

            // do the first partial byte, if necessary - this requires some masking
            uint8_t mod = (y & 7);
            if (mod) {
                // mask off the high n bits we want to set
                mod = 8 - mod;
                // note - lookup table results in a nearly 10% performance
                // improvement in fill* functions
                // uint8_t mask = ~(0xFF >> mod);
                static const uint8_t PROGMEM premask[8] = {0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, 0xFE};
                uint8_t                      mask       = pgm_read_byte(&premask[mod]);
                // adjust the mask if we're not going to reach the end of this byte
                if (h < mod)
                    mask &= (0XFF >> (mod - h));

                switch (color) {
                case SSD1306_WHITE:
                    *pBuf |= mask;
                    break;
                case SSD1306_BLACK:
                    *pBuf &= ~mask;
                    break;
                case SSD1306_INVERSE:
                    *pBuf ^= mask;
                    break;
                }
                pBuf += WIDTH;
            }

            if (h >= mod) { // More to go?
                h -= mod;
                // Write solid bytes while we can - effectively 8 rows at a time
                if (h >= 8) {
                    if (color == SSD1306_INVERSE) {
                        // separate copy of the code so we don't impact performance of
                        // black/white write version with an extra comparison per loop
                        do {
                            *pBuf ^= 0xFF; // Invert byte
                            pBuf += WIDTH; // Advance pointer 8 rows
                            h -= 8;        // Subtract 8 rows from height
                        } while (h >= 8);
                    } else {
                        // store a local value to work with
                        uint8_t val = (color != SSD1306_BLACK) ? 255 : 0;
                        do {
                            *pBuf = val;   // Set byte
                            pBuf += WIDTH; // Advance pointer 8 rows
                            h -= 8;        // Subtract 8 rows from height
                        } while (h >= 8);
                    }
                }

                if (h) { // Do the final partial byte, if necessary
                    mod = h & 7;
                    // this time we want to mask the low bits of the byte,
                    // vs the high bits we did above
                    // uint8_t mask = (1 << mod) - 1;
                    // note - lookup table results in a nearly 10% performance
                    // improvement in fill* functions
                    static const uint8_t PROGMEM postmask[8] = {0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F};
                    uint8_t                      mask        = pgm_read_byte(&postmask[mod]);
                    switch (color) {
                    case SSD1306_WHITE:
                        *pBuf |= mask;
                        break;
                    case SSD1306_BLACK:
                        *pBuf &= ~mask;
                        break;
                    case SSD1306_INVERSE:
                        *pBuf ^= mask;
                        break;
                    }
                }
            }
        } // endif positive height
    } // endif x in bounds
}

/*!
    @brief  Return color of a single pixel in display buffer.
*/
bool SSD1306::getPixel(int16_t x, int16_t y) {
    if ((x >= 0) && (x < width()) && (y >= 0) && (y < height())) {
        // Pixel is in-bounds. Rotate coordinates if needed.
        switch (getRotation()) {
        case 1:
            ssd1306_swap(x, y);
            x = WIDTH - x - 1;
            break;
        case 2:
            x = WIDTH - x - 1;
            y = HEIGHT - y - 1;
            break;
        case 3:
            ssd1306_swap(x, y);
            y = HEIGHT - y - 1;
            break;
        }
        return (buffer[x + (y / 8) * WIDTH] & (1 << (y & 7)));
    }
    return false; // Pixel out of bounds
}

/*!
    @brief  Get base address of display buffer for direct reading or writing.
*/
uint8_t * SSD1306::getBuffer(void) {
    return buffer;
}

// REFRESH DISPLAY ---------------------------------------------------------

/*!
    @brief  Push data currently in RAM to SSD1306 display.
*/
void SSD1306::display(void) {
    TRANSACTION_START
    static const uint8_t PROGMEM dlist1[] = {SSD1306_PAGEADDR,
                                             0,                   // Page start address
                                             0xFF,                // Page end (not really, but works here)
                                             SSD1306_COLUMNADDR}; // Column start address
    ssd1306_commandList(dlist1, sizeof(dlist1));

    if (WIDTH == 64) {
        ssd1306_command1(0x20);             // Column start
        ssd1306_command1(0x20 + WIDTH - 1); // Column end address
    } else {
        ssd1306_command1(0);           // Column start
        ssd1306_command1((WIDTH - 1)); // Column end address
    }

    uint16_t  count = WIDTH * ((HEIGHT + 7) / 8);
    uint8_t * ptr   = buffer;
    wire->beginTransmission(i2caddr);
    WIRE_WRITE((uint8_t)0x40);
    uint16_t bytesOut = 1;
    while (count--) {
        if (bytesOut >= WIRE_MAX) {
            wire->endTransmission();
            wire->beginTransmission(i2caddr);
            WIRE_WRITE((uint8_t)0x40);
            bytesOut = 1;
        }
        WIRE_WRITE(*ptr++);
        bytesOut++;
    }
    wire->endTransmission();
    TRANSACTION_END
}

// SCROLLING FUNCTIONS -----------------------------------------------------

/*!
    @brief  Activate a right-handed scroll for all or part of the display.
*/
// To scroll the whole display, run: display.startscrollright(0x00, 0x0F)
void SSD1306::startscrollright(uint8_t start, uint8_t stop) {
    TRANSACTION_START
    static const uint8_t PROGMEM scrollList1a[] = {SSD1306_RIGHT_HORIZONTAL_SCROLL, 0X00};
    ssd1306_commandList(scrollList1a, sizeof(scrollList1a));
    ssd1306_command1(start);
    ssd1306_command1(0X00);
    ssd1306_command1(stop);
    static const uint8_t PROGMEM scrollList1b[] = {0X00, 0XFF, SSD1306_ACTIVATE_SCROLL};
    ssd1306_commandList(scrollList1b, sizeof(scrollList1b));
    TRANSACTION_END
}

/*!
    @brief  Activate a left-handed scroll for all or part of the display.
*/
// To scroll the whole display, run: display.startscrollleft(0x00, 0x0F)
void SSD1306::startscrollleft(uint8_t start, uint8_t stop) {
    TRANSACTION_START
    static const uint8_t PROGMEM scrollList2a[] = {SSD1306_LEFT_HORIZONTAL_SCROLL, 0X00};
    ssd1306_commandList(scrollList2a, sizeof(scrollList2a));
    ssd1306_command1(start);
    ssd1306_command1(0X00);
    ssd1306_command1(stop);
    static const uint8_t PROGMEM scrollList2b[] = {0X00, 0XFF, SSD1306_ACTIVATE_SCROLL};
    ssd1306_commandList(scrollList2b, sizeof(scrollList2b));
    TRANSACTION_END
}

/*!
    @brief  Cease a previously-begun scrolling action.
*/
void SSD1306::stopscroll(void) {
    TRANSACTION_START
    ssd1306_command1(SSD1306_DEACTIVATE_SCROLL);
    TRANSACTION_END
}

// OTHER HARDWARE SETTINGS -------------------------------------------------

/*!
    @brief  Enable or disable display invert mode.
*/
void SSD1306::invertDisplay(bool i) {
    TRANSACTION_START
    ssd1306_command1(i ? SSD1306_INVERTDISPLAY : SSD1306_NORMALDISPLAY);
    TRANSACTION_END
}

/*!
    @brief  Dim the display.
*/
void SSD1306::dim(bool dim) {
    // the range of contrast is too small to be really useful,
    // but it is useful to dim the display
    TRANSACTION_START
    ssd1306_command1(SSD1306_SETCONTRAST);
    ssd1306_command1(dim ? 0 : contrast);
    TRANSACTION_END
}
