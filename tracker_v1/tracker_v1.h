#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

#include <NeoSWSerial.h>  //it is used instead of SoftwareSerial
#include <stdlib.h>   //the lib is needed for the floatToString() helper function

//NOTE: THE CODE WILL ONLY WORK IF YOU HAVE ALREADY UNLOCKED YOUR SIM CARD - SO THE PHONE WON'T ASK FOR PIN CODE WHENEVER YOU USE IT

//SoftwareSerial variables
static const uint32_t gpsBaud = 4800; //0.14666666666
static const int simRX = 10, simTX = 11;
static const uint32_t simBaud = 19200;
static const uint32_t debugBaud = 9600;

//SoftwareSerial instances
NeoSWSerial Sim800l(simRX, simTX);

//GPS instance
NMEAGPS gps;
gps_fix fix;

//helper variables for waitUntilResponse() function
String response = "";
static long maxResponseTime = 5000;
unsigned long lastTime;

//The frequency of http requests (seconds)
int refreshRate = 1; //SECONDS
//variables for a well-scheduled loop - in which the sendData() gets called every 15 secs (refresh rate)
unsigned long last;
unsigned long current;
unsigned long elapsed;

//if there is an error in sendLocation() function after the GPRS Connection is setup - and the number of errors exceeds 3 - the system reboots. (with the help of the reboot pin)
int maxNumberOfErrors = 3;
boolean gprsConnectionSetup = false;
boolean reboot = false;
int errorsEncountered = 0; //number of errors encountered after gprsConnection is set up - if exceeds the max number of errors the system reboots
int resetPin = 12;

//if any error occurs with the gsm module or gps module, the corresponding LED will light up - until they don't get resolved
int sim800Pin = 6; //error pin
int gpsPin = 7; //error pin
volatile boolean IsMessageSent = false;


//a helper function which converts a float to a string with a given precision
String location = "";

String floatToString(float x, byte precision = 2);
static void GPSloop();
void sendData(gps_fix& fix);
void setupGPRSConnection();
void waitUntilResponse(String resp);
void sendLocation(String location);
void readResponse();
void tryToRead();
void updateSerial();

static void GPSloop(){
  while (gps.available( gpsPort )) {
    fix = gps.read();
    // DEBUG_PORT.print(floatToString(fix.latitude(), 7));
    // long send_time = micros();
    sendData(fix);
    // DEBUG_PORT.println(micros()-send_time);
  }

} // GPSloop

void sendData(gps_fix& fix) {
  String location = "Lat,lon,hdop,sat: ";
  location += floatToString(fix.latitude(), 7);
  location += ",";
  location += floatToString(fix.longitude(), 7);
  location += ",";
  location += floatToString( (fix.hdop * 0.001), 3 );
  location += ",";
  location += floatToString( (fix.satellites * 1.0), 1 );
  DEBUG_PORT.print("data to be sent ==>");
  DEBUG_PORT.println(location);
  // DEBUG_PORT.println(sizeof(location));

  if (fix.valid.location && fix.valid.hdop && fix.valid.satellites) {
    digitalWrite(gpsPin, LOW);
    // String lat = floatToString(fix.latitude(), 5);
    // String lon = floatToString(fix.longitude(), 5);
    DEBUG_PORT.print("IsMessageSent: ");
    DEBUG_PORT.println(IsMessageSent);
    if (IsMessageSent == false) {
      IsMessageSent = true;
      DEBUG_PORT.println("sending....");
      // long send_time = micros();
      sendLocation(location);
      // DEBUG_PORT.println(micros()-send_time);
    }
  } else {
    DEBUG_PORT.println("GPS Invalid");
    digitalWrite(gpsPin, HIGH);
  }
}// sendData

//the function - which is responsible for sending data to the GSM receiver
void sendLocation(String location) {
  
  long send_time = micros();

  Sim800l.listen();
  Sim800l.println("AT+CMGS=\"+6282130310254\"");
  updateSerial();
  Sim800l.println(location);
  updateSerial();
  Sim800l.write(26);
  delay(100);
  Sim800l.flush(); //1065464
  DEBUG_PORT.print("Location sent ");
  DEBUG_PORT.println(micros()-send_time);
  gpsPort.listen();
      
  
} // sendLocation

void readResponse() {
  response = "";
  while (response.length() <= 0 || !response.endsWith("\n"))
  {
    tryToRead();
    if (millis() - lastTime > maxResponseTime)
    {
      return;
    }
  }
} // readResponse

void tryToRead() {
  while (Sim800l.available()) {
    char c = Sim800l.read();  //gets one byte from serial buffer
    response += c; //makes the string readString
  }
} // tryToRead

//ERROR handler - exits if error arises or a given time exceeds with no answer - or when everything is OK
void waitUntilResponse(String resp) {
  lastTime = millis();
  response = "";
  String totalResponse = "";
  while (response.indexOf(resp) < 0 && millis() - lastTime < maxResponseTime)
  {
    readResponse();
    totalResponse = totalResponse + response;
    DEBUG_PORT.println(response);
  }

  if (totalResponse.length() <= 0)
  {
    DEBUG_PORT.println("NO RESPONSE");
    digitalWrite(sim800Pin, HIGH);
    if (gprsConnectionSetup == true) {
      DEBUG_PORT.println("error");
      errorsEncountered++;
    }
  }
  else if (response.indexOf(resp) < 0)
  {
    if (gprsConnectionSetup == true) {
      DEBUG_PORT.println("error");
      errorsEncountered++;
    }
    DEBUG_PORT.println("UNEXPECTED RESPONSE");
    DEBUG_PORT.println(totalResponse);
    digitalWrite(sim800Pin, HIGH);
  } else {
    DEBUG_PORT.println("SUCCESSFUL");
    digitalWrite(sim800Pin, LOW);
    errorsEncountered = 0;
  }

  //if there are more errors or equal than previously set ==> reboot!
  if (errorsEncountered >= maxNumberOfErrors) {
    reboot = true;
  }
} // waitUntilResponse


void setupGPRSConnection() {
  Sim800l.println("AT"); //Once the handshake test is successful, it will back to OK
  waitUntilResponse("OK");
  // Sim800l.println("AT+CREG?");
  // // waitUntilResponse("+CREG: 0,1\n\nOK");
  // waitUntilResponse("+CREG: 0,1");
  // Sim800l.println("AT+CMGF=1"); //Enabling HTTP mode
  // waitUntilResponse("OK");
  char isConnect;
  do
  {
    Sim800l.println("AT+CREG?"); //Check whether it has registered in the network
    delay(10);
    String x;
    if (Sim800l.available() > 0)
    {
      x = Sim800l.readString();
      Serial.print("I received: ");
      Serial.println(x);
      isConnect = x.charAt(20);
      Serial.println(isConnect);
    }
  } while (isConnect != '1' && isConnect != '5');
  Sim800l.println("AT+CMGF=1"); // Configuring TEXT mode
  waitUntilResponse("OK");

  gprsConnectionSetup = true;
} // setupGPRSConnection



String floatToString(float x, byte precision = 2) {
  char tmp[50];
  dtostrf(x, 0, precision, tmp);
  return String(tmp);
}// floatToString

void updateSerial()
{
  delay(10);
  while (DEBUG_PORT.available()) 
  {
    Sim800l.write(DEBUG_PORT.read());//Forward what Serial received to Software Serial Port
  }
  while(Sim800l.available()) 
  {
    DEBUG_PORT.write(Sim800l.read());//Forward what Software Serial received to Serial Port
  }
}// updateSerial