#include <Hanover_Flipdot.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <rom/rtc.h>

static const uint8_t pin_enable[] = {PIN_ENABLE1, PIN_ENABLE2, PIN_ENABLE3, PIN_ENABLE4};

Hanover_Flipdot::Hanover_Flipdot(void) : Adafruit_GFX(DB_COLS * DB_BOARDS, DB_ROWS)
{
}

void Hanover_Flipdot::begin(void)
{

    // Set GPIO pins of dot board to output
    pinMode(PIN_ENABLE1, OUTPUT);
    pinMode(PIN_ENABLE2, OUTPUT);
    pinMode(PIN_ENABLE3, OUTPUT);
    pinMode(PIN_ENABLE4, OUTPUT);
    pinMode(PIN_ROW_ADVANCE, OUTPUT);
    pinMode(PIN_COL_ADVANCE_ROW_RESET, OUTPUT);
    pinMode(PIN_COL_RESET, OUTPUT);
    pinMode(PIN_SET_UNSET, OUTPUT);
    pinMode(PIN_COIL_DRIVE, OUTPUT);

    // Set GPIO pins of status LEDs to output
    pinMode(PIN_LED_A, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);
    pinMode(PIN_LED_C, OUTPUT);

    // Set GPIO pins of dot board to low (to avoid potentially overheating coils)
    digitalWrite(PIN_COL_RESET, LOW);
    digitalWrite(PIN_SET_UNSET, LOW);
    digitalWrite(PIN_COIL_DRIVE, LOW);

    if (rtc_get_reset_reason(0) == 1 || rtc_get_reset_reason(0) == 15)
    { // POWERON_RESET || RTCWDT_BROWN_OUT_RESET
        // Lamp test while waiting for power to stabilize (if cold boot)
        digitalWrite(PIN_LED_A, HIGH);
        digitalWrite(PIN_LED_B, HIGH);
        digitalWrite(PIN_LED_C, HIGH);
        sleep(1);
    }

    // Set GPIO pins of status LEDs
    digitalWrite(PIN_LED_A, LOW);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_C, LOW);

    // Lamp test / force clear
    fillScreen(1);
    writeDisplay();
    clear();
    writeDisplay();
}

uint8_t Hanover_Flipdot::getWidth(void)
{
    return DB_COLS * DB_BOARDS;
}
uint8_t Hanover_Flipdot::getHeight(void)
{
    return DB_ROWS;
}

void Hanover_Flipdot::enable(uint8_t db)
{
    // Hold row and column counters in reset
    digitalWrite(PIN_COL_ADVANCE_ROW_RESET, HIGH);
    digitalWrite(PIN_COL_RESET, HIGH);

    // Latch/unlatch dot board
    for (uint8_t i = 0; i < DB_BOARDS; i++)
        digitalWrite(pin_enable[i], i == db ? HIGH : LOW);

    // Clear row and column resets
    digitalWrite(PIN_COL_ADVANCE_ROW_RESET, LOW);
    digitalWrite(PIN_COL_RESET, LOW); // Clear column reset after row, as row reset advances column
}

void Hanover_Flipdot::advanceRow(void)
{
    digitalWrite(PIN_ROW_ADVANCE, HIGH);
    if (LED_DEBUG)
        digitalWrite(PIN_LED_B, HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_ROW_ADVANCE, LOW);
    if (LED_DEBUG)
        digitalWrite(PIN_LED_B, LOW);
    delayMicroseconds(1);
}

void Hanover_Flipdot::advanceCol(void)
{
    digitalWrite(PIN_COL_ADVANCE_ROW_RESET, HIGH);
    if (LED_DEBUG)
        digitalWrite(PIN_LED_C, HIGH);
    delayMicroseconds(1);
    digitalWrite(PIN_COL_ADVANCE_ROW_RESET, LOW);
    if (LED_DEBUG)
        digitalWrite(PIN_LED_C, LOW);
    delayMicroseconds(1);
}

void Hanover_Flipdot::flipDot(bool state, uint8_t pulse_time)
{
    digitalWrite(PIN_SET_UNSET, state ^ DB_INVERT ? HIGH : LOW);
    digitalWrite(PIN_COIL_DRIVE, HIGH);
    if (LED_DEBUG)
        digitalWrite(PIN_LED_A, HIGH);

    delayMicroseconds(pulse_time);

    digitalWrite(PIN_COIL_DRIVE, LOW);
    if (LED_DEBUG)
        digitalWrite(PIN_LED_A, LOW);

    delayMicroseconds(DB_THROTTLE);
}

void Hanover_Flipdot::clear(void)
{
    for (uint8_t i = 0; i < DB_COLS * DB_BOARDS; i++)
        db_buffer[i] = 0;
}

void Hanover_Flipdot::writeDisplay(void)
{
    // Iterate dot boards
    for (uint8_t db = 0; db < DB_BOARDS; db++)
    {
        enable(db);

        uint8_t col_start = db * DB_COLS;
        uint8_t col_end = col_start + DB_COLS;

        // Iterate columns
        uint8_t col_backlog = 0;
        for (uint8_t col = col_start; col < col_end; col++)
        {
            // Iterate rows
            uint8_t row_backlog = 0;
            for (uint8_t row = 0; row < DB_ROWS; row++)
            {
                bool db_buffer_bit = db_buffer[col] & (1 << row);
                bool db_displayed_bit = db_displayed[col] & (1 << row);
                if (db_displayed_bit != db_buffer_bit)
                {
                    // Increment to match
                    for (uint8_t cb = 0; cb < col_backlog; cb++)
                        advanceCol();
                    for (uint8_t rb = 0; rb < row_backlog; rb++)
                        advanceRow();
                    row_backlog = 0;
                    col_backlog = 0;
                    uint16_t pulse_time = pow(db, 2) * DB_SEQ_THROTTLE_COIL + (db_buffer_bit ? PULSE_COIL_ON : PULSE_COIL_OFF);
                    flipDot(db_buffer_bit, pulse_time);
                }
                row_backlog++;
            } // Iterate rows
            db_displayed[col] = db_buffer[col];
            Serial.println();
            col_backlog++;
        } // Iterate columns
    }     // Iterate dot boards
}

// Drawing functions (Adafruit GFX Primitives)

void Hanover_Flipdot::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if ((y < 0) || (y >= DB_ROWS))
        return;
    if ((x < 0) || (x >= DB_COLS * DB_BOARDS))
        return;

    if (color)
    {
        db_buffer[x] |= 1 << y;
    }
    else
    {
        db_buffer[x] &= ~(1 << y);
    }
}

void Hanover_Flipdot::fillScreen(uint16_t color)
{
    for (uint8_t i = 0; i < DB_COLS * DB_BOARDS; i++)
        db_buffer[i] = pow(2, DB_ROWS) - 1;
}