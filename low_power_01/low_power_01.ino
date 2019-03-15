// **** INCLUDES *****
#include "LowPower.h"

// Use pin 2 as wake up pin
const int wakeUpPin = 2;

int ledPin = 9;    // LED connected to digital pin 9
void wakeUp()
{
  // Just a handler for the pin interrupt.

}

int x=0;
void setup()
{
  // Configure wake up pin as input.
  // This will consumes few uA of current.
  pinMode(wakeUpPin, INPUT);
//  pinMode(3, OUTPUT);
//  pinMode(4, OUTPUT);
//  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  Serial.begin(9600);
//  digitalWrite(3, x);
//  digitalWrite(4, x);
//  digitalWrite(5, x);
  digitalWrite(6, x);
  digitalWrite(7, x);
  Serial.begin(9600);
}

void loop()
{

  // Do something here
  // Example: Read sensor, data logging, data transmission.
  x = 1;
//  digitalWrite(3, x);
//  digitalWrite(4, x);
//  digitalWrite(5, x);

  Serial.println("fade in");
  digitalWrite(6, x);
  digitalWrite(7, x);
  for (int i = 0; i < 5; i++)
  {
    for (int fadeValue = 0 ; fadeValue <= 255; fadeValue += 5) {
      // sets the value (range from 0 to 255):
      analogWrite(6, fadeValue);
      // wait for 30 milliseconds to see the dimming effect
      delay(30);
    }

    // fade out from max to min in increments of 5 points:
    for (int fadeValue = 255 ; fadeValue >= 0; fadeValue -= 5) {
      // sets the value (range from 0 to 255):
      analogWrite(6, fadeValue);
      // wait for 30 milliseconds to see the dimming effect
      delay(30);
    }
  }

  Serial.println("sleep mode on");
  delay(1000);
    // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(digitalPinToInterrupt(wakeUpPin), wakeUp, LOW);

  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_OFF);

  // Disable external pin interrupt on wake up pin.
  detachInterrupt(digitalPinToInterrupt(wakeUpPin));
}
