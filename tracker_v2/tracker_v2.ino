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
#include "tracker_v2.h"

void setup(){

}

void loop ()
{
  setupLowPower();
  setupGPS();
  setupGSM();
  DEBUG_PORT.println("SLEEP MODE : OFF ");
  do {
    // DEBUG_PORT.println("gps loop");
    GPSloop();
    if(reboot){
      digitalWrite(resetPin, LOW);
    }
  }while (IsMessageSent == false);


  DEBUG_PORT.println("SLEEP MODE : ON");
  delay(5000);
  digitalWrite(enablePin, LOW);
  delay(5000);
    // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, LOW);

  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));

}


