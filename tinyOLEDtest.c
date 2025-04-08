#include "tinyOLED.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

// messages to print on OLED
const char Label[] PROGMEM = "VOLTS:";

// Initialize ADC
void ADC_init(void)
{
  ADMUX = (1 << MUX1) | (1 << MUX0);                  // use ADC3 (PB3 / pin 2)
  ADCSRA = (1 << ADEN) | (1 << ADPS1) | (1 << ADPS0); // enable ADC, prescaler 8
}

// Read analog pin (PB3 / ADC3)
uint16_t ADC_read(void)
{
  ADCSRA |= (1 << ADSC); // start conversion
  while (ADCSRA & (1 << ADSC))
    ; // wait until done
  return ADC;
}

int main(void)
{
  char buffer[12];
  OLED_init(); // initialize the OLED
  ADC_init();

  I2C_start(OLED_ADDR);
  I2C_write(OLED_CMD_MODE);
  I2C_write(0xA1); // segment remap
  I2C_write(0xC8); // COM scan direction
  I2C_stop();

  OLED_clear();
  while (1)
  {
    uint16_t adc = ADC_read();
    float voltage = (adc / 1023.0) * 3.3; // if Vcc is 3.3V

    dtostrf(voltage, 4, 2, buffer); // convert float to string, 2 decimal places

    OLED_cursor(0, 1);

    OLED_printP(Label);

    OLED_cursor(40, 1);
    for (uint8_t i = 0; i < strlen(buffer); i++)
      OLED_printC(buffer[i]);

    _delay_ms(500);
  }
}
