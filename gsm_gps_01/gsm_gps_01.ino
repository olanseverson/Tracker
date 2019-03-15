#include <TinyGPS++.h>
#include <SoftwareSerial.h>
//#include <NeoSWSerial.h>
/*
   This sample code demonstrates the normal use of a TinyGPS++ (TinyGPSPlus) object.
   It requires the use of SoftwareSerial, and assumes that you have a
   4800-baud serial GPS device hooked up on pins 4(rx) and 3(tx).
*/
static const int RXPin = 8, TXPin = 9; // 6, 5
static const uint32_t GPSBaud = 4800; // 4800 untuk 3.3v

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);
//NeoSWSerial ss(RXPin, TXPin);

static float f_val = 0.0;
static char outstr[15];
static int count = 0;
char buffer[20];

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(10, 11); //SIM800L Tx & Rx is connected to Arduino #3 & #2 // 4 3
//NeoSWSerial mySerial(10, 11); //SIM800L Tx & Rx is connected to Arduino #3 & #2

void setup()
{
//  pinMode(7, OUTPUT);
//  pinMode(8, OUTPUT);
//  digitalWrite(7, HIGH);
//  digitalWrite(8, LOW);

  Serial.begin(9600);
  //  mySerial.begin(9600);
  ss.begin(GPSBaud);

  //  Serial.println(F("FullExample.ino"));
  //  Serial.println(F("An extensive example of many interesting TinyGPS++ features"));
  //  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  //  Serial.println(F("by Mikal Hart"));
  //  Serial.println();
  //  Serial.println(F("Sats HDOP  Latitude   Longitude   Fix  Date       Time     Date Alt    Course Speed Card  Distance Course Card  Chars Sentences Checksum"));
  //  Serial.println(F("           (deg)      (deg)       Age                      Age  (m)    --- from GPS ----  ---- to London  ----  RX    RX        Fail"));
  //  Serial.println(F("----------------------------------------------------------------------------------------------------------------------------------------"));

  //Begin serial communication with Arduino and SIM800L
  mySerial.begin(9600);

  Serial.println("Initializing...");
  delay(1000);

  mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
  updateSerial();
  char isConnect;
  do
  {

    mySerial.println("AT+CREG?"); //Check whether it has registered in the network
    delay(10);
    String x;
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

//  mySerial.println("AT+CMGS=\"+6282130310254\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms +6282130310254  +6285270817607
//  updateSerial();
//  mySerial.print("Haiii sayaangg....."); //text content
//  updateSerial();
//  mySerial.write(26);
//  delay(1000);

}

void loop()
{
  static char hdop_str[10];
  static char lat_str[20];
  static char lng_str[20];
  String str = "";

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
    static String out;
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
