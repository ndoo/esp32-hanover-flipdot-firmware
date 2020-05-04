#ifndef Hanover_Flipdot_h
#define Hanover_Flipdot_h

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "SPI.h"
#include "Wire.h"

// Dimensions

// Dot-board dimensions
#define DB_ROWS 32
#define DB_COLS 32

// Number of dot-boards
#define DB_X 1
#define DB_Y 1

// Other dot board settings
#define DB_INVERT 1
#define DB_THROTTLE 100 // For aesthetics, to save power, or to give time for core magnetic saturation

// GPIO pin definitions - dot board
#define PIN_ENABLE1 15
#define PIN_ENABLE2 13
#define PIN_ENABLE3 12
#define PIN_ENABLE4 14
#define PIN_ROW_ADVANCE 33
#define PIN_COL_ADVANCE 27
#define PIN_RESET 26
#define PIN_SET_UNSET 25
#define PIN_COIL_DRIVE 2

// GPIO pin definitions - status LEDs
#define PIN_LED_A 23
#define PIN_LED_B 19
#define PIN_LED_C 22

// Timings (µS)
#define PULSE_ENABLE_WAIT 10 // Capacitance on disable line causes stray dot activation on previous dotboard
#define PULSE_RST_HIGH 10
#define PULSE_ROW_HIGH 10
#define PULSE_COL_HIGH 20
#define PULSE_LOW 10
#define PULSE_COIL_ON 180
#define PULSE_COIL_OFF 130

// Buffers
typedef uint8_t db_column_t[DB_ROWS * DB_Y];
typedef db_column_t db_t[DB_COLS * DB_X];

// Public subs
void flipdot_init();
void update_dotboard(db_t *dots, bool force);
void update_dotboard(db_t *dots);

class Hanover_Flipdot : public Adafruit_GFX
{
public:
    Hanover_Flipdot(void);
    void begin();
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void writeDisplayParallel(void);
    void writeDisplay(void);
    void clear(void);
    uint8_t getWidth(void);
    uint8_t getHeight(void);

private:
    void enable(uint8_t db);
    void disable(uint8_t db);
    void reset(void);
    void writeDot(uint8_t col, uint8_t row);
    void advanceRow(void);
    void advanceCol(void);
};

#endif // Hanover_Flipdot_h