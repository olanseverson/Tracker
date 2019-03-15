//======================================================================
//  Program: NMEAsimple.ino
//
//  Description:  This program shows simple usage of NeoGPS
//
//  Prerequisites:
//     1) NMEA.ino works with your device (correct TX/RX pins and baud rate)
//     2) At least one of the RMC, GGA or GLL sentences have been enabled in NMEAGPS_cfg.h.
//     3) Your device at least one of those sentences (use NMEAorder.ino to confirm).
//     4) LAST_SENTENCE_IN_INTERVAL has been set to one of those sentences in NMEAGPS_cfg.h (use NMEAorder.ino).
//     5) LOCATION and ALTITUDE have been enabled in GPSfix_cfg.h
//
//  'Serial' is for debug output to the Serial Monitor window.
//======================================================================
#include "tracker_v1.h"

void setup(){

  pinMode(sim800Pin, OUTPUT);
  pinMode(gpsPin, OUTPUT);

  //setup resetPin
  pinMode(resetPin, OUTPUT);
  delay(200);
  digitalWrite(resetPin, HIGH);

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

  Sim800l.begin(simBaud);
  
  Sim800l.write(27); //Clears buffer for safety
  DEBUG_PORT.println("Beginning...");
  delay(1000); //Waiting for Sim800L to get signal
  DEBUG_PORT.println("SIM800L should have booted up");
  
  Sim800l.listen(); //The GSM module and GPS module can't communicate with the arduino board at once - so they need to get focus once we need them
  setupGPRSConnection(); //Enable the internet connection to the SIM card
  DEBUG_PORT.println("Connection set up");
  
  gpsPort.listen();

  last = millis();
  
}

// void loop(){

//   current = millis();
//   elapsed += current - last;
//   last = current;
//   // DEBUG_PORT.println(elapsed);
//   // while (1){} 
//   while (gps.available(gpsPort)) {
//       fix = gps.read();
//   }
//   if(elapsed >= (refreshRate * 1000)) {
//     sendData();
//     elapsed -= (refreshRate * 1000);
//   }

//   if ((gps.statistics.chars < 10)) {
//      //no gps detected (maybe wiring)
//      DEBUG_PORT.println("NO GPS DETECTED OR BEFORE FIRST HTTP REQUEST");
//   }

//   if(reboot){
//     digitalWrite(resetPin, LOW);   
//   }
  
// }

void loop ()
{
  // long send_time = micros();
  // DEBUG_PORT.println(millis());
  GPSloop();
  if(reboot){
    digitalWrite(resetPin, LOW);   
  }
}

// =================================================
// void setup()
// {
//   DEBUG_PORT.begin(9600);
//   while (!Serial)
//     ;
//   DEBUG_PORT.print( F("NMEAsimple.INO: started\n") );

//   gpsPort.begin(4800);
// }

// //--------------------------

// void loop()
// {
//   while (gps.available( gpsPort )) {
//     fix = gps.read();

//     DEBUG_PORT.print( F("Location: ") );
//     if (fix.valid.location) {
//       DEBUG_PORT.print( fix.latitude(), 6 );
//       DEBUG_PORT.print( ',' );
//       DEBUG_PORT.print( fix.longitude(), 6 );
//     }

//     DEBUG_PORT.print( F(", Altitude: ") );
//     if (fix.valid.altitude)
//       DEBUG_PORT.print( fix.altitude() );

//     DEBUG_PORT.println();
//   }
// }
