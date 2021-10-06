#include <LiquidCrystal595.h>
#include <Time.h>  
#include <Wire.h>  
#include <DS1307RTC.h>  // a basic DS1307 library that returns time as a time_t
#include <LiquidCrystal_I2C.h>
#include <SD.h>

#include <PN532_I2C.h>
#include <PN532.h>

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);



// initialize the library with the numbers of the interface pins + the row count
// datapin, latchpin, clockpin, num_lines
LiquidCrystal595 lcd(0,1,A3);
LiquidCrystal_I2C lcd1(0x20,16,2);  // set the LCD address to 0x3f for a 16 chars and 2 line display
const int chipSelect = 9;

int currentPin = A1;    // select the input pin for the potentiometer
int voltagePin = A0;
int voltage2 = 0;
int bill = 0;
int units = 8;
String dataString = "";
void setup() {
   cli();
   TCCR1A = 0;
   TCCR1B = 0;  //prescaler 1024
   TIMSK1 = 0;
   TCNT1 = 0;
   TIMSK1 |= (1<<TOIE1);
   attachInterrupt(0, start1, RISING);  //voltage_interrupt
   attachInterrupt(1, stop1, RISING);   //Current interrupt
   sei();
   lcd.begin(16,4);
   lcd1.init();   // initialize the lcd 
   lcd1.backlight();
    nfc.begin();
//**********To Set the Time************//
 /*   Wire.beginTransmission(0x68); // activate DS1307
    Wire.write(0); // where to begin
    Wire.write(0x00);          //seconds
    Wire.write(0x05);    //minutes
    Wire.write(0x21);    //hours (24hr time)
    Wire.write(0x01);  // Day 01-07
    Wire.write(0x12);  // Date 0-31
    Wire.write(0x04);  // month 0-12
    Wire.write(0x16);  // Year 00-99
    //Wire.write(0x10); // Control 0x10 produces a 1 HZ square wave on pin 7. 
    Wire.endTransmission(); 
*/
   //Serial.begin(9600);
   

  
   pinMode(currentPin, INPUT);
   pinMode(voltagePin, INPUT);
//   pinMode(2,INPUT);  //Voltage_freq_pin
   // make sure that the default chip select pin is set to
   // output, even if you don't use it:
   pinMode(10, OUTPUT);
   
   setSyncProvider(RTC.get);   // the function to get the time from the RTC
   if(timeStatus()!= timeSet) 
     lcd1.print("Unable to sync");
   else
     lcd1.print("RTC has set"); 
  // see if the card is present and can be initialized:
  lcd1.setCursor(0,1);
  if (!SD.begin(chipSelect)) {
    lcd1.print("Card failed");
    // don't do anything more:
    return;
  }
  lcd1.print("card initialized");
  
     //RFID PN532 board verification
     uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    lcd.print("NO RFID BOARD");
    lcd.setCursor(0,1);
  }
  else if(versiondata)
  {
  lcd.print("RFID is Working");
  }
    nfc.setPassiveActivationRetries(0xFF);
  
  // configure board to read RFID tags
  nfc.SAMConfig();  
  //delay(2000);
  lcd.clear();
  lcd1.clear();  
}

unsigned int total_freq = 0, time = 0 , theta = 0 , freq = 0;
unsigned int power = 0 , sec = 0, apparent_power = 0;
unsigned int reactive_power = 0;
float pf,theta_rad , energy , total_energy, cond = 0 , cap = 0;
float current = 0;  // variable to store the value coming from the sensor
float voltage = 0;  // variable to store the value coming from the sensor
unsigned int current_freq = 0 , total_current_freq = 0;
boolean success;
   //boolean success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
 
void loop() {
        
  
  for(int i =0; i<2; i++)
  {
  //Frequency Finder
  freq = pulseIn(2,HIGH);
  total_freq = freq + total_freq;
  }
  //Serial.println(freq);
  total_freq = total_freq/2;
  total_freq = (1000000/(2*total_freq));
  for(int i =0; i<2; i++)
  {
  //Frequency Finder
  current_freq = pulseIn(3,HIGH);
  total_current_freq = current_freq + total_current_freq;
  }
  //Serial.println(freq);
  total_current_freq = total_current_freq/2;
  total_current_freq = (1000000/(2*total_current_freq));
  // read the value from the sensor:
 //   lcd.setCursor(0,0);
//  lcd.print("Frequency:");
 // lcd.print(total_freq);
//  lcd.print("Hz");

    if (timeStatus() == timeSet) 
    {
    lcd1_print_I2C();
    } 
    else 
    {
      lcd1.print("Time NOT SET");
    //Serial.println("The time has not been set.  Please run the Time");
    //Serial.println("TimeRTCSet example, or DS1307RTC SetTime example.");
    //Serial.println();
    //delay(4000);
  }
  //delay(1000);
  current = analogRead(currentPin) - 445.5; //* 0.0048828125)-2.700)*10);
  current = current/10;
  voltage = analogRead(voltagePin)* 0.0150;
  voltage = voltage + 0.7;
  voltage = (voltage / sqrt(2))*20;
  apparent_power = voltage * current;
  theta = map(time,20,10050,0,90);
  theta_rad = theta*0.0174533;
  pf = cos(theta_rad);
  power = apparent_power * pf;
  reactive_power = sqrt((apparent_power*apparent_power)-(power*power));
  cond = (voltage*voltage)/reactive_power;
  cap = (1000000)/(2*3.14*total_freq*cond);
  sec = millis();
  energy = (apparent_power*sec*0.01)/(3600);
  delay(1);  
  total_energy = energy + total_energy;
  bill = total_energy * units;
  //Serial.print(current);
  //Serial.print("A");  
  //Serial.print(" ");
  //Serial.print(voltage);
  //Serial.print("V");
  //Serial.print(" ");
  //Serial.print(power);
  //Serial.print("W");
  //Serial.print(" ");
  //Serial.print(theta);
  //Serial.print("D");
  //Serial.print(" ");
  //Serial.print(pf);
  //Serial.print("p");
  //Serial.print(" ");
  //Serial.print(total_freq);
  //Serial.println("Hz");
  //Serial.print(" ");
  //Serial.print(energy);
  //Serial.println("WH");
  lcd_print_595();
  power_logger();
  
  //RFID CARD
    
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength; 
  //RFID CARD testing 
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);
    //RFID
   if (success) 
   {
       lcd.clear();
  lcd.print("Found a card!");
     
        if(uid[0] == 91 && uid[1] == 102 && uid[2] == 51 && uid[3] == 158)
    {
     RFID_bill_6498();
    }
        if(uid[0] == 203 && uid[1] == 134 && uid[2] == 57 && uid[3] == 181)
    {
     RFID_bill_6508();
    }
        if(uid[0] == 155 && uid[1] == 120 && uid[2] == 215 && uid[3] == 181)
    {
     RFID_bill_6162();     
    }
 //delay(5000);    
  //  Serial.println(""); 
   } 
  //RFID END

  //delay(1000);
  
}

