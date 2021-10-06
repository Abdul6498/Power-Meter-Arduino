//DFRobot.com
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal595.h>

// initialize the library with the numbers of the interface pins + the row count
// datapin, latchpin, clockpin, num_lines
LiquidCrystal595 lcd(0,1,A3);

LiquidCrystal_I2C lcd1(0x20,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display

void setup()
{
  lcd1.init();                      // initialize the lcd 
  lcd.begin(20,4);

  // Print a message to the LCD.
  lcd1.backlight();
  lcd1.print("Hello, world!");
    lcd.setCursor(0,0);
  lcd.print("This is line 1");
  delay(1000);
  
  lcd.setCursor(0,1);
  lcd.print("This is line 2");
  delay(1000);
  
  lcd.setCursor(0,2);
  lcd.print(127, HEX);
  delay(1000);
  
  lcd.setCursor(0,3);
  lcd.print("This is line ");
  lcd.print(4, BIN); // << Snagged from the Print class
  delay(1000);

}

void loop()
{
}
