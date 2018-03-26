#ifndef RUBBER_BAND_SHOOTER_H
#define RUBBER_BAND_SHOOTER_H
/*
Rubber_Band_Shooter.h

This class controls the Rubber Band Shooter PTZ.

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
#include <Arduino.h>
#include <Servo.h>
#include "Stepper.h"
#include <SoftwareSerial.h>

////////////// Serial and Debug Definitions //////////////
#define SERIAL_BAUD_RATE 57600
#define INPUT_BUFFER_MAX_SIZE 500 //bytes - HTTP Requests cannot exceed this size!
#define USE_WATCHDOG_TIMER
#define MAX_BLOCKING_DELAY 5 //ms


////////////// Servo Control Definitions //////////////
// hammer servo - SG90 micro servo
// fire direction = positive (travel is 0-180deg)
#define ARMED_HAMMER_POSITION 35 //degrees
#define FIRE_HAMMER_POSITION 150 //degrees
#define HAMMER_PIN 3             //must be a PWM pin

// elevation servo - some random servo from my toolbox
// up = positive (travel is 0-180deg)
#define ELEVATION_CENTER_POSITION 120 //degrees
#define ELEVATION_MOVEMENT_RANGE  30 //elevation can move plus or minus this many degrees
#define ELEVATION_PIN 11 //must be a PWM pin
#define ELEVATION_POSITION_INCREMENT 5 //degrees


// Base (azimuth) stepper motor (z-down, so CW is positive)
// Unlimited travel
#define STEPS_PER_REV  32   // Number of steps per internal motor shaft revolution
                            // 2048 steps = 1 internal shaft revolution
// For correct sequencing using ULN2003APG Motor driver,
// initialize in this order: In1, In3, In2, In4 

#define BASE_STEP_INCREMENT 5 //degrees per increment
#define BASE_STEPS_PER_DEGREE 6 //roughly - 2048/360 = 5.6889
#define BASE_MAX_SPEED 500


class Rubber_Band_Shooter{
private:
  Servo hammer;
  Servo elevation;  // create servo object to control a servo
  int elevation_command_position = ELEVATION_CENTER_POSITION;
  Stepper * small_stepper;
  String serial_input_buffer;

public:
  Rubber_Band_Shooter();
  void fire();
  void turn_up();
  void turn_down();
  void turn_right();
  void turn_left();
};

#endif