void start1()
{
  //time = micros();
  TCNT1 = 0;
  TCCR1B |= (1<<CS11);
}

void stop1()
{
  TCCR1B = 0;
  time = TCNT1;
//TCNT1 = 0;
}
void lcd_print_595()
{
/***************Freq***************/
  lcd.setCursor(0,0);
  lcd.print("F1:");
  lcd.print(total_freq);
  lcd.print("Hz");
  lcd.print(" ");  
  lcd.print("F2:");
  lcd.print(total_current_freq);
  lcd.print("Hz");  
/**************current*************/
  lcd.setCursor(0,1);
  lcd.print("I:");
  lcd.print(current);
  //lcd.print("A");
/**************VOLTAGE*************/
  lcd.print(" ");
  lcd.print("V:");
  lcd.print(voltage);
  //lcd.print("v");
/***********POWER & ENERGY*********/
  lcd.setCursor(0,2);
  lcd.print("P:");
  lcd.print(power);
  lcd.print("W");
  lcd.print(" ");
  lcd.print("E:");
  lcd.print(total_energy);
  lcd.print("Wh");  
/*******Power Factor & THETa********/
  lcd.setCursor(0,3);
  lcd.print("A_P:");
  lcd.print(apparent_power);
  lcd.print("W");
 // lcd.print("Cap:");
 // lcd.print(cap);
 // lcd.print("uf");
  lcd.print(" ");
  lcd.print("PF=");
  lcd.print(pf);
}
void lcd1_print_I2C()
{
  lcd1.setCursor(0,0);
  lcd1.print(hour());
  printDigits(minute());
  printDigits(second());
  //Serial.print(" ");
  lcd1.setCursor(0,1);
  lcd1.print(day());
  lcd1.print("-");
  lcd1.print(month());
  lcd1.print("-");
  lcd1.print(year()); 
  //Serial.println(); 
}

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  lcd1.print(":");
  if(digits < 10)
    lcd1.print('0');
  lcd1.print(digits);
}

void power_logger()
{
  File dataFile = SD.open("datalog3.txt", FILE_WRITE);
    if (dataFile) {
    dataFile.print("Freq1 = ");
    dataFile.print(total_freq);
    dataFile.print("Hz , ");
    dataFile.print("Freq2 = ");
    dataFile.print(total_current_freq);
    dataFile.print("Hz , ");
    dataFile.print("Voltage=");
    dataFile.print(voltage);
    dataFile.print("volts,");
    dataFile.print("  Current=");
    dataFile.print(current);
    dataFile.print("Amp,");
    dataFile.print("  Power");
    dataFile.print(power);
    dataFile.print("Watt");
    dataFile.print(",");
    dataFile.print("  Energy=");
    dataFile.print(total_energy);
    dataFile.print("Wh,");  
  /*******Power Factor & THETa********/
    
    dataFile.print("  Apparent_power:");
    dataFile.print(apparent_power);
    dataFile.print("W,");
    dataFile.print("  Cap:");
    dataFile.print(cap);
    dataFile.print("uf,");
    dataFile.print("  PF=");
    dataFile.print(pf);
    dataFile.println(",");
    dataFile.close();
    }
}

//RFID card if available
void RFID_bill_6498()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("NAME = ABDUL");
    lcd.setCursor(0,1);
    lcd.print("No. = 6498");
    lcd.setCursor(0,2);
    lcd.print("Units = ");
    lcd.print(total_energy);
    lcd.setCursor(0,3);
    lcd.print("Bill=");
    lcd.print(bill);
    lcd.print("Rs");
    delay(5000);
  }
 
 void RFID_bill_6162()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("NAME = Waheed");
    lcd.setCursor(0,1);
    lcd.print("No. = 6162");
    lcd.setCursor(0,2);
    lcd.print("Units = ");
    lcd.print(total_energy);
    lcd.setCursor(0,3);
    lcd.print("Bill=");
    lcd.print(bill);
    lcd.print("Rs");
    delay(5000);
  }
 
 void RFID_bill_6508()
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("NAME = Fahad");
    lcd.setCursor(0,1);
    lcd.print("No. = 6508");
    lcd.setCursor(0,2);
    lcd.print("Units = ");
    lcd.print(total_energy);
    lcd.setCursor(0,3);
    lcd.print("Bill=");
    lcd.print(bill);
    lcd.print("Rs");
    delay(5000);
  } 
