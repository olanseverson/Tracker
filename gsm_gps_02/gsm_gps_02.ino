#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include "LowPower.h"
/*
   This sample code demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = 8, TXPin = 9;  // 6, 5 RX TX
static const uint32_t GPSBaud = 4800; // 4800 untuk 3.3v

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

static float f_val = 0.0;
static char outstr[15];
static int count = 0;
char buffer[20];

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(10, 11); //SIM800L Tx & Rx is connected to Arduino #3 & #2 // 4 3



// Use pin 2 as wake up pin
const int wakeUpPin = 2;

int ledPin = 9;    // LED connected to digital pin 9
void wakeUp()
{
  // Just a handler for the pin interrupt.
  //  digitalWrite(7, HIGH);
  //  count = 0;
}
void setup()
{
  pinMode(wakeUpPin, INPUT);
  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  Serial.begin(9600);
  ss.begin(GPSBaud);

  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

//  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
//  updateSerial();
//  char isConnect;
//  do
//  {
//
//    mySerial.println("AT+CREG?"); //Check whether it has registered in the network
//    delay(10);
//    String x;
//    if (mySerial.available() > 0)
//    {
//      x = mySerial.readString();
//      Serial.print("I received: ");
//      Serial.println(x);
//      isConnect = x.charAt(20);
//      Serial.println(isConnect);
//    }
//  } while (isConnect != '1' && isConnect != '5');
//
//  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
//  updateSerial();
//  delay(1000);

  //  init_gsm();
  //  do {
  //    send_gps();
  //    //    Serial.print("ini looping ke-");
  //    //    Serial.println(count);
  //  } while (count < 5);
  //  smartDelay(5000);
}

void loop()
{
  /*====================== ENTER THE SLEEP MODE ============================*/

  //  if (count == 5)
  //  {

  //  digitalWrite(7, LOW);
  // Allow wake up pin to trigger interrupt on low.
  //  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, LOW);

  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  //  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // Disable external pin interrupt on wake up pin.
  //  detachInterrupt(digitalPinToInterrupt(wakeUpPin));
  //  Serial.println("Enter Sleep Mode");
  //    count = 0;
  //  }

  // Do something here
  // Example: Read sensor, data logging, data transmission.
  //  for (int i = 0; i < 5; i++)
  //  {
  //    for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
  //      // sets the value (range from 0 to 255):
  //      analogWrite(ledPin, fadeValue);
  //      // wait for 30 milliseconds to see the dimming effect
  //      delay(30);
  //    }
  //
  //    // fade out from max to min in increments of 5 points:
  //    for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
  //      // sets the value (range from 0 to 255):
  //      analogWrite(ledPin, fadeValue);
  //      // wait for 30 milliseconds to see the dimming effect
  //      delay(30);
  //    }
  //  }


  //  Serial.println("Begin to transmitting data");
  //  digitalWrite(7, HIGH);
  //  count = 0;
  /*====================== TO ENSURE THAT THE GSM IS CONNECTED ============================*/
  init_gsm();
  Serial.println("here");
  /*====================== CONNECTING THE GPS ============================*/
  //    digitalWrite(7, HIGH);
  do {
    send_gps();
    //    Serial.print("ini looping ke-");
    //    Serial.println(count);
  } while (count < 5);
  Serial.println("sleep ...");
  Serial.println(count);
  delay(6000);
  digitalWrite(7, LOW);

  //  while (count < 5)
  //  {
  //    static char hdop_str[10];
  //    static char lat_str[20];
  //    static char lng_str[20];
  //    String str = "";
  //
  //    ss.listen();
  //    dtostrf(gps.hdop.hdop(), 6, 2, hdop_str);
  //    dtostrf(gps.location.lat(), 12, 7, lat_str);
  //    dtostrf(gps.location.lng(), 12, 7, lng_str);
  //
  //    str += hdop_str;
  //    str += lat_str;
  //    str += lng_str;
  //    Serial.println(str);
  //
  //    if (count > 0 && count < 5)
  //    {
  //      static String out;
  //      out += str;
  //      Serial.println("disini");
  //      if (count == 4)
  //      {
  //        mySerial.listen();
  //        mySerial.println("AT+CMGS=\"+6282130310254\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms +6282130310254  +6285270817607
  //        updateSerial();
  //        mySerial.print(out);
  //        updateSerial();
  //        mySerial.write(26); // Carriage Return
  //        Serial.println("\nSent");
  //      }
  //    }
  //
  //    if (gps.hdop.isValid() && gps.location.isValid())
  //    {
  //      count++;
  //    }
  //    Serial.println(count);
  //    smartDelay(1000);
  //  }
  //  smartDelay(1000);
  //  Serial.println("Transmission succeed");
}



void init_gsm() {
//  count = 0;
  digitalWrite(7, HIGH);
  char isConnect;
  String x;

  Serial.println("Initializing...");
  delay(1000);

  mySerial.listen();
  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();

  do
  {
    mySerial.println("AT+CREG?"); //Check whether it has registered in the network
    delay(10);

    if (mySerial.available() > 0)
    {
      x = mySerial.readString();
      Serial.print("I received: ");
      Serial.println(x);
      isConnect = x.charAt(20);
      Serial.println(isConnect);
    }
  } while (isConnect != '1' && isConnect != '5');

  mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  delay(1000);
}

void send_gps()
{
  static char hdop_str[10];
  static char lat_str[20];
  static char lng_str[20];
  String str = "";
  static String out;

  ss.listen();
  dtostrf(gps.hdop.hdop(), 6, 2, hdop_str);
  dtostrf(gps.location.lat(), 12, 7, lat_str);
  dtostrf(gps.location.lng(), 12, 7, lng_str);

  str += hdop_str;
  str += lat_str;
  str += lng_str;
  Serial.println(str);

  if (count > 0 && count < 5)
  {
    out += str;
    Serial.println("disini");
    if (count == 4)
    {
      mySerial.listen();
      mySerial.println("AT+CMGS=\"+6282130310254\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms +6282130310254  +6285270817607
      updateSerial();
      mySerial.print(out);
      updateSerial();
      mySerial.write(26); // Carriage Return
      Serial.println("\nSent");
      Serial.println(out);
      out = "";
    }
  }

  if (gps.hdop.isValid() && gps.location.isValid())
  {
    count++;
  }
  Serial.println(count);
  smartDelay(1000);

}
void updateSerial()
{
  delay(500);
  while (Serial.available())
  {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while (mySerial.available())
  {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (ss.available())
      gps.encode(ss.read());
  } while (millis() - start < ms);
}


static void printFloat(float val, bool valid, int len, int prec)
{
  //  char buffer[12];
  //  sprintf(buffer, "%d %d \n", gps.hdop.isValid(), gps.location.isValid());
  //  Serial.print(buffer);
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i = flen; i < len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}
