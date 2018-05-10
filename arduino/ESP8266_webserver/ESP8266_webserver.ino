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

//! @todo set esp maxconns to 1
//! @todo when level shifter hardware is available, increase baud to 115200bps
//! @todo Use HTTP "Content Length:" header instead of terminating connections
//! @todo Add a camera to the shooter/website (snap after each turn)
//! @todo Create an ESP initializer program, to discover serial baud and set up settings 
//! @todo Code lint 
//! @todo Implement TLS
//! @todo Make a custom icon for the project.
//! @todo Try to do auto-targeting with camera image        
//! @todo Add fuzz testing of inputs
//! @todo fix the problem with declaring prefetch length as ints


char *strnstr_P(char *haystack, PGM_P needle, size_t haystack_length)
{
    size_t needle_length = strlen_P(needle);
    size_t i;

    for (i = 0; i < haystack_length; i++)
    {
        if (i + needle_length > haystack_length)
        {
            return NULL;
        }

        if (strncmp_P(&haystack[i], needle, needle_length) == 0)
        {
            return &haystack[i];
        }
    }
    return NULL;
}



#include <AltSoftSerial.h>
#include <MemoryUsage.h>
#include "ESP8266.h"
#include "Rubber_Band_Shooter.h"


#define DEBUG_MEMORY false ///<flag to enable serial port prints indicating amount of free heap.


#if DEBUG_MEMORY
#define PRINT_FREE_MEMORY() {Serial.print(F("'| Free: "));Serial.println(mu_freeRam());}
#else
#define PRINT_FREE_MEMORY() {}
#endif


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
//! @todo This is a bigtime memory waster - implement everything I'm doing here in the ESP8266 class to existing buffers
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
  esp = new ESP8266(&softPort, PRINT_SERIAL_STREAM, 0);
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

    if(strnstr_P(input_line,PSTR("GET"),MAXIMUM_INPUT_LINE_LENGTH) != NULL){
      Serial.print(F("|  GET received on channel ")); Serial.println(channel,DEC);
      if(strnstr_P(input_line,PSTR("/config"),MAXIMUM_INPUT_LINE_LENGTH)){
        //config page has been requested
        Serial.println(F("|     config page requested"));
        esp->send_http_200_with_prefetch(channel,(char *)config_website_text_0,config_website_text_0_len-1,
                                        (char *)config_website_text_2,config_website_text_2_len,
                                        config_website_prefetch, config_website_PREFETCH_LEN-1);
      } else if (strnstr_P(input_line,PSTR("/info/networks"),MAXIMUM_INPUT_LINE_LENGTH)){
        esp->send_networks_list(channel);
      } else {
        Serial.println(F("|     targeting page requested"));
        esp->send_http_200_static(channel,(char *)static_website_text_0,(sizeof(static_website_text_0)-1));      
      }
      PRINT_FREE_MEMORY();
    }else {
      if(strnstr_P(input_line,PSTR("POST"),MAXIMUM_INPUT_LINE_LENGTH) != NULL){
        Serial.print(F("|  POST received on channel ")); Serial.println(channel,DEC);
        if(strnstr_P(input_line,PSTR("tilt_up"),MAXIMUM_INPUT_LINE_LENGTH) != NULL){
          Serial.println(F("tilt_up"));
          shooter->turn_up();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strnstr_P(input_line,PSTR("tilt_down"),MAXIMUM_INPUT_LINE_LENGTH) != NULL){
          Serial.println(F("tilt_down"));
          shooter->turn_down();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strnstr_P(input_line,PSTR("pan_right"),MAXIMUM_INPUT_LINE_LENGTH) != NULL){
          Serial.println(F("pan_right"));
          shooter->turn_right();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strnstr_P(input_line,PSTR("pan_left"),MAXIMUM_INPUT_LINE_LENGTH) != NULL){
          Serial.println(F("pan_left"));
          shooter->turn_left();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strnstr_P(input_line,PSTR("fire"),MAXIMUM_INPUT_LINE_LENGTH) != NULL){
          Serial.println(F("FIRRRRRRE!!!!"));
          shooter->fire();
          esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
        } else if(strnstr_P(input_line,PSTR("settings"),MAXIMUM_INPUT_LINE_LENGTH)){
          Serial.println(F("Settings Request Received!"));
          esp->process_settings(channel,input_line,MAXIMUM_INPUT_LINE_LENGTH);
        }else {
          Serial.println(F("OTHER"));
        }
        PRINT_FREE_MEMORY();
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

