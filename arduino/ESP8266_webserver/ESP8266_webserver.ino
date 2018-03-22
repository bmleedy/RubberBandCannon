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
 */
#include <SoftwareSerial.h>
#include <MemoryUsage.h>
#include "ESP8266.h"
#include "webserver_constants.h"


#define DEBUG_MEMORY true

//// Serial Port Definitions
#define PRINT_SERIAL_STREAM true
#define SERIAL_BAUD_RATE 57600

//todo: can't create a pointer to flash memory, so need a way to access flash mem at specific addresses.
const static char static_website_text[] = "\n<html><head><title>Brett's IOT Device</title></head><body><h1>Hello, fine world.</h1><p>I am Brett. :-)</p><h1>Arduino Stats.</h1><p>todo: put some stats in here.</p></body></html>";

SoftwareSerial softPort(8,9); //TX,RX


//// ESP8266 Definitions
ESP8266 * esp;


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

  Serial.print("STATIC WEBSITE DATA:\n[");
  Serial.write(static_website_text,sizeof(static_website_text)-1);
  Serial.println("]");
  
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
  if(esp->check_for_request(F("GET"))){
    Serial.println(F("\n| Got a request!!!"));
    Serial.print(F("|   Free Memory: "));Serial.println(mu_freeRam());
    esp->send_http_200_static(0,(char*)static_website_text,sizeof(static_website_text));
    Serial.print(F("\n|   Response complete. Free Memory: "));Serial.println(mu_freeRam());
  }

  // Read any data from the serial port and output it to the esp8266 port
  // (this is for manual commands, etc.)
  while(Serial.available()) {
    unsigned char data = Serial.read();
    softPort.write(data);
  }
}

