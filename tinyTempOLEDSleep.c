// main.c
/* Test code for ECE-304 2024 Digital Thermometer Circuit based on
TMP36, ATtiny85, and 0.96" 128x64 I2C OLED */

#define NUMSAMPLES 25
#define VREF 1.1
#define HIGHTEMP 75
#include "tinyOLED.h"
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>
void adc_init(void);
unsigned int get_adc(void);
void WDT_OFF(void);
#include <avr/wdt.h>
#include <avr/sleep.h>

// messages to print on OLED
const char Message1[] PROGMEM = " DEG F";
const char Message2[] PROGMEM = " DEG C";

int main(void)
{

  WDT_OFF();
  char buffer[6];
  float temperature;
  unsigned int digitalValue, tempx10;
  unsigned long int totalValue;
  DDRB = 1 << DDB4;

  OLED_init(); // initialize the OLED
  OLED_clear();
  adc_init();

  // acquire and average NUMSAMPLES temp measurements
  totalValue = 0;
  for (int i = 0; i < NUMSAMPLES; i++)
  {
    totalValue += get_adc(); // Get a sample from temp sensor
    _delay_ms(5);
  }
  digitalValue = totalValue / NUMSAMPLES;
  temperature = digitalValue * 1.0 * VREF / 10.24 - 50.; // Convert digital value to degrees C
  temperature = temperature * 9. / 5. + 32.;             // Convert degrees C to degrees F
  tempx10 = temperature * 10.;

  // Show temp in degrees F
  itoa(tempx10, buffer, 10); // Convert dig value to character string
  OLED_cursor(20, 1);
  OLED_printC(tempx10 / 100 + '0');     // hundreds Digit
  OLED_printC(tempx10 / 10 % 10 + '0'); // tens digit
  OLED_printC('.');
  OLED_printC(tempx10 % 10 + '0'); // 10's digit
  if (temperature > HIGHTEMP)
    PORTB |= 1 << PB4; // Turn overheat LED on
  else
    PORTB &= ~(1 << PB4); // Turn overheat LED off
  OLED_cursor(45, 1);     // set cursor position
  OLED_printP(Message1);  // print message 1

/*
  // Show temp in degrees C
  temperature = digitalValue * 1.0 * VREF / 10.24 - 50.; // Convert digital value to degrees C
  tempx10 = temperature * 10.;
  itoa(tempx10, buffer, 10); // Convert dig value to character string
  OLED_cursor(20, 3);
  OLED_printC(tempx10 / 100 + '0');     // hundreds Digit
  OLED_printC(tempx10 / 10 % 10 + '0'); // tens digit
  OLED_printC('.');
  OLED_printC(tempx10 % 10 + '0'); // 10's digit
  OLED_cursor(45, 3);              // set cursor position
  OLED_printP(Message2);           // print message 1
*/
  wdt_enable(WDTO_8S);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_mode();
  while (1)
    ;
}

// Initialize ADC peripheral: select ADC input & Vref
// Enable ADC; set speed to 125 KHz for a 16 MHz clock
void adc_init(void)
{
  DDRB &= ~(1 << 3); // Make ADC3 = PORTB3 an input pin
  ADCSRA = 0x83;     // Enable ADC; set speed to 125 KHZ for 1 MHZ clock
  if (VREF == 1.1)
  {
    ADMUX = 0x83; // Input is ADC3; Vref = 1.1V
  }
  else
  {
    ADMUX = 0x83; // Input is ADC3; Vref = 1.1
  }
}
// Read ADC value
unsigned int get_adc(void)
{
  ADCSRA |= (1 << ADSC); // start ADC conversion
  while ((ADCSRA & (1 << ADIF)) == 0)
    ;
  // ADCSRA &= ~(1<<ADIF); // cleat ADIF flag ***
  // _delay_ms(10);
  return ADCL | (ADCH << 8); // read ADCL first !
}

// Disable WTD and clear reset flag immediately at startup
void WDT_OFF()
{
    MCUSR &= ~(1 << WDRF);
    WDTCR = (1 << WDCE) | (1 << WDE);
    WDTCR = 0x00;
}


/*

// Calculate temperature, return 10*temperature as an int
int calculate_temp(unsigned int digitalValue, unsigned char tempScale)
{
  float temperature;; //, tempC, tempF;
  temperature = digitalValue * VREF / 10.24 - 50.; // Convert digital value to degrees C
  temperature = temperature * 9. / 5. + 32.;             // Convert degrees C to degrees F


  if (tempScale == 0)
  {
    temperature = tempC;  // Button pressed so use deg C
    PORTB |= 1 << PORTB4; // Turn on the C/F LED
  }
  else
  {
    temperature = tempF;     // Button not pressed so use deg F
    PORTB &= ~(1 << PORTB4); // Turn off the C/F LED
  }




  return temperature * 10.; // Convert to 3 digit integer
}

*/
