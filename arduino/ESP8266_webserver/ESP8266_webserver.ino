/*!
 * @file ESP8266_webserver.ino
 * 
 * @mainpage RubberBandCannon Arduino Code
 * 
 * @section intro_sed Introduction
 * 
 * The sole intention of this project is to shoot things with rubber bands using the coolest technology possible.
 * 
 * This program uses the ESP8266 class to serve up a website. The
 * objective is to ontrol the rubber band launching
 * device from my phone, tablet, the internet, or whatever.
 * 
 * RESEARCH LINKS: 
 *     https://github.com/sleemanj/ESP8266_Simple/blob/master/README.md <br>
 *     https://nurdspace.nl/ESP8266 <br>
 *     http://www.electrodragon.com/w/images/f/ff/4a-esp8266_at_instruction_set_en_%281%29.pdf <br>
 *     https://github.com/sleemanj/ESP8266_Simple/blob/master/arduino-wiring-diagram.jpg <br>
 *     https://smile.amazon.com/gp/product/B01EA3UJJ4/ref=oh_aui_detailpage_o02_s00?ie=UTF8&psc=1 <br>
 *     https://hackingmajenkoblog.wordpress.com/2016/02/04/the-evils-of-arduino-strings/ <br>
 *     https://www.youtube.com/watch?v=ETLDW22zoMA&t=9s <br>
 * 
 * ESP8266 SETUP:
 *     Default baud rate on the chips: 115200bps
 *     To enable the chips: set all middle pins to +3.3v
 *     Default baud rate is 115200 (software serial port is flaky at this rate)
 *     Use AT+UART_DEF=19200,8,1,0,0 to switch to 19200
 * 
 * SOFTWARESERIAL SETUP:
 * The pin used for RX must support change interrupts: 
 *                https://playground.arduino.cc/Code/Interrupts 
 *                
 *                
 * REFERENCE LINKS: 
 *        https://os.mbed.com/users/wim/notebook/sc16is750-i2c-or-spi-to-uart-bridge/
 *        https://www.nxp.com/docs/en/brochure/75015676.pdf
 *        https://www.maximintegrated.com/en/products/interface/controllers-expanders/MAX3107.html
 *        https://forum.arduino.cc/index.php?topic=419359.0
 *        http://arduino-esp8266.readthedocs.io/en/latest/PROGMEM.html
 *        
 *        
 *          
 * @section dependencies Dependencies
 *  This library depends on AltSoftSerial, ServoTimer2, and Stepper Arduino drivers being present on your system. Please make sure you have
 * installed the latest version before using this library.
 * 
 * @section author Author
 * 
 * Written by Brett "bmleedy" Leedy for fun and profit.
 * 
 * @section license License
 * 
 * Copyright Brett Leedy, 2018.  All rights reserved.
 */


//! @todo Scrub all code for todo's
//! @todo Flesh out webserver stack - make more reliable
//! @todo Store the website in EEPROM
//! @todo Use a CDN (like CloudFront) for icons, graphics and other eye candy (maybe S3?)
//! @todo Use HTTP "Content Length:" header instead of terminating connections
//! @todo Remove all delay() calls in code
//! @todo Enable watchdog timer
//! @todo Use AJAX for command buttons - don't reload the website for each button press
//! @todo Add super-basic MVC to the webserver
//! @todo Add a camera to the shooter/website (snap after each turn)
//! @todo Create an ESP initializer program, since 
//! @todo Assemble more better-integrated ESP boards
//! @todo Build a protoboard with everything on it
//! @todo Connectorize harness to board (0.1in block connector)
//! @todo Consolidate all constants, etc to a header file
//! @todo Get a better name for the webserver
//! @todo Code lint 
//! @todo Implement TLS
//! @todo Figure out what other goodness the ESP8266 gives us
//! @todo Make the ESP8266 an access point
//! @todo Use access point functionality for device setup
//! @todo Support multiple connections (instead of just connection 0)
//! @todo Make a you tube video and link in readme
//! @todo Make a custom icon for the project.
//! @todo Try to do auto-targeting with camera image        
//! @todo Add fuzz testing of inputs
//! @todo fix the case on prefetch length #defines
//! @todo fix the problem with declaring prefetch length as ints

#include <AltSoftSerial.h>
#include <MemoryUsage.h>
#include "ESP8266.h"
#include "Rubber_Band_Shooter.h"


#define DEBUG_MEMORY true ///<flag to enable serial port prints indicating amount of free heap.

// Serial Port Definitions
#define PRINT_SERIAL_STREAM false ///<If set, all data to and from the ESP8266 is dumped to the debug serial port.
/*! @def SERIAL_BAUD_RATE
 * Serial baud rate to talk to the ESP8266 and to the debug serial port.
 * Altsoftserial in my configration (non-ideal level shifting) is flaky at
 * 57600bps.*/
#define SERIAL_BAUD_RATE 19200

