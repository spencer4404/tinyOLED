// tinyOLED.c
//  This code serves as an SSD1306 OLED & I2C Library for ATtiny MCU
// Tested with ATtiny28 & 0.96" 128x64 OLED
// modified from the original by D. McLaughlin 12/22/23
// Original code 2020 by Stefan Wagner can be found here:
// Project Files (Github):  https://github.com/wagiminatorb
// License: http://creativecommons.org/licenses/by-sa/3.0/
// Changes made: separate single file into .c and .h files;
// used Main/Secondary nomenclature; other small changes

#include "tinyOLED.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

// OLED init settings
const uint8_t OLED_INIT_CMD[] PROGMEM = {
    0xA8, 0x1F,       // set multiplex (HEIGHT-1): 0x1F for 128x32, 0x3F for 128x64
    0x22, 0x00, 0x03, // set min and max page
    0x20, 0x00,       // set horizontal memory addressing mode
    0xDA, 0x02,       // set COM pins hardware configuration to sequential
    0x8D, 0x14,       // enable charge pump
    0xAF,             // switch on OLED
    0xA1, 0xC8};

// standard ASCII 5x8 font (adapted from Neven Boyanov and Stephen Denne)
const uint8_t OLED_FONT[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, //   0
    0x00, 0x00, 0x2f, 0x00, 0x00, // ! 1
    0x00, 0x07, 0x00, 0x07, 0x00, // " 2
    0x14, 0x7f, 0x14, 0x7f, 0x14, // # 3
    0x24, 0x2a, 0x7f, 0x2a, 0x12, // $ 4
    0x62, 0x64, 0x08, 0x13, 0x23, // % 5
    0x36, 0x49, 0x55, 0x22, 0x50, // & 6
    0x00, 0x05, 0x03, 0x00, 0x00, // ' 7
    0x00, 0x1c, 0x22, 0x41, 0x00, // ( 8
    0x00, 0x41, 0x22, 0x1c, 0x00, // ) 9
    0x14, 0x08, 0x3E, 0x08, 0x14, // * 10
    0x08, 0x08, 0x3E, 0x08, 0x08, // + 11
    0x00, 0x00, 0xA0, 0x60, 0x00, // , 12
    0x08, 0x08, 0x08, 0x08, 0x08, // - 13
    0x00, 0x60, 0x60, 0x00, 0x00, // . 14
    0x20, 0x10, 0x08, 0x04, 0x02, // / 15
    0x3E, 0x51, 0x49, 0x45, 0x3E, // 0 16
    0x00, 0x42, 0x7F, 0x40, 0x00, // 1 17
    0x42, 0x61, 0x51, 0x49, 0x46, // 2 18
    0x21, 0x41, 0x45, 0x4B, 0x31, // 3 19
    0x18, 0x14, 0x12, 0x7F, 0x10, // 4 20
    0x27, 0x45, 0x45, 0x45, 0x39, // 5 21
    0x3C, 0x4A, 0x49, 0x49, 0x30, // 6 22
    0x01, 0x71, 0x09, 0x05, 0x03, // 7 23
    0x36, 0x49, 0x49, 0x49, 0x36, // 8 24
    0x06, 0x49, 0x49, 0x29, 0x1E, // 9 25
    0x00, 0x36, 0x36, 0x00, 0x00, // : 26
    0x00, 0x56, 0x36, 0x00, 0x00, // ; 27
    0x08, 0x14, 0x22, 0x41, 0x00, // < 28
    0x14, 0x14, 0x14, 0x14, 0x14, // = 29
    0x00, 0x41, 0x22, 0x14, 0x08, // > 30
    0x02, 0x01, 0x51, 0x09, 0x06, // ? 31
    0x32, 0x49, 0x59, 0x51, 0x3E, // @ 32
    0x7C, 0x12, 0x11, 0x12, 0x7C, // A 33
    0x7F, 0x49, 0x49, 0x49, 0x36, // B 34
    0x3E, 0x41, 0x41, 0x41, 0x22, // C 35
    0x7F, 0x41, 0x41, 0x22, 0x1C, // D 36
    0x7F, 0x49, 0x49, 0x49, 0x41, // E 37
    0x7F, 0x09, 0x09, 0x09, 0x01, // F 38
    0x3E, 0x41, 0x49, 0x49, 0x7A, // G 39
    0x7F, 0x08, 0x08, 0x08, 0x7F, // H 40
    0x00, 0x41, 0x7F, 0x41, 0x00, // I 41
    0x20, 0x40, 0x41, 0x3F, 0x01, // J 42
    0x7F, 0x08, 0x14, 0x22, 0x41, // K 43
    0x7F, 0x40, 0x40, 0x40, 0x40, // L 44
    0x7F, 0x02, 0x0C, 0x02, 0x7F, // M 45
    0x7F, 0x04, 0x08, 0x10, 0x7F, // N 46
    0x3E, 0x41, 0x41, 0x41, 0x3E, // O 47
    0x7F, 0x09, 0x09, 0x09, 0x06, // P 48
    0x3E, 0x41, 0x51, 0x21, 0x5E, // Q 49
    0x7F, 0x09, 0x19, 0x29, 0x46, // R 50
    0x46, 0x49, 0x49, 0x49, 0x31, // S 51
    0x01, 0x01, 0x7F, 0x01, 0x01, // T 52
    0x3F, 0x40, 0x40, 0x40, 0x3F, // U 53
    0x1F, 0x20, 0x40, 0x20, 0x1F, // V 54
    0x3F, 0x40, 0x38, 0x40, 0x3F, // W 55
    0x63, 0x14, 0x08, 0x14, 0x63, // X 56
    0x07, 0x08, 0x70, 0x08, 0x07, // Y 57
    0x61, 0x51, 0x49, 0x45, 0x43, // Z 58
    0x00, 0x7F, 0x41, 0x41, 0x00, // [ 59
    0x02, 0x04, 0x08, 0x10, 0x20, // \ 60
    0x00, 0x41, 0x41, 0x7F, 0x00, // ] 61
    0x04, 0x02, 0x01, 0x02, 0x04, // ^ 62
    0x40, 0x40, 0x40, 0x40, 0x40  // _ 63
};

