#include <Hanover_Flipdot.h>
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>

static db_t db_displayed; // Displayed contents
static db_t db_buffer;    // Scratch buffer
bool fast_clear = false;

Hanover_Flipdot::Hanover_Flipdot(void) : Adafruit_GFX(DB_COLS * DB_X, DB_ROWS * DB_Y)
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
    digitalWrite(PIN_ENABLE1, LOW);
    digitalWrite(PIN_ENABLE2, LOW);
    digitalWrite(PIN_ENABLE3, LOW);
    digitalWrite(PIN_ENABLE4, LOW);
    digitalWrite(PIN_ROW_ADVANCE, LOW);
    digitalWrite(PIN_COL_ADVANCE_ROW_RESET, LOW);
    digitalWrite(PIN_COL_RESET, LOW);
    digitalWrite(PIN_SET_UNSET, LOW);
    digitalWrite(PIN_COIL_DRIVE, LOW);

    // Set GPIO pins of status LEDs to high (lamp test)
    digitalWrite(PIN_LED_A, LED_DEBUG ? HIGH : LOW);
    digitalWrite(PIN_LED_B, LED_DEBUG ? HIGH : LOW);
    digitalWrite(PIN_LED_C, LED_DEBUG ? HIGH : LOW);

    clear();
    writeDisplayParallel();
}

uint8_t Hanover_Flipdot::getWidth(void)
{
    return DB_COLS * DB_X;
}
uint8_t Hanover_Flipdot::getHeight(void)
{
    return DB_ROWS * DB_Y;
}

void Hanover_Flipdot::enable(uint8_t db)
{
    // Clear row and column resets
    digitalWrite(PIN_COL_ADVANCE_ROW_RESET, LOW);
    digitalWrite(PIN_COL_RESET, LOW); // Clear column reset after row, as calling row reset advances column

    // Enable (unlatch) dot board after clearing resets
    switch (db)
    {
    case 0:
        digitalWrite(PIN_ENABLE1, HIGH);
        break;
    case 1:
        digitalWrite(PIN_ENABLE2, HIGH);
        break;
    case 2:
        digitalWrite(PIN_ENABLE3, HIGH);
        break;
    case 3:
        digitalWrite(PIN_ENABLE4, HIGH);
        break;
    default:
        digitalWrite(PIN_ENABLE1, HIGH);
        digitalWrite(PIN_ENABLE2, HIGH);
        digitalWrite(PIN_ENABLE3, HIGH);
        digitalWrite(PIN_ENABLE4, HIGH);
        break;
    }
}

void Hanover_Flipdot::disable(uint8_t db)
{
    // Hold row and column counters in reset
    digitalWrite(PIN_COL_ADVANCE_ROW_RESET, HIGH);
    digitalWrite(PIN_COL_RESET, HIGH); // Hold column in reset after row, as calling row reset advances column

    // Latch dot board after holding reset to keep them reset state while pulsing other dot boards
    switch (db)
    {
    case 0:
        digitalWrite(PIN_ENABLE1, LOW);
        break;
    case 1:
        digitalWrite(PIN_ENABLE2, LOW);
        break;
    case 2:
        digitalWrite(PIN_ENABLE3, LOW);
        break;
    case 3:
        digitalWrite(PIN_ENABLE4, LOW);
        break;
    default:
        digitalWrite(PIN_ENABLE2, LOW);
        digitalWrite(PIN_ENABLE2, LOW);
        digitalWrite(PIN_ENABLE3, LOW);
        digitalWrite(PIN_ENABLE4, LOW);
        break;
    }
}

void Hanover_Flipdot::advanceRow(void)
{
    digitalWrite(PIN_ROW_ADVANCE, HIGH);
    if (LED_DEBUG) digitalWrite(PIN_LED_B, HIGH);
    delayMicroseconds(PULSE_ROW_HIGH);
    digitalWrite(PIN_ROW_ADVANCE, LOW);
    if (LED_DEBUG) digitalWrite(PIN_LED_B, LOW);
}

void Hanover_Flipdot::advanceCol(void)
{
    digitalWrite(PIN_COL_ADVANCE_ROW_RESET, HIGH);
    if (LED_DEBUG) digitalWrite(PIN_LED_C, HIGH);
    delayMicroseconds(PULSE_COL_HIGH);
    digitalWrite(PIN_COL_ADVANCE_ROW_RESET, LOW);
    if (LED_DEBUG) digitalWrite(PIN_LED_C, LOW);
}

void Hanover_Flipdot::writeDot(uint8_t col, uint8_t row)
{
    digitalWrite(PIN_SET_UNSET, db_buffer[col][row] ^ DB_INVERT ? HIGH : LOW);
    digitalWrite(PIN_COIL_DRIVE, HIGH);
    if (LED_DEBUG) digitalWrite(PIN_LED_A, HIGH);

    delayMicroseconds(db_buffer[col][row] ? PULSE_COIL_ON : PULSE_COIL_OFF);
    db_displayed[col][row] = db_buffer[col][row];

    digitalWrite(PIN_COIL_DRIVE, LOW);
    if (LED_DEBUG) digitalWrite(PIN_LED_A, LOW);


    delayMicroseconds(DB_THROTTLE);
}

void Hanover_Flipdot::writeDisplayParallel(void)
{
    enable(5);
    // Iterate dot board
    for (uint col = 0; col < DB_ROWS; col++)
    {
        for (uint row = 0; row < DB_COLS; row++)
        {
            writeDot(col, row);
            advanceRow();
        }

        if (col < DB_ROWS)
        {
            advanceCol();
        }
    }
    disable(5);
}

void Hanover_Flipdot::clear(void)
{
    // Initialize empty scratch to trigger update
    for (uint y = 0; y < DB_ROWS * DB_Y; y++)
    {
        for (uint x = 0; x < DB_COLS * DB_X; x++)
        {
            db_buffer[x][y] = 0;
        }
    }
}

void Hanover_Flipdot::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    db_buffer[x][y] = color == 0 ? 0 : 1;
}

void Hanover_Flipdot::writeDisplay(void)
{

    uint8_t db = 0;
    uint8_t row_backlog;
    uint8_t col_backlog;

    // For each dot board - x
    for (uint8_t db_x = 0; db_x < DB_X; db_x++)
    {
        // For each dot board - y
        for (uint8_t db_y = 0; db_y < DB_Y; db_y++)
        {

            enable(db);
            
            uint8_t col_start = db_x * DB_COLS;
            uint8_t col_end = col_start + DB_COLS;

            uint8_t row_start = db_y * DB_ROWS;
            uint8_t row_end = row_start + DB_ROWS;

            // Iterate dot board
            col_backlog = 0;
            for (uint8_t col = col_start; col < col_end; col++)
            {
                row_backlog = 0;
                for (uint8_t row = row_start; row < row_end; row++)
                {
                    if (db_buffer[col][row] != db_displayed[col][row]) {
                        
                        // Increment to match
                        for (uint8_t cb = 0; cb < col_backlog; cb++) advanceCol();
                        for (uint8_t rb = 0; rb < row_backlog; rb++) advanceRow();
                        row_backlog = 0;
                        col_backlog = 0;

                        writeDot(col, row);
                    }
                    row_backlog++;
                }
                col_backlog++;
            }
            // Disable dot board
            disable(db);
            db++;
        }
    }
}