/*! @var softPort
 *  9 = TX;
 *  8 = RX;
 *  This is the serial port used to communicate with the ESP8266
 */
AltSoftSerial softPort;

ESP8266 * esp; ///<This is the class used to interface the ESP.
/*! var input_line
 * This is used to hold the most recent line read from the ESP.
 */
//! @todo This is a bigtime memory waster - figure out how to eliminate this
#define MAXIMUM_INPUT_LINE_LENGTH 200
char input_line[MAXIMUM_INPUT_LINE_LENGTH+1];

Rubber_Band_Shooter * shooter;///<This is the class used to interface rubber band shooter

#define SHOOTER_HAMMER_PIN      3 ///<The pin to use to control the hammer servo.
#define SHOOTER_ELEVATION_PIN   11 ///<The pin to use to control the hammer servo.

/*!
 * @fn setup
 * 
 * @brief Runs once before the main loop
 * 
 * This function initializes all serial ports then calls the constructor 
 * for the ESP8266 class.
 */
void setup() {
  // Setup the debug serial port
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F(""));
  Serial.println(F("| Serial Port Initialized..."));
  Serial.println(F("| Initializing software serial..."));
  softPort.begin(SERIAL_BAUD_RATE);
  Serial.print(F("|   Done. Free Memory: "));Serial.println(mu_freeRam());

  // Setup the connection to the ESP8266
  Serial.println(F("| Initializing ESP8266..."));
  esp = new ESP8266(&softPort, PRINT_SERIAL_STREAM);
  Serial.print(F("|   Done. Free Memory: "));Serial.println(mu_freeRam());

  shooter = new Rubber_Band_Shooter(SHOOTER_HAMMER_PIN, SHOOTER_ELEVATION_PIN);
  
  if(PRINT_SERIAL_STREAM)
    Serial.println(F("\n\nENTERING INTERACTIVE SERIAL PASSTHROUGH-------------------"));
  else
    Serial.println(F("\n\nREADY TO RECEIVE COMMANDS, BUT NOT ECHOING WIFI DATA------"));

}





/*!
 * @fn loop
 * 
 * @brief main loop for the Arduino
 * 
 */
char channel = 0; ///<The channel on which the last request to me was sent.
void loop() {

  // Read a line (delimited by '\n') from the ESP8266
  if(esp->read_line(input_line,MAXIMUM_INPUT_LINE_LENGTH)){
    //Serial.print("Input Line length:  ");Serial.println(input_line->length());

    //First, parse out the connection channel, zero of none is found
    channel = esp->get_channel_from_string(input_line,50);

    if(strstr_P(input_line,PSTR("GET")) != NULL){
      Serial.print(F("|  GET received on channel ")); Serial.println(channel,DEC);
      if(strstr_P(input_line,PSTR("/config"))){
        //config page has been requested
        Serial.print(F("|     config page requested"));
        esp->send_http_200_with_prefetch(channel,(char *)config_website_text_0,config_website_text_0_len-1,
                                        (char *)config_website_text_2,config_website_text_2_len,
                                        config_website_prefetch, config_website_PREFETCH_LEN-1);
      } else if (strstr_P(input_line,PSTR("/info/networks"))){
        esp->send_networks_list(channel);
      } else {
        Serial.print(F("|     targeting page requested"));
        esp->send_http_200_static(channel,(char *)static_website_text_0,(sizeof(static_website_text_0)-1));      
      }
      Serial.print(F("'| Free: "));Serial.println(mu_freeRam());
    }else {
      if(strstr_P(input_line,PSTR("POST")) != NULL){
        Serial.print(F("|  POST received on channel ")); Serial.println(channel,DEC);
        if(strstr_P(input_line,PSTR("tilt_up")) != NULL){
          Serial.println(F("tilt_up"));
          shooter->turn_up();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strstr_P(input_line,PSTR("tilt_down")) != NULL){
          Serial.println(F("tilt_down"));
          shooter->turn_down();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strstr_P(input_line,PSTR("pan_right")) != NULL){
          Serial.println(F("pan_right"));
          shooter->turn_right();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strstr_P(input_line,PSTR("pan_left")) != NULL){
          Serial.println(F("pan_left"));
          shooter->turn_left();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strstr_P(input_line,PSTR("fire")) != NULL){
          Serial.println(F("FIRRRRRRE!!!!"));
          shooter->fire();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strstr_P(input_line,PSTR("settings"))){
          Serial.println(F("Settings Request Received!"));
          esp->process_settings(channel,input_line,MAXIMUM_INPUT_LINE_LENGTH);
        }else {
          Serial.println(F("OTHER"));
        }
        Serial.print(F("'| Free: "));Serial.println(mu_freeRam());
      }
    }
  }

  // Pass through manual commands to the ESP8266
  unsigned char data;
  while(Serial.available()) {
    data = Serial.read();
    softPort.write(data);
  }
}

