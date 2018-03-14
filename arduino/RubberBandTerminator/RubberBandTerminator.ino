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
*/

#include <Servo.h>
#include <avr/wdt.h>  //http://www.nongnu.org/avr-libc/user-manual/modules.html 
#include "Stepper.h"
#include "IRremote.h"


#define SERIAL_BAUD_RATE 57600
#define USE_WATCHDOG_TIMER

Servo base;  // create servo object to control a servo
Servo hammer;

////////////// IR Receiver Control Definitions //////////////
#define IR_RECEIVER_PIN  2     // Signal Pin of IR receiver to Arduino Digital Pin
IRrecv irrecv(IR_RECEIVER_PIN);       // create instance of 'irrecv'
decode_results results;   // IR receiver results 

////////////// Servo Control Definitions //////////////
#define ARMED_HAMMER_POSITION 35 //degrees
#define FIRE_HAMMER_POSITION 150 //degrees
#define HAMMER_PIN 3             //must be a PWM pin

//base, now elevation for now
#define BASE_CENTER_POSITION 120 //degrees
#define BASE_MOVEMENT_RANGE  30 //base can move plus or minus this many degrees
#define BASE_PIN 5 //must be a PWM pin
#define BASE_POSITION_INCREMENT 5 //degrees
int base_command_position = BASE_CENTER_POSITION;

/*----- Variables, Pins -----*/
#define STEPS  32   // Number of steps per revolution of Internal shaft
int  Steps2Take;    // 2048 = 1 Revolution

/*-----( Declare objects )-----*/
// Setup of proper sequencing for Motor Driver Pins
// In1, In2, In3, In4 in the sequence 1-3-2-4

Stepper small_stepper(STEPS, 8, 9, 10, 7);




void fire() {
  hammer.write(FIRE_HAMMER_POSITION);  //fiiiirrrre!
  delay(1000);  //wait a second for the servo to get there //todo: get rid of this blocking delay
  hammer.write(ARMED_HAMMER_POSITION);  //ready to load again
}

void turn_up() {
  //move in positive direction
  base_command_position = base_command_position + BASE_POSITION_INCREMENT;
  if(base_command_position > (BASE_CENTER_POSITION + BASE_MOVEMENT_RANGE) )
    base_command_position = (BASE_CENTER_POSITION + BASE_MOVEMENT_RANGE);

  base.write(base_command_position);
}

void turn_down() {
  //move in positive direction
  base_command_position = base_command_position - BASE_POSITION_INCREMENT;
  if(base_command_position < (BASE_CENTER_POSITION - BASE_MOVEMENT_RANGE) )
    base_command_position = (BASE_CENTER_POSITION - BASE_MOVEMENT_RANGE);

  base.write(base_command_position);
}

void turn_right(){
  small_stepper.setSpeed(500); //Max seems to be 500
  small_stepper.step(30);// Rotate CW about 5 degrees (30 steps)
  delay(500); 
}

void turn_left(){
  small_stepper.setSpeed(500); //Max seems to be 500
  small_stepper.step(-30);// Rotate CCW about 5 degrees (30 steps)
  delay(500); 
}

/*
 * translateIR()
 * 
 * Take action on IR remote control inputs.
 * 
 * This function makes changes to global variables, which are applied 
 *   to the digital IO on the regular program cycle.
 *   
 */
void translateIR(decode_results results) // takes action based on IR code received
{

  switch(results.value)

  {
  case 0xFFA25D: Serial.println("POWER"); break;
  case 0xFFE21D: Serial.println("VOL STOP"); break;
  case 0xFF629D: Serial.println("MODE"); break;
  case 0xFF22DD: Serial.println("PAUSE");
                  fire();
                  break;
  case 0xFF02FD: Serial.println("FAST BACK");
                 turn_left();
                  break;
  case 0xFFC23D: Serial.println("FAST FORWARD");
                 turn_right();
                 break;
  case 0xFFE01F: Serial.println("EQ");    break;
  case 0xFFA857: Serial.println("VOL-");
                 turn_down();
                 break;
  case 0xFF906F: Serial.println("VOL+");
                 turn_up();
                 break;
  case 0xFF9867: Serial.println("RETURN");    break;
  case 0xFFB04F: Serial.println("USB SCAN");    break;
  case 0xFF6897: Serial.println("0");
                 break;
  case 0xFF30CF: Serial.println("1");
                 break;
  case 0xFF18E7: Serial.println("2");
                 break;
  case 0xFF7A85: Serial.println("3");
                 break;
  case 0xFF10EF: Serial.println("4");
                 break;
  case 0xFF38C7: Serial.println("5");
                 break;
  case 0xFF5AA5: Serial.println("6");
                 break;
  case 0xFF42BD: Serial.println("7");
                 break;
  case 0xFF4AB5: Serial.println("8");
                 break;
  case 0xFF52AD: Serial.println("9");
                 break;
  case 0xFFFFFFFF: Serial.println(" REPEAT");break;  

  default: 
    Serial.println(" other button   ");

  }// End Case
} 


void setup() {
  // Setup the debug serial port
  Serial.begin(SERIAL_BAUD_RATE);

    // Enable IR Remote Control Input
  irrecv.enableIRIn(); // Start the receiver
  
  base.attach(BASE_PIN);  // attaches the servo on pin 9 to the servo object
  hammer.write(base_command_position);
  
  hammer.attach(HAMMER_PIN);
  hammer.write(ARMED_HAMMER_POSITION);

#ifdef USE_WATCHDOG_TIMER
  wdt_enable(WDTO_8S);  // all code must execute in less than 8s.
#endif


  Serial.println("Setup complete.  Running...");
}


void loop() {
  
#ifdef USE_WATCHDOG_TIMER
  wdt_reset();  //reset the countdown of the watchdog.
#endif

  // Check for remote control commands
  if (irrecv.decode(&results)) // have we received an IR signal?
  {
    translateIR(results); 
    irrecv.resume(); // receive the next value
  }
  else
  {
    delay(100);  //chill out for a sec
  }


}