// I2C init function
void I2C_init(void)
{
  DDRB &= ~((1 << I2C_SDA) | (1 << I2C_SCL));  // pins as input (HIGH-Z) -> lines released
  PORTB &= ~((1 << I2C_SDA) | (1 << I2C_SCL)); // should be LOW when as ouput
}

// I2C transmit one data byte to the secondary, ignore ACK bit, no clock stretching allowed
void I2C_write(uint8_t data)
{
  for (uint8_t i = 8; i; i--)
  {                // transmit 8 bits, MSB first
    I2C_SDA_LOW(); // SDA LOW for now (saves some flash this way)
    if (data & 0x80)
      I2C_SDA_HIGH(); // SDA HIGH if bit is 1
    I2C_SCL_HIGH();   // clock HIGH -> secondary reads the bit
    data <<= 1;       // shift left data byte, acts also as a delay
    I2C_SCL_LOW();    // clock LOW again
  }
  I2C_SDA_HIGH(); // release SDA for ACK bit of secondary
  I2C_SCL_HIGH(); // 9th clock pulse is for the ACK bit
  // asm("nop");         // ACK bit is ignored, just a delay COMMENTED OUT DM 3/20/24
  I2C_SCL_LOW(); // clock LOW again
}

// I2C start transmission
void I2C_start(uint8_t addr)
{
  I2C_SDA_LOW();   // start condition: SDA goes LOW first
  I2C_SCL_LOW();   // start condition: SCL goes LOW second
  I2C_write(addr); // send secondary address
}

// I2C stop transmission
void I2C_stop(void)
{
  I2C_SDA_LOW();  // prepare SDA for LOW to HIGH transition
  I2C_SCL_HIGH(); // stop condition: SCL goes HIGH first
  I2C_SDA_HIGH(); // stop condition: SDA goes HIGH second
}

// OLED init function
void OLED_init(void)
{
  I2C_init();               // initialize I2C first
  I2C_start(OLED_ADDR);     // start transmission to OLED
  I2C_write(OLED_CMD_MODE); // set command mode
  for (uint8_t i = 0; i < OLED_INIT_LEN; i++)
    I2C_write(pgm_read_byte(&OLED_INIT_CMD[i])); // send the command bytes
  I2C_stop();                                    // stop transmission
}

// OLED set vertical shift
void OLED_shift(uint8_t ypos)
{
  I2C_start(OLED_ADDR);     // start transmission to OLED
  I2C_write(OLED_CMD_MODE); // set command mode
  I2C_write(0xD3);          // vertical shift command
  I2C_write(ypos);          // set vertical shift value
  I2C_stop();               // stop transmission
}

// OLED print a character
void OLED_printC(char ch)
{
  I2C_start(OLED_ADDR);      // start transmission to OLED
  I2C_write(OLED_DAT_MODE);  // set data mode
  uint16_t offset = ch - 32; // calculate position of character in font array
  offset += offset << 2;     // -> offset = (ch - 32) * 5
  I2C_write(0x00);           // print spacing between characters
  for (uint8_t i = 5; i; i--)
    I2C_write(pgm_read_byte(&OLED_FONT[offset++])); // print character
  I2C_stop();
}

// OLED print a string from program memory
void OLED_printP(const char *p)
{
  I2C_start(OLED_ADDR);       // start transmission to OLED
  I2C_write(OLED_DAT_MODE);   // set data mode
  char ch = pgm_read_byte(p); // read first character from program memory
  while (ch != 0)
  {                          // repeat until string terminator
    OLED_printC(ch);         // print character on OLED
    ch = pgm_read_byte(++p); // read next character
  }
  I2C_stop(); // stop transmission
}

// OLED set the cursor
void OLED_cursor(uint8_t xpos, uint8_t ypos)
{
  I2C_start(OLED_ADDR);            // start transmission to OLED
  I2C_write(OLED_CMD_MODE);        // set command mode
  I2C_write(xpos & 0x0F);          // set low nibble of start column
  I2C_write(0x10 | (xpos >> 4));   // set high nibble of start column
  I2C_write(0xB0 | (ypos & 0x07)); // set start page
  I2C_stop();                      // stop transmission
}

// OLED clear screen
void OLED_clear(void)
{
  OLED_cursor(0, 0);        // set cursor at upper left corner
  I2C_start(OLED_ADDR);     // start transmission to OLED
  I2C_write(OLED_DAT_MODE); // set data mode
  for (uint16_t i = 512; i; i--)
    I2C_write(0x00); // clear the screen
  I2C_stop();        // stop transmission
  OLED_shift(0);     // reset vertical shift
}
