#include "Rubber_Band_Shooter.h"

#define SERIAL_BAUD_RATE 19200

#define SHOOTER_HAMMER_PIN      3 ///<The pin to use to control the hammer servo.
#define SHOOTER_ELEVATION_PIN   11 ///<The pin to use to control the hammer servo.

Rubber_Band_Shooter * shooter;///<This is the class used to interface rubber band shooter

void setup() {
  // Setup the debug serial port
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  shooter = new Rubber_Band_Shooter(SHOOTER_HAMMER_PIN, SHOOTER_ELEVATION_PIN);

}

void loop() {

  for(int i=0;i<5;i++){
    Serial.println(F("Turn Up..."));
    shooter->turn_up();
    delay(500);
  }

  shooter->fire();
  Serial.println(F("Fire!"));

  for(int i=0;i<5;i++){
    Serial.println(F("Turn Down..."));
    shooter->turn_down();
    delay(500);
  }

  Serial.println(F("Fire!"));
  shooter->fire();

  for(int i=0;i<5;i++){
    Serial.println(F("Turn Left..."));
    shooter->turn_left();
    delay(500);
  }

  Serial.println(F("Fire!"));
  shooter->fire();

  for(int i=0;i<5;i++){
    Serial.println(F("Turn Right..."));
    shooter->turn_right();
    delay(500);
  }

  shooter->fire();

}
