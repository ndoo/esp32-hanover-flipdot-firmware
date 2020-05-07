#ifndef Hanover_Flipdot_h
#define Hanover_Flipdot_h

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include "SPI.h"
#include "Wire.h"

/*
  PCB Revisions
  * Issue A - PCB color is green
  * Issue B - PCB color is black
  * Issue C - PCB color is blue
*/
#define PCB_ISSUE_C

// Dimensions

// Dot-board dimensions
#define DB_ROWS 32
#define DB_COLS 32

// Number of dot-boards
#define DB_BOARDS 4

// Other dot board settings
#define DB_INVERT 1
#define DB_THROTTLE 0 // For aesthetics, to save power, or to give time for core magnetic saturation

// Delay to ensure stability for >1 dot board (exponential for every additional board)
#define DB_SEQ_THROTTLE_COIL 15 // Extra pulse time for each dot board coil pulse

// Timings (µS) - you can decrease coil pulse timings for an increase in speed, but dots may not reliably flip
#define PULSE_COIL_ON 180  // Note that these are after inversion
#define PULSE_COIL_OFF 150

#if defined(PCB_ISSUE_B) || defined(PCB_ISSUE_C)

// GPIO pin definitions - dot board
#define PIN_ENABLE1 15
#define PIN_ENABLE2 13
#define PIN_ENABLE3 12
#define PIN_ENABLE4 14
#define PIN_ROW_ADVANCE 33
#define PIN_COL_ADVANCE_ROW_RESET 27
#define PIN_COL_RESET 26
#define PIN_SET_UNSET 25
#define PIN_COIL_DRIVE 2

// GPIO pin definitions - status LEDs
#define LED_DEBUG true
#define PIN_LED_A 23
#define PIN_LED_B 19
#define PIN_LED_C 22

#endif

#ifdef PCB_ISSUE_A // Green PCB

// Other dot board settings
#define DB_INVERT 1
#define DB_THROTTLE 0 // For aesthetics, to save power, or to give time for core magnetic saturation

// GPIO pin definitions - dot board
#define PIN_ENABLE1 2
#define PIN_ENABLE2 15
#define PIN_ENABLE3 13
#define PIN_ENABLE4 12
#define PIN_ROW_ADVANCE 25
#define PIN_COL_ADVANCE_ROW_RESET 14
#define PIN_COL_RESET 27
#define PIN_SET_UNSET 26
#define PIN_COIL_DRIVE 33

// GPIO pin definitions - status LEDs
#define LED_DEBUG true
#define PIN_LED_A 32 // Actually wired to IO34 but it is input-only
#define PIN_LED_B 32 // Actually wired to IO35 but it is input-only
#define PIN_LED_C 32

#endif

typedef uint32_t db_t[DB_COLS * DB_BOARDS];

// Public subs
void flipdot_init();

class Hanover_Flipdot : public Adafruit_GFX
{
public:
    Hanover_Flipdot(void);
    void begin();
    void drawPixel(int16_t x, int16_t y, uint16_t color);
    void writeDisplay(void);
    void clear(void);
    void backup(void);
    void restore(void);
    void fillScreen(uint16_t color);
    uint8_t getWidth(void);
    uint8_t getHeight(void);
    db_t db_buffer;

private:
    void enable(uint8_t db);
    void flipDot(bool state, uint8_t pulse_time);
    void advanceRow(void);
    void advanceCol(void);
    db_t db_backup;
    db_t db_displayed;
};

#endif // Hanover_Flipdot_h