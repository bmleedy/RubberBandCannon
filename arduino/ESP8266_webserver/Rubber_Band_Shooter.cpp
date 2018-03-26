/*
RubberBandTerminator.ino

The sole intention of this project is to shoot things with rubber bands using the coolest technology possible. 

Ideas: 
    * 3d print a base to attach servo to stepper motor
    * rename variables for the elevation
    * Add a camera for targeting
    * Try to do auto-targeting with camera image
    * Setup control via wifi
    * Setup display website
    * Setup a website to control the gun
    * possibly get rid of
    * Might need to add software serial receive for additional I/O: 
    *   https://www.arduino.cc/en/Tutorial/TwoPortReceive
    *   https://www.arduino.cc/en/Reference/SoftwareSerial
    *   http://www.ladyada.net/make/eshield/download.html
    * Consider storing website static data in EEPROM: https://learn.adafruit.com/memories-of-an-arduino/eeprom
    * 
*/

#include "Rubber_Band_Shooter.h"


// Fire the rubber band
void Rubber_Band_Shooter::fire() {
  hammer.write(FIRE_HAMMER_POSITION);  //fiiiirrrre!
  delay(1000);  //wait a second for the servo to get there //todo: get rid of blocking delay
  hammer.write(ARMED_HAMMER_POSITION);  //ready to load again
}

// increase the elevation by one increment
void Rubber_Band_Shooter::turn_up() {
  //move in positive direction
  elevation_command_position = elevation_command_position + ELEVATION_POSITION_INCREMENT;
  if(elevation_command_position > (ELEVATION_CENTER_POSITION + ELEVATION_MOVEMENT_RANGE) )
    elevation_command_position = (ELEVATION_CENTER_POSITION + ELEVATION_MOVEMENT_RANGE);

  elevation.write(elevation_command_position);
}

// decrease the elevation by one increment
void Rubber_Band_Shooter::turn_down() {
  //move in positive direction
  elevation_command_position = elevation_command_position - ELEVATION_POSITION_INCREMENT;
  if(elevation_command_position < (ELEVATION_CENTER_POSITION - ELEVATION_MOVEMENT_RANGE) )
    elevation_command_position = (ELEVATION_CENTER_POSITION - ELEVATION_MOVEMENT_RANGE);

  elevation.write(elevation_command_position);
}

// turn clockwise by one increment
void Rubber_Band_Shooter::turn_right(){
  int step_distance = BASE_STEP_INCREMENT * BASE_STEPS_PER_DEGREE;
  small_stepper->setSpeed(BASE_MAX_SPEED);
  small_stepper->step(step_distance);// Rotate CW
  delay(MAX_BLOCKING_DELAY); //todo: make this non-blocking
}

// turn counterclockwise by one increment
void Rubber_Band_Shooter::turn_left(){
  int step_distance = -1 * BASE_STEP_INCREMENT * BASE_STEPS_PER_DEGREE;
  small_stepper->setSpeed(BASE_MAX_SPEED); //Max is 500
  small_stepper->step(step_distance);// Rotate CCW
  delay(MAX_BLOCKING_DELAY); //todo: make this non-blocking
}


// Constructor
Rubber_Band_Shooter::Rubber_Band_Shooter(){

  // reserve space for my serial input buffer
  serial_input_buffer.reserve(INPUT_BUFFER_MAX_SIZE);
  serial_input_buffer = "";
  
  elevation.attach(ELEVATION_PIN);  // attaches the servo on pin 9 to the servo object
  hammer.write(elevation_command_position);
  
  hammer.attach(HAMMER_PIN);
  hammer.write(ARMED_HAMMER_POSITION);

  small_stepper = new Stepper(STEPS_PER_REV, 2, 6, 10, 7);

  Serial.println("Setup complete.  Running...");
}


