#include <SoftwareSerial.h>
 #include <String.h>
 
//SIM800 TX is connected to Arduino D8
#define SIM800_TX_PIN 10
 
//SIM800 RX is connected to Arduino D7
#define SIM800_RX_PIN 11
 
//Create software serial object to communicate with SIM800
SoftwareSerial serialSIM800(SIM800_TX_PIN,SIM800_RX_PIN);
 int a[10] = {48,49,50,51,52,53,54,55,56,57};
String textForSMS;
float currentTemp;
  
  int i =0;
 int power = 339;
 int temp = 0;
void setup() {
  //Begin serial comunication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  while(!Serial);

  //Being serial communication witj Arduino and SIM800
  serialSIM800.begin(9600);
  delay(1000);
   
  Serial.println("Setup Complete!");
  Serial.println("Sending SMS...");
   
  //Set SMS format to ASCII
  serialSIM800.write("AT+CMGF=1\r\n");
  delay(1000);
 
  //Send new SMS command and message number
  serialSIM800.write("AT+CMGS=\"+923355818528\"\r\n");
  delay(1000);
  //Send SMS content
   convert_int();
  //Send Ctrl+Z / ESC to denote SMS message is complete
  serialSIM800.write((char)26);
  delay(1000);
     
  Serial.println("SMS Sent!");
}
 
void loop() {
}

void convert_int()
{
  serialSIM800.write("Your Current Power is = ");
  if(power < 100)
  {
    i = power/10;
    serialSIM800.write(a[i]);
    i = power%10;
    serialSIM800.write(a[i]);
  }

  else if(power>100 && power<1000)
  {
    i = power/100;
    serialSIM800.write(a[i]);
    temp = power - (i*100);
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

