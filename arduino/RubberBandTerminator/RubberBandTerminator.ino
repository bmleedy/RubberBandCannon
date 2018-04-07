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

#include <Servo.h>
#include <avr/wdt.h>  //http://www.nongnu.org/avr-libc/user-manual/modules.html 
#include "Stepper.h"
#include "IRremote.h"
#include <SoftwareSerial.h>

////////////// Serial and Debug Definitions //////////////
#define SERIAL_BAUD_RATE 57600
#define INPUT_BUFFER_MAX_SIZE 500 //bytes - HTTP Requests cannot exceed this size!
#define USE_WATCHDOG_TIMER
#define MAX_BLOCKING_DELAY 5 //ms


////////////// IR Receiver Control Definitions //////////////
#define IR_RECEIVER_PIN  2     // Signal Pin of IR receiver to Arduino Digital Pin
IRrecv irrecv(IR_RECEIVER_PIN);       // create instance of 'irrecv'


////////////// Servo Control Definitions //////////////
// hammer servo - SG90 micro servo
// fire direction = positive (travel is 0-180deg)
Servo hammer;
#define ARMED_HAMMER_POSITION 35 //degrees
#define FIRE_HAMMER_POSITION 150 //degrees
#define HAMMER_PIN 3             //must be a PWM pin

// elevation servo - some random servo from my toolbox
// up = positive (travel is 0-180deg)
Servo elevation;  // create servo object to control a servo
#define ELEVATION_CENTER_POSITION 120 //degrees
#define ELEVATION_MOVEMENT_RANGE  30 //elevation can move plus or minus this many degrees
#define ELEVATION_PIN 5 //must be a PWM pin
#define ELEVATION_POSITION_INCREMENT 5 //degrees
int elevation_command_position = ELEVATION_CENTER_POSITION;

// Base (azimuth) stepper motor (z-down, so CW is positive)
// Unlimited travel
#define STEPS_PER_REV  32   // Number of steps per internal motor shaft revolution
                            // 2048 steps = 1 internal shaft revolution
// For correct sequencing using ULN2003APG Motor driver,
// initialize in this order: In1, In3, In2, In4 
Stepper small_stepper(STEPS_PER_REV, 8, 9, 10, 7);
#define BASE_STEP_INCREMENT 5 //degrees per increment
#define BASE_STEPS_PER_DEGREE 6 //roughly - 2048/360 = 5.6889
#define BASE_MAX_SPEED 500


////////////// Action Functions //////////////
// Fire the rubber band
void fire() {
  hammer.write(FIRE_HAMMER_POSITION);  //fiiiirrrre!
  delay(1000);  //wait a second for the servo to get there //todo: get rid of blocking delay
  hammer.write(ARMED_HAMMER_POSITION);  //ready to load again
}

// increase the elevation by one increment
void turn_up() {
  //move in positive direction
  elevation_command_position = elevation_command_position + ELEVATION_POSITION_INCREMENT;
  if(elevation_command_position > (ELEVATION_CENTER_POSITION + ELEVATION_MOVEMENT_RANGE) )
    elevation_command_position = (ELEVATION_CENTER_POSITION + ELEVATION_MOVEMENT_RANGE);

  elevation.write(elevation_command_position);
}

// decrease the elevation by one increment
void turn_down() {
  //move in positive direction
  elevation_command_position = elevation_command_position - ELEVATION_POSITION_INCREMENT;
  if(elevation_command_position < (ELEVATION_CENTER_POSITION - ELEVATION_MOVEMENT_RANGE) )
    elevation_command_position = (ELEVATION_CENTER_POSITION - ELEVATION_MOVEMENT_RANGE);

  elevation.write(elevation_command_position);
}

// turn clockwise by one increment
void turn_right(){
  int step_distance = BASE_STEP_INCREMENT * BASE_STEPS_PER_DEGREE;
  small_stepper.setSpeed(BASE_MAX_SPEED);
  small_stepper.step(step_distance);// Rotate CW
  delay(MAX_BLOCKING_DELAY); //todo: make this non-blocking
}

// turn counterclockwise by one increment
void turn_left(){
  int step_distance = -1 * BASE_STEP_INCREMENT * BASE_STEPS_PER_DEGREE;
  small_stepper.setSpeed(BASE_MAX_SPEED); //Max is 500
  small_stepper.step(step_distance);// Rotate CCW
  delay(MAX_BLOCKING_DELAY); //todo: make this non-blocking
}


////////////// Control Functions //////////////
//Take action on IR remote control inputs.
void translateIR(decode_results results)
{
  switch(results.value)

  {
  case 0xFF22DD: Serial.println(F("PAUSE"));
                  fire();
                  break;
  case 0xFF02FD: Serial.println(F(("FAST BACK")));
                 turn_left();
                  break;
  case 0xFFC23D: Serial.println(F("FAST FORWARD"));
                 turn_right();
                 break;
  case 0xFFA857: Serial.println(F("VOL-"));
                 turn_down();
                 break;
  case 0xFF906F: Serial.println(F("VOL+"));
                 turn_up();
                 break;
  case 0xFFFFFFFF: Serial.println(F(" REPEAT"));break;  
  default: 
    Serial.print(F(" Other button: ")); Serial.println(results.value, HEX);
  }
} 

///////////////////////////////////
////////////// Setup //////////////
///////////////////////////////////

String serial_input_buffer = "";

void setup() {

  // reserve space for my serial input buffer
  serial_input_buffer.reserve(INPUT_BUFFER_MAX_SIZE);
  
  // Setup the debug serial port
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

    // Enable IR Remote Control Input
  irrecv.enableIRIn(); // Start the receiver
  
  elevation.attach(ELEVATION_PIN);  // attaches the servo on pin 9 to the servo object
  hammer.write(elevation_command_position);
  
  hammer.attach(HAMMER_PIN);
  hammer.write(ARMED_HAMMER_POSITION);

#ifdef USE_WATCHDOG_TIMER
  wdt_enable(WDTO_8S);  // all code must execute in less than 8s.
#endif

  Serial.println("Setup complete.  Running...");
}

void handle_serial_input(String * input_buffer) {
  // todo: handle serial input data here
}


///////////////////////////////////
////////////// Loop ///////////////
///////////////////////////////////


void loop() {
  
#ifdef USE_WATCHDOG_TIMER
  wdt_reset();  //reset the countdown of the watchdog.
#endif

  // Check for remote control commands
  decode_results results;   // IR receiver results 
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    translateIR(results); 
    irrecv.resume(); // receive the next value
  }
  else
  {
    delay(MAX_BLOCKING_DELAY);  //chill out for a short time
  }

  // Check for serial input
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    serial_input_buffer += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      handle_serial_input(&serial_input_buffer);
    } else if (serial_input_buffer.length() >= INPUT_BUFFER_MAX_SIZE) {
      Serial.println(F("WARNING: serial buffer size exceeded without processing newline."));
      handle_serial_input(&serial_input_buffer);
    }
  }

}



