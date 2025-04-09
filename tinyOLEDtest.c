#include "tinyOLED.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

// messages to print on OLED
const char Label[] PROGMEM = "TEMP:";

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

// get rolling average temp
float get_avg_temp(uint8_t num_samples)
{
  float total = 0.0;
  for (uint8_t i = 0; i < num_samples; i++)
  {
    uint16_t adc = ADC_read();
    float voltage = (adc / 1023.0) * 3.3; // 3.3 volts
    float tempC = (voltage - 0.5) * 100.0;
    total += tempC;
    _delay_ms(10); // optional small delay between readings
  }
  return total / num_samples;
}

int main(void)
{
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

    float avgC = get_avg_temp(25);
    float avgF = avgC * 9.0 / 5.0 + 32.0;

    char tempF_str[6];
    char tempC_str[6];
    dtostrf(avgF, 4, 1, tempF_str);
    dtostrf(avgC, 4, 1, tempC_str);

    OLED_cursor(0, 1);
    OLED_printP(Label);

    OLED_cursor(40, 1);
    for (uint8_t i = 0; i < strlen(tempF_str); i++)
      OLED_printC(tempF_str[i]);
    OLED_printC('F');

    OLED_printC(' ');
    OLED_printC('/');
    OLED_printC(' ');

    for (uint8_t i = 0; i < strlen(tempC_str); i++)
      OLED_printC(tempC_str[i]);
    OLED_printC('C');

    _delay_ms(500);
  }
}
