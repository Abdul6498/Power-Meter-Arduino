
#include <LiquidCrystal.h>
#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

int sensorVolt = A0;
int sensorTA12 = A1; // Analog input pin that sensor is attached to

int voltage;
float nVPP;   // Voltage measured across resistor
float nCurrThruResistorPP; // Peak Current Measured Through Resistor
float nCurrThruResistorRMS; // RMS current through Resistor
float nCurrentThruWire;     // Actual RMS current in Wire
float energy,total_energy, energy_recived, total_energy_received;
unsigned int sec = 0;
int Power, Power_received;
int incomming = 0;
void setup() 
 {
   Serial.begin(9600);
   lcd.begin(16, 2);
    Mirf.spi = &MirfHardwareSpi;
    Mirf.init();
    Mirf.setRADDR((byte *)"serv1"); 
    Mirf.payload = sizeof(unsigned long); 
    Mirf.config();
   
   pinMode(sensorTA12, INPUT);
   pinMode(sensorVolt, INPUT);
   
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
   energy = (Power*2.5)/(3600);
   delay(1);
   Serial.println(sec);  
   total_energy = energy + total_energy;
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
   Serial.println();
   
   lcd_display();
   
  byte data[Mirf.payload];   
  if(!Mirf.isSending() && Mirf.dataReady()){
    Serial.println("Got packet");
    Mirf.getData(data);
    Serial.println(char (data[0]));
    incomming = data[0];
 }
   Power_received = map (incomming, 0, 255, 0, 1000);
   energy_recived = (Power_received*2.5)/(3600);
   delay(1);
   Serial.println(sec);  
   total_energy_received = energy_recived + total_energy_received;
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
 
void lcd_display()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("V:");
  lcd.print(voltage);
  lcd.print("I:");
  lcd.print(nCurrentThruWire);
  lcd.print("mA");
  
  lcd.setCursor(0,1);
  lcd.print("E1:");
  lcd.print(total_energy);
  lcd.print("Wh");
  lcd.print("E2:");
  lcd.print(total_energy_received);
  lcd.print("Wh");
}
