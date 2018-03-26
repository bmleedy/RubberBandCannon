/*---------------------------------------------------------------
 * ESP8266_webserver
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
 * 
 * ESP8266 SETUP:
 *     Default baud rate on the chips: 115200bps
 *     To enable the chips: set all middle pins to +3.3v
 *     Default baud rate is 115200 (software serial port is flaky at this rate)
 *     Use AT+UART_DEF=57600,8,1,0,0 to switch to 57600
 * 
 * SOFTWARESERIAL SETUP:
 * The pin used for RX must support change interrupts: 
 *                https://playground.arduino.cc/Code/Interrupts 
 *                
 * PLANS:
 *     * Test the new \n comparison, should be a lot faster
 *     * Get the elevation axis stabilized - figure out what's going on there
 *     * Implement ring buffer
 *     * Move all strings to progmeme
 *     * OK and FAIL strings to a single function, so we only store it in in place
 *     * Make button width and height 100% and give them some color
 *     * Make button table height bigger
 *     * Scrub all code for todo's
 *     * De-stringify
 *     * Consider adding I2C-to-UART bridge for better Serial performance
 *        https://os.mbed.com/users/wim/notebook/sc16is750-i2c-or-spi-to-uart-bridge/
 *        https://www.nxp.com/docs/en/brochure/75015676.pdf
 *        https://www.maximintegrated.com/en/products/interface/controllers-expanders/MAX3107.html
 *        https://www.nxp.com/products/analog/signal-chain/bridges/single-uart-with-i2c-bus-spi-interface-64-bytes-of-transmit-and-receive-fifos-irda-sir-built-in-support:SC16IS740_750_760?tab=Buy_Parametric_Tab&amp;fromSearch=false
 *        https://forum.arduino.cc/index.php?topic=419359.0
 *        
 */
//#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <MemoryUsage.h>
#include "ESP8266.h"
#include "webserver_constants.h"
#include "rubber_band_shooter.h"

#define DEBUG_MEMORY true

//// Serial Port Definitions
#define PRINT_SERIAL_STREAM false
#define SERIAL_BAUD_RATE 57600

///SoftwareSerial softPort(8,9); //TX,RX
AltSoftSerial softPort(8,9);

//// ESP8266 Definitions
ESP8266 * esp;
char input_line[SERIAL_INPUT_BUFFER_MAX_SIZE+1];

Rubber_Band_Shooter * shooter;

/*---------------------------------------------------------------
 * SETUP
 *-------------------------------------------------------------*/
void setup() {
  // Setup the debug serial port
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("");
  Serial.println(F("| Serial Port Initialized..."));
  Serial.println(F("| Initializing software serial..."));
  softPort.begin(SERIAL_BAUD_RATE);
  Serial.print(F("|   Done. Free Memory: "));Serial.println(mu_freeRam());

  // Setup the connection to the ESP8266
  Serial.println(F("| Initializing ESP8266..."));
  esp = new ESP8266(&softPort, PRINT_SERIAL_STREAM);
  Serial.print(F("|   Done. Free Memory: "));Serial.println(mu_freeRam());

  shooter = new Rubber_Band_Shooter();
  
  if(PRINT_SERIAL_STREAM)
    Serial.println(F("\n\nENTERING INTERACTIVE SERIAL PASSTHROUGH-------------------"));
  else
    Serial.println(F("\n\nREADY TO RECEIVE COMMANDS, BUT NOT ECHOING WIFI DATA------"));

}

/*---------------------------------------------------------------
 * LOOP
 *-------------------------------------------------------------*/



void loop() {

  
  // Read a line (delimited by '\n') from the ESP8266
  if(esp->read_line(input_line)){
    Serial.print(F("| Line:   '"));Serial.print(input_line);Serial.print(F("'| Free: "));Serial.println(mu_freeRam());
    //Serial.print("Input Line length:  ");Serial.println(input_line->length());
    if(strstr(input_line,"GET") != NULL){
      //Serial.print(F("|   Free Memory: "));Serial.println(mu_freeRam());
      esp->send_http_200_static(0,(char *)static_website_text,sizeof(static_website_text));
      Serial.println(F("|  GET received"));
    }else {
      if(strstr(input_line,"POST") != NULL){
        Serial.print(F("|  POST received : "));
        if(strstr(input_line,"tilt_up") != NULL){
          Serial.println(F("tilt_up"));
          shooter->turn_up();
        } else if(strstr(input_line,"tilt_down") != NULL){
          Serial.println(F("tilt_down"));
          shooter->turn_down();
        } else if(strstr(input_line,"pan_right") != NULL){
          Serial.println(F("pan_right"));
          shooter->turn_right();
        } else if(strstr(input_line,"pan_left") != NULL){
          Serial.println(F("pan_left"));
          shooter->turn_left();
        } else if(strstr(input_line,"fire") != NULL){
          Serial.println(F("FIIIIRRRRRREEEE!!!!!!!!!!!!!!"));
          shooter->fire();
        }else {
          Serial.println(F("OTHER"));
        }
        esp->send_http_200_static(0,(char *)static_website_text,sizeof(static_website_text));
      }
 
    }
    esp->clear_buffer();
  }

  // todo: consider moving esp8266 to hardware serial

  // Read any data from the serial port and output it to the esp8266 port
  // (this is for manual commands, etc.)
  while(Serial.available()) {
    unsigned char data = Serial.read();
    softPort.write(data);
  }
}

