/*---------------------------------------------------------------
 * ESP8266_webserver
 * 
 * The sole intention of this project is to shoot things with rubber bands using the coolest technology possible.
 * 
 * This program uses the ESP8266 class to serve up a website. The
 * eventual objective is to merge this code with 
 * RubberBandTerminator.ino to control the rubber band launching
 * device from my phone, tablet, the internet, or whatever.
 * 
 * RESEARCH LINKS: 
 *     https://github.com/sleemanj/ESP8266_Simple/blob/master/README.md
 *     https://nurdspace.nl/ESP8266
 *     http://www.electrodragon.com/w/images/f/ff/4a-esp8266_at_instruction_set_en_%281%29.pdf
 *     https://github.com/sleemanj/ESP8266_Simple/blob/master/arduino-wiring-diagram.jpg
 *     https://smile.amazon.com/gp/product/B01EA3UJJ4/ref=oh_aui_detailpage_o02_s00?ie=UTF8&psc=1
 *     https://hackingmajenkoblog.wordpress.com/2016/02/04/the-evils-of-arduino-strings/ 
 *     https://www.youtube.com/watch?v=ETLDW22zoMA&t=9s
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
 * PLANS:
 *          * Scrub all code for todo's
 *          * Flesh out webserver stack - make more reliable
 *          * Store the website in EEPROM
 *          * Use a CDN (like CloudFront) for icons, graphics and other eye candy (maybe S3?)
 *          * Use HTTP "Content Length:" header instead of terminating connections
 *          * Remove all delay() calls in code
 *          * Enable watchdog timer
 *          * Use AJAX for command buttons - don't reload the website for each button press
 *          * Add super-basic MVC to the webserver
 *          * Add a camera to the shooter/website (snap after each turn)
 *          * Create an ESP initializer program, since 
 *          * Assemble more better-integrated ESP boards
 *          * Build a protoboard with everything on it
 *          * Connectorize harness to board (0.1in block connector)
 *          * Consolidate all constants, etc to a header file
 *          * Get a better name for the webserver
 *          * Code lint 
 *          * Implement TLS
 *          * Figure out what other goodness the ESP8266 gives us
 *          * Make the ESP8266 an access point
 *          * Use access point functionality for device setup
 *          * Support multiple connections (instead of just connection 0)
 *          * Make a you tube video and link in readme
 *          * Make a custom icon for the project.
 *          * Try to do auto-targeting with camera image        
 */
#include <AltSoftSerial.h>
#include <MemoryUsage.h>
#include "ESP8266.h"
#include "webserver_constants.h"
#include "rubber_band_shooter.h"

#define DEBUG_MEMORY true

//// Serial Port Definitions
#define PRINT_SERIAL_STREAM false
#define SERIAL_BAUD_RATE 19200

///SoftwareSerial softPort(9,8); //TX,RX
AltSoftSerial softPort;

//// ESP8266 Definitions
ESP8266 * esp;
char input_line[SERIAL_INPUT_BUFFER_MAX_SIZE+1];

Rubber_Band_Shooter * shooter;

#define SHOOTER_HAMMER_PIN      3
#define SHOOTER_ELEVATION_PIN   11

/*---------------------------------------------------------------
 * SETUP
 *-------------------------------------------------------------*/
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

/*---------------------------------------------------------------
 * LOOP
 *-------------------------------------------------------------*/


char get_channel(char line[], int len){
  char string_to_parse[len]; 
  strncpy(string_to_parse,line,len);
  char rv = 0;

  //Try to find the channel indicator
  char * token = strstr_P(string_to_parse, PSTR("IPD,"));

  if(token != NULL){
    strtok(token,","); //this is "IPD"
    char * chan_string = strtok(NULL,","); //this is the channel ID
    rv = atoi(chan_string);
  }
  return rv;
  
}

char channel = 0;
void loop() {

  
  // Read a line (delimited by '\n') from the ESP8266
  if(esp->read_line(input_line)){
    //Serial.print("Input Line length:  ");Serial.println(input_line->length());

    //First, parse out the connection channel, zero of none is found
    channel = get_channel(input_line,50);

    if(strstr_P(input_line,PSTR("GET")) != NULL){
      Serial.print(F("|  GET received on channel ")); Serial.println(channel);
      esp->send_http_200_static(channel,(char *)static_website_text,(sizeof(static_website_text)-1));
      Serial.print(F("'| Free: "));Serial.println(mu_freeRam());
    }else {
      if(strstr_P(input_line,PSTR("POST")) != NULL){
        Serial.print(F("|  POST received on channel ")); Serial.println(channel,DEC);
        if(strstr_P(input_line,PSTR("tilt_up")) != NULL){
          Serial.println(F("tilt_up"));
          shooter->turn_up();
        } else if(strstr_P(input_line,PSTR("tilt_down")) != NULL){
          Serial.println(F("tilt_down"));
          shooter->turn_down();
        } else if(strstr_P(input_line,PSTR("pan_right")) != NULL){
          Serial.println("pan_right");
          shooter->turn_right();
        } else if(strstr_P(input_line,PSTR("pan_left")) != NULL){
          Serial.println(F("pan_left"));
          shooter->turn_left();
        } else if(strstr_P(input_line,PSTR("fire")) != NULL){
          Serial.println(F("FIRRRRRRE!!!!"));
          shooter->fire();
        }else {
          Serial.println(F("OTHER"));
        }
        //todo: only refresh a status section on form button submit: 
        esp->send_http_200_static(channel,(char *)blank_website_text,(sizeof(blank_website_text)-1));
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

