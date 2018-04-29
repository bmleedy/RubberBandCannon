/*!
 * @file Rubber_Band_Shooter.h
 * 
 * @brief This class controls the Rubber Band Shooter PTZ.
*/

#ifndef RUBBER_BAND_SHOOTER_H
#define RUBBER_BAND_SHOOTER_H

#include <Arduino.h>
#include <ServoTimer2.h>
#include "Stepper.h"

////////////// Serial and Debug Definitions //////////////
//#define SERIAL_BAUD_RATE 57600
/*! @def INPUT_BUFFER_MAX_SIZE
 * HTTP Requests cannot exceed this size!*/
#define INPUT_BUFFER_MAX_SIZE 500
/*! @def MAX_BLOCKING_DELAY
 * Max delay in ms for the rubber band shooter internal logic*/
#define MAX_BLOCKING_DELAY 5


////////////// Servo Control Definitions //////////////
/*! @def ARMED_HAMMER_POSITION
 * Armed hammer position, in degrees
 * hammer servo - SG90 micro servo
 * fire direction = positive (travel is 0-180deg)*/
#define ARMED_HAMMER_POSITION 120
/*! @def FIRE_HAMMER_POSITION
 * Fire hammer position, in degrees*/
#define FIRE_HAMMER_POSITION 5
/*! @def HAMMER_PIN
 * must be a PWM pin*/
#define HAMMER_PIN 3


/*! @def ELEVATION_CENTER_POSITION
 * center position for elevation, in degrees
 * up = positive (travel is 0-180deg)
 */
#define ELEVATION_CENTER_POSITION 90
/*! @def ELEVATION_MOVEMENT_RANGE
 * elevation can move plus or minus this many degrees*/
#define ELEVATION_MOVEMENT_RANGE  30
/*! @def ELEVATION_PIN
 * pin to drive elevation servo-must be a PWM pin*/
#define ELEVATION_PIN 11
/*! @def ELEVATION_POSITION_INCREMENT
 * degrees per up or down command step*/
#define ELEVATION_POSITION_INCREMENT 5

/*! @def STEPS_PER_REV
 * Base (azimuth) stepper motor (z-down, so CW is positive)
 * Unlimited travel
 * Number of steps per internal motor shaft revolution
 * 2048 steps = 1 internal shaft revolution*/
#define STEPS_PER_REV  32   
/*! @def BASE_STEP_INCREMENT
 * degrees per increment*/
#define BASE_STEP_INCREMENT 5
/*! @def BASE_STEPS_PER_DEGREE
 * roughly - 2048/360 = 5.6889*/
#define BASE_STEPS_PER_DEGREE 6
/*! @def BASE_MAX_SPEED
 * max increments per second*/
#define BASE_MAX_SPEED 500

/*!
 * @class Rubber_Band_Shooter
 * 
 * @brief Handles all arduino functions to control movement of rubber band shooter.
 * 
 */
class Rubber_Band_Shooter{
private:
  ServoTimer2 hammer;
  ServoTimer2 elevation;  // create servo object to control a servo
  int elevation_command_position;
  Stepper * small_stepper;
  String serial_input_buffer;

public:
  Rubber_Band_Shooter();
  Rubber_Band_Shooter(unsigned char hammer_pin, unsigned char elevation_pin);
  void fire();
  void turn_up();
  void turn_down();
  void turn_right();
  void turn_left();
};

#endif

