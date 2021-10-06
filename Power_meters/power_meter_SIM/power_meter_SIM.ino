
#include <LiquidCrystal.h>
#include <SoftwareSerial.h>
#include <EEPROM.h>
//SIM800 TX is connected to Arduino D8
#define SIM800_TX_PIN 10
 
//SIM800 RX is connected to Arduino D7
#define SIM800_RX_PIN 11
LiquidCrystal lcd(2, 3, 4, 5, 6, 7);
SoftwareSerial serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN);
 int a[10] = {48,49,50,51,52,53,54,55,56,57};
 

int sensorVolt = A0;
int sensorTA12 = A1; // Analog input pin that sensor is attached to

int voltage;
float nVPP;   // Voltage measured across resistor
float nCurrThruResistorPP; // Peak Current Measured Through Resistor
float nCurrThruResistorRMS; // RMS current through Resistor
float nCurrentThruWire;     // Actual RMS current in Wire
float energy,total_energy;
unsigned int sec = 0;
int Power;
int temp = 0;
int i = 0;
int energy_eeprom ,saved_energy;
void setup() 
 {
   Serial.begin(9600);
   lcd.begin(16, 2);
   serialSIM800.begin(9600);
    delay(1000);
   
   pinMode(sensorTA12, INPUT);
   pinMode(sensorVolt, INPUT);
         energy_eeprom = EEPROM.read(0);
         lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Stored E:");
      lcd.print(energy_eeprom);
      lcd.print("WH");
      lcd.setCursor(0,1);
      lcd.print("Please not it");
      delay(10000);
   
 }
 
 
 void loop() 
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

   
//   Serial.print("Volts Peak : ");
//   Serial.println(nVPP,3);
 
   
/*   Serial.print("Current Through Resistor (Peak) : ");
   Serial.print(nCurrThruResistorPP,3);
   Serial.println(" mA Peak to Peak");
   
   Serial.print("Current Through Resistor (RMS) : ");
   Serial.print(nCurrThruResistorRMS,3);
   Serial.println(" mA RMS");*/
   
   Power = nCurrThruResistorRMS  *  voltage;
   sec = millis()/1024;
   energy = (Power*8.5)/(3600);
   delay(1);
   Serial.println(sec);  
 
   total_energy = energy + total_energy;
 
     energy_eeprom = total_energy ;
     EEPROM.write(0, energy_eeprom);
 
   Serial.print("Current Through Wire : ");
   Serial.print(nCurrentThruWire,3);
   Serial.println(" mA RMS");
       
    Serial.print("AC Voltage : ");
   Serial.print(voltage);
   Serial.println(" VRMS");

    Serial.print("Power : ");
   Serial.print(Power);
   Serial.println(" Watt");

    Serial.print("Energy : ");
   Serial.print(total_energy);
   Serial.println(" Wh");
   
   Serial.print("Energy_eeprom : ");
     Serial.print(energy_eeprom);
    Serial.println();
   
   Serial.println();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.print(voltage);
  lcd.print(" I:");
  lcd.print(nCurrentThruWire);
  lcd.print("mA");
  
  lcd.setCursor(0,1);
  lcd.print("P:");
  lcd.print(Power);
  lcd.print("W");
  lcd.print("E:");
  lcd.print(total_energy);
  lcd.print("Wh");
 
  send_message_user();
  send_message_server();
 }


 /************************************ 
In order to calculate RMS current, we need to know
the peak to peak voltage measured at the output across the
200 Ohm Resistor

The following function takes one second worth of samples
and returns the peak value that is measured
 *************************************/
 
//Current 
float getVPP()
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       readValue = analogRead(sensorTA12);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
   }
   
   // Convert the digital data to a voltage
   result = (maxValue * 5.0)/1024.0;
  //result = map(maxValue, 0, 1024, 0, 230);
   return result;
 }
 
//Voltage 
 float getVolts()
{
  float Vrms;
  int Volt_Value;             //value read from the sensor
  int maxVolt = 0;          // store max value here
   uint32_t start_time = millis();
   while((millis()-start_time) < 1000) //sample for 1 Sec
   {
       Volt_Value = analogRead(sensorVolt);
       // see if you have a new maxValue
       if (Volt_Value > maxVolt) 
       {
           /*record the maximum sensor value*/
           maxVolt = Volt_Value;
       }
   }
   
   // Convert the digital data to a voltage
   //result = (maxValue * 5.0)/1024.0;
    Vrms = map(maxVolt, 0, 1024, 0, 230);
   return Vrms;
 }
 
 void send_message_user()
 {
   serialSIM800.write("AT+CMGF=1\r\n");
  delay(1000);
 
  //Send new SMS command and message number
  serialSIM800.write("AT+CMGS=\"+92433013924\"\r\n");
  delay(1000);
  //Send SMS content
   convert_int();
  //Send Ctrl+Z / ESC to denote SMS message is complete
  serialSIM800.write((char)26);
  delay(1000);
 }
 void send_message_server()
 {
   serialSIM800.write("AT+CMGF=1\r\n");
  delay(1000);
 
  //Send new SMS command and message number
  serialSIM800.write("AT+CMGS=\"+923218989261\"\r\n");
  delay(1000);
  //Send SMS content
   convert_int();
  //Send Ctrl+Z / ESC to denote SMS message is complete
  serialSIM800.write((char)26);
  delay(1000);
 }
 void convert_int()
{
  serialSIM800.write("Your Current Power is = ");
  if(Power < 100)
  {
    i = Power/10;
    serialSIM800.write(a[i]);
    i = Power%10;
    serialSIM800.write(a[i]);
  }

  else if(Power>100 && Power<1000)
  {
    i = Power/100;
    serialSIM800.write(a[i]);
    temp = Power - (i*100);
    i = temp/10;
    serialSIM800.write(a[i]);
    i =  temp%10;
    serialSIM800.write(a[i]);   
  }
  serialSIM800.write("Watt");
  delay(1000);
  temp = 0;
  i = 0;
}



