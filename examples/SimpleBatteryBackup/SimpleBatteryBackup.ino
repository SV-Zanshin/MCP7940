/**************************************************************************************************
Example program to demonstrate a battery-backup MCP7940N usage. The library as well as the most
current version of this program is available on GitHub at https://github.com/Zanduino/MCP7940 and
a more detailed description of this program (and the library) can be found at
https://github.com/Zanduino/MCP7940/wiki/SimpleBatteryBackup.ino The MCP7940 library uses the
standard SPI Wire library for communications with the RTC chip and has also used the class
definitions of the standard RTClib library from Adafruit/Jeelabs. The data sheet for the MCP7940M is
located at http://ww1.microchip.com/downloads/en/DeviceDoc/20002292B.pdf. The MCP7940N has extra
functionality revolving around battery backup but this library's functionality covers that chip as
well.
This example program shows how the battery backup information can be used on the MCP7940N chip (the
"M" version doesn't support battery backup). When power is turned on and the program starts, it
checks to see if the MCP7940N has detected a power failure and displays the relevant information.
If the MCP7940N shows no power failure, then the program sets the time (using the sketch compilation
date and time) and loops.

This program is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version. This program is distributed in the hope that it
will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details. You should
have received a copy of the GNU General Public License along with this program.  If not, see
<http://www.gnu.org/licenses/>.

Vers.  Date       Developer  Comments
====== ========== ========== ====================================================
1.0.0  2020-11-21 SV-Zanshin Issue #52 - Cloned from TestBatteryBackup.ino

*/
#include <MCP7940.h>  // Include the MCP7940 RTC library

/***************************************************************************************************
** Declare all program constants                                                                  **
***************************************************************************************************/
const uint32_t SERIAL_SPEED{115200};     // Set the baud rate for Serial I/O
const uint8_t  LED_PIN{13};              // Arduino built-in LED pin number
const uint8_t  SPRINTF_BUFFER_SIZE{32};  // Buffer size for sprintf()

/***************************************************************************************************
** Declare global variables and instantiate classes                                               **
***************************************************************************************************/
MCP7940_Class MCP7940;                           // Create an instance of the MCP7940
char          inputBuffer[SPRINTF_BUFFER_SIZE];  // Buffer for sprintf()/sscanf()
/***************************************************************************************************
** Method Setup(). This is an Arduino IDE method which is called upon boot or restart. It is only **
** called one time and then control goes to the main loop, which loop indefinately.               **
***************************************************************************************************/
void setup() {
  Serial.begin(SERIAL_SPEED);  // Start serial port at Baud rate
#ifdef __AVR_ATmega32U4__
  delay(3000);
#endif
  Serial.print(F("\nStarting SimpleBatteryBackup program\n"));
  Serial.print(F("- Compiled with c++ version "));
  Serial.print(F(__VERSION__));
  Serial.print(F("\n- On "));
  Serial.print(F(__DATE__));
  Serial.print(F(" at "));
  Serial.print(F(__TIME__));
  Serial.print(F("\n"));
  while (!MCP7940.begin()) {  // Initialize RTC communications
    Serial.println(F("Unable to find MCP7940N. Checking again in 3s."));  // Show error and wait
    delay(3000);
  }  // of loop until device is located
  Serial.println(F("MCP7940N initialized."));
  if (MCP7940.getPowerFail()) {  // Check for a power failure
    Serial.println(F("Power failure mode detected!\n"));
    Serial.print(F("Power failed at   "));
    DateTime now = MCP7940.getPowerDown();                      // Read when the power failed
    sprintf(inputBuffer, "....-%02d-%02d %02d:%02d:..",         // Use sprintf() to pretty print
            now.month(), now.day(), now.hour(), now.minute());  // date/time with leading zeros
    Serial.println(inputBuffer);
    Serial.print(F("Power restored at "));
    now = MCP7940.getPowerUp();                                 // Read when the power restored
    sprintf(inputBuffer, "....-%02d-%02d %02d:%02d:..",         // Use sprintf() to pretty print
            now.month(), now.day(), now.hour(), now.minute());  // date/time with leading zeros
    Serial.println(inputBuffer);
    MCP7940.clearPowerFail();  // Reset the power fail switch
  } else {
    while (!MCP7940.deviceStatus()) {  // Turn oscillator on if necessary
      Serial.println(F("Oscillator is off, turning it on."));
      bool deviceStatus = MCP7940.deviceStart();  // Start oscillator and return state
      if (!deviceStatus) {                        // If it didn't start
        Serial.println(F("Oscillator did not start, trying again."));  // Show error and
        delay(1000);                                                   // wait for a second
      }                // of if-then oscillator didn't start
    }                  // of while the oscillator is off
    MCP7940.adjust();  // Set to library compile Date/Time
    Serial.println(F("Enabling battery backup mode"));
    MCP7940.setBattery(true);     // enable battery backup mode
    if (!MCP7940.getBattery()) {  // Check if successful
      Serial.println(F("Couldn't set Battery Backup, is this a MCP7940N?"));
    }                        // if-then battery mode couldn't be set
  }                          // of if-then-else we have detected a priorpower failure
  pinMode(LED_PIN, OUTPUT);  // Declare built-in LED as output
}  // of method setup()
/***************************************************************************************************
** This is the main program for the Arduino IDE, it is an infinite loop and keeps on repeating.   **
***************************************************************************************************/
void loop() {
  static uint8_t secs;                 // store the seconds value
  DateTime       now = MCP7940.now();  // get the current time
  if (secs != now.second()) {          // Output if seconds have changed
    sprintf(inputBuffer, "%04d-%02d-%02d %02d:%02d:%02d",
            now.year(),  // Use sprintf() to pretty print
            now.month(), now.day(), now.hour(), now.minute(),
            now.second());                         // date/time with leading zeros
    Serial.print(inputBuffer);                     // Display the current date/time
    secs = now.second();                           // Set the counter variable
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Toggle the LED
    Serial.println();
  }            // of if the seconds have changed
  delay(100);  // Do nothing for 100 milliseconds
}  // of method loop()
