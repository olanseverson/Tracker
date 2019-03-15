#include <SoftwareSerial.h>

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(10, 11); //SIM800L Tx & Rx is connected to Arduino #3 & #2

void setup()
{
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  
  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Initializing..."); 
  delay(1000);

  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();

  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  mySerial.println("AT+CMGS=\"+6282130310254\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms +6282130310254  +6285270817607
  updateSerial();
  
//  static float f_val = 123.6794;
//  static char outstr[15];
//  Serial.println(f_val);
//  dtostrf(f_val,8, 4, outstr);
//  Serial.println(outstr);
  
//  updateSerial();
  mySerial.print("Last Minute Engineers | lastminuteengineers.com"); //text content
  updateSerial();
  mySerial.write(26);
}

void loop()
{
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) 
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}
