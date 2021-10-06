#ifndef F_CPU
#define F_CPU 1600000UL
#endif
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <string.h>
#include "stdio.h"
#include <stdint.h>
#include "lcd.h"
#include "mirf.h"
#include "nRF24L01.h"

void Initialize()
{
mirf_init();
// Wait for mirf to come up
_delay_ms(50);
// Activate interrupts
sei();	
// Configure mirf		
mirf_config();

}

void adc_init()
{
    // AREF = AVcc
    ADMUX = (1<<REFS0);

    // ADC Enable and prescaler of 128
    
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// read adc value
uint16_t adc_read(uint8_t ch)
{
    // select the corresponding channel 0~7
   
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8)|ch;     

    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);

    // wait for conversion to complete
    // ADSC becomes '0' again
    
    while(ADCSRA & (1<<ADSC));

    return (ADC);
}
//Current 
float getVPP()
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
   while(start_time < 1000) //sample for 1 Sec
   {
       readValue = adc_read(1);;
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
	   	_delay_ms(1);
		start_time++;
   }
   
   // Convert the digital data to a voltage
   result = (maxValue * 5.0)/1024.0;
   return result;
 }
 
//Voltage 
 float getVolts()
{
  float Vrms;
  int Volt_Value;             //value read from the sensor
  int maxVolt = 0;          // store max value here
   uint32_t start_time = 0;
   while(start_time < 1000) //sample for 1 Sec
   {
       Volt_Value = adc_read(0);;
       // see if you have a new maxValue
       if (Volt_Value > maxVolt) 
       {
           /*record the maximum sensor value*/
           maxVolt = Volt_Value;
       }
		_delay_ms(1);
		start_time++;
   }
   Vrms = (((maxVolt * 0.0150)+0.7)/1.414)*20;
   return Vrms;
}
void lcd_display()
{
	 lcd_clrscr();
  lcd_gotoxy(0,0);
  lcd_puts("V:");
  itoa(voltage,buffer,10);
  lcd_puts(buffer);
  lcd_puts("I:");
  itoa(nCurrentThruWire,buffer,10);
  lcd_puts(buffer);
  lcd_puts("mA");
  
  lcd_gotoxy(0,1);
  lcd_puts("P:");
  itoa(Power,buffer,10);
  lcd_puts(buffer);
  lcd_puts("W");
  lcd_puts("E:");
  itoa(total_energy,buffer,10);
  lcd_puts(buffer);
  lcd_puts("Wh");	
}

void power_sender()
{	nrf_power = Power/4;
	itoa(nrf_power,nrf_buffer,10);
	mirf_send(nrf_buffer,buffersize);
	_delay_ms(200);
}

volatile unsigned int sensorVolt = 0, sensorCurrent = 0;
volatile int voltage;
volatile float nVPP;   // Voltage measured across resistor
volatile float nCurrThruResistorPP; // Peak Current Measured Through Resistor
volatile float nCurrThruResistorRMS; // RMS current through Resistor
volatile float nCurrentThruWire;     // Actual RMS current in Wire
volatile float energy,total_energy;
volatile unsigned int sec = 0;
volatile int Power,nrf_power;
volatile uint8_t * nrf_buffer[20];
volatile uint8_t buffersize = 20;
volatile char buffer[10];
int main()
{
	Initialize();
	adc_init();
    lcd_init(LCD_DISP_ON_CURSOR);

    
    lcd_clrscr();
	lcd_gotoxy(0,0);
	
    _delay_ms(50);
	   while(1)
    {
	voltage = getVolts();
   nVPP = getVPP();
   
   /*
   Use Ohms law to calculate current across resistor
   and express in mA 
   */
   
   nCurrThruResistorPP = (nVPP/200.0) * 1000.0;
   
   /* 
   Use Formula for SINE wave to convert
   to RMS 
   */
   
   nCurrThruResistorRMS = nCurrThruResistorPP * 0.707;
   
   /* 
   Current Transformer Ratio is 1000:1...
   
   Therefore current through 200 ohm resistor
   is multiplied by 1000 to get input current
   */
   
   nCurrentThruWire = nCurrThruResistorRMS * 1000;
	Power = nCurrThruResistorRMS  *  voltage;
   energy = (Power*2.5*1.05)/(3600);
    total_energy = energy + total_energy;
	
	lcd_display();
	power_sender();
	    }
}