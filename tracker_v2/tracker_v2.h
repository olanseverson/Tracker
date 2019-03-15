#include <NMEAGPS.h>
#include <GPSport.h>
#include <Streamers.h>

#include <NeoSWSerial.h>  //it is used instead of SoftwareSerial
#include <stdlib.h>   //the lib is needed for the floatToString() helper function

#include <LowPower.h> // for power down library


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
int sim800Pin = 5; //error pin
int gpsPin = 6; //error pin
volatile boolean IsMessageSent = false;


//a helper function which converts a float to a string with a given precision
String location = "";

// Use pin 2 as wake up pin
const int wakeUpPin = 2;

//enablePin to control enable 
const int enablePin = 7;


String floatToString(float x, byte precision = 2);
static void GPSloop();
void sendData(gps_fix& fix);
void ConnectGSM();
void waitUntilResponse(String resp);
void sendLocation(String location);
void readResponse();
void tryToRead();
void updateSerial();
void wakeUp();
void setupGPS();
void setupGSM();
void setupLowPower();

static void GPSloop(){
  while (gps.available( gpsPort )) {
    fix = gps.read();
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

  if (fix.valid.location && fix.valid.hdop && fix.valid.satellites) {
    digitalWrite(gpsPin, LOW);
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
  DEBUG_PORT.print("Location sent !!!!!!");
  // DEBUG_PORT.println(micros()-send_time);
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


void ConnectGSM() {
  Sim800l.println("AT"); //Once the handshake test is successful, it will back to OK
  waitUntilResponse("OK");
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

void updateSerial(){
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


void wakeUp(){
  // Just a handler for the pin interrupt.

}


void setupGPS()
{
  pinMode(gpsPin, OUTPUT);
  //init
  DEBUG_PORT.begin(debugBaud);
  while (!Serial)
    ;

  DEBUG_PORT.print( F("NMEA.INO: started\n") );
  DEBUG_PORT.print( F("  fix object size = ") );
  DEBUG_PORT.println( sizeof(gps.fix()) );
  DEBUG_PORT.print( F("  gps object size = ") );
  DEBUG_PORT.println( sizeof(gps) );
  DEBUG_PORT.println( F("Looking for GPS device on " GPS_PORT_NAME) );

  #ifndef NMEAGPS_RECOGNIZE_ALL
    #error You must define NMEAGPS_RECOGNIZE_ALL in NMEAGPS_cfg.h!
  #endif

  #ifdef NMEAGPS_INTERRUPT_PROCESSING
    #error You must *NOT* define NMEAGPS_INTERRUPT_PROCESSING in NMEAGPS_cfg.h!
  #endif

  #if !defined( NMEAGPS_PARSE_GGA ) & !defined( NMEAGPS_PARSE_GLL ) & \
      !defined( NMEAGPS_PARSE_GSA ) & !defined( NMEAGPS_PARSE_GSV ) & \
      !defined( NMEAGPS_PARSE_RMC ) & !defined( NMEAGPS_PARSE_VTG ) & \
      !defined( NMEAGPS_PARSE_ZDA ) & !defined( NMEAGPS_PARSE_GST )

    DEBUG_PORT.println( F("\nWARNING: No NMEA sentences are enabled: no fix data will be displayed.") );

  #else
    if (gps.merging == NMEAGPS::NO_MERGING) {
      DEBUG_PORT.print  ( F("\nWARNING: displaying data from ") );
      DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
      DEBUG_PORT.print  ( F(" sentences ONLY, and only if ") );
      DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
      DEBUG_PORT.println( F(" is enabled.\n"
                            "  Other sentences may be parsed, but their data will not be displayed.") );
    }
  #endif

  DEBUG_PORT.print  ( F("\nGPS quiet time is assumed to begin after a ") );
  DEBUG_PORT.print  ( gps.string_for( LAST_SENTENCE_IN_INTERVAL ) );
  DEBUG_PORT.println( F(" sentence is received.\n"
                        "  You should confirm this with NMEAorder.ino\n") );

  trace_header( DEBUG_PORT );
  DEBUG_PORT.flush();

  gpsPort.begin(gpsBaud);
}

void setupGSM()
{
  pinMode(sim800Pin, OUTPUT);

  Sim800l.begin(simBaud);
  
  Sim800l.write(27); //Clears buffer for safety
  DEBUG_PORT.println("Beginning...");
  delay(1000); //Waiting for Sim800L to get signal
  DEBUG_PORT.println("SIM800L should have booted up");
  
  Sim800l.listen(); //The GSM module and GPS module can't communicate with the arduino board at once - so they need to get focus once we need them
  ConnectGSM(); //Enable the internet connection to the SIM card
  DEBUG_PORT.println("Connection set up");
  
  gpsPort.listen();

  last = millis();
}

void setupLowPower()
{
  // Configure wake up pin as input.
  // This will consumes few uA of current.
  pinMode(wakeUpPin, INPUT);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);
  IsMessageSent = false;
}