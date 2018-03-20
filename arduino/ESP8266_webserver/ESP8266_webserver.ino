/* Software Serial Passthrough 
 *  
 */
#include <SoftwareSerial.h>
#include "ESP8266.h"

/*
 * Looks like the best place to start: 
 *     https://github.com/sleemanj/ESP8266_Simple/blob/master/README.md
 *     
 * Links from the amazon page: 
 *     https://nurdspace.nl/ESP8266
 *     
 * Datasheet for the chip:     
 *     http://www.electrodragon.com/w/images/f/ff/4a-esp8266_at_instruction_set_en_%281%29.pdf
 * 
 * Level shifting: 
 *     https://github.com/sleemanj/ESP8266_Simple/blob/master/arduino-wiring-diagram.jpg
 *  
 * Amazon page with links: 
 *     https://smile.amazon.com/gp/product/B01EA3UJJ4/ref=oh_aui_detailpage_o02_s00?ie=UTF8&psc=1
 *     
 * Default baud rate on the chips:
 * 
 * To enable the chips: set all middle pins to +3.3v
 * Default baud rate is 115200 (software serial port is flaky at this rate)
 * Use AT+UART_DEF=57600,8,1,0,0 to switch to 57600
 * Use AT+CWMODE_DEF=
 *  
 *  
 *  https://hackingmajenkoblog.wordpress.com/2016/02/04/the-evils-of-arduino-strings/ 

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 */

#define PRINT_SERIAL_STREAM true
#define SERIAL_BAUD_RATE 57600


int last_output_time = 0;

const char static_website_text[] PROGMEM =
"<!DOCTYPE html>\r\n \
<html>\r\n \
  <head>\r\n \
    <title>Brett's IOT Device</title>\r\n \
  </head>\r\n \
  <body>\r\n \
    <h1>Hello, fine world.</h1>\r\n \
      <p>I am Brett. :-)</p>\r\n \
    <h1>Arduino Stats.</h1>\r\n \
      <p>todo: put some stats in here.</p>\r\n \
  </body>\r\n \
</html>\r\n";


String helloworld_page(){  //todo: fix to pass pointer to rv instead


  //todo: use a CDN to source images, and possibly other page content for this device
  // e.g.  <img src="w3schools.jpg" alt="W3Schools.com" width="104" height="142"> 
  // https://www.w3schools.com/html/html_basic.asp

  // Just give it a super-simple html structure
  // todo: make this an encapsulating class
  String content = "";

 
  content.concat(String(F("<!DOCTYPE html>\r\n")));
  content.concat(String(F("<html>\r\n")));
  content.concat(String(F("<head>\r\n")));
  content.concat(String(F("<title>Brett's IOT Device</title>\r\n")));
  content.concat(String(F("</head>\r\n")));
  content.concat(String(F("<body>\r\n")));  //make all this stringifying and concatenation a cleaner class method
  content.concat(String(F("<h1>Hello, fine world.</h1>\r\n")));
  content.concat(String(F("<p>I am Brett. :-)</p>\r\n")));
  content.concat(String(F("<h1>Arduino Stats.</h1>\r\n")));
  content.concat(String(F("<p>todo: put some stats in here.</p>\r\n")));
  content.concat(String(F("</body>\r\n")));
  content.concat(String(F("</html>\r\n")));
  
  return content;
}


//softPort(TX,RX)
SoftwareSerial softPort(8,9);
ESP8266 * esp;

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

  esp = new ESP8266(&softPort, PRINT_SERIAL_STREAM);

  if(PRINT_SERIAL_STREAM)
    Serial.println(F("\n\nENTERING INTERACTIVE SERIAL PASSTHROUGH-------------------"));
  else
    Serial.println(F("\n\nREADY TO RECEIVE COMMANDS, BUT NOT ECHOING WIFI DATA------"));
}





void loop() {
  char latest_byte = ' ';
  // put your main code here, to run repeatedly:

  if(esp->check_for_request(F("GET"))){
    Serial.println(F("got a request!!!"));
    esp->send_http_200(0,helloworld_page());
    esp->send_http_200_static(0,static_website_text,sizeof(static_website_text));
  }

///check_for_request here
  
  if (Serial.available()) {
    unsigned char data = Serial.read();
    softPort.write(data);
//    Serial.write(data);
  }
}
//append to a string until I get a newline, then process it


/*
#define BUFFER_SIZE 200 //bytes
class Fifo
{
  char data[BUFFER_SIZE];
  char * writePointer; //pointer to the next object to be written
  char * readPointer;  //pointer to the last object read

  Fifo(){
    write_pointer=data;
    read_pointer=data;
  }

  // Add a byte to the queue
  void add_byte(char * write_byte){
    if(write_pointer==data + BUFFER_SIZE){
      //roll over
      write_pointer=data;
    }
    *write_pointer = *write_byte;
    write_pointer++;
    if(write_pointer==read_pointer){
      //we just caught up to the read pointer
      //push it forward by reading one byte.
      //it will wrap automatically
      read_byte();  
    }
  }

  // Get the next byte to read
  char read_byte(){
    char * read_pointer_orig = read_pointer;
    read_pointer++;
    if(read_pointer == data + BUFFER_SIZE){
      //roll over
      read_pointer=data;
    }
    //Check for collision with read pointer
    if(read_pointer == write_pointer){
      //crap = bad code - should have called peek before reading
      //rollback my pointer and return a "-1" character
      read_pointer=read_pointer_orig;
      return -1;
    }

    return *read_pointer;
  }


  //returns whether any bytes are available
  bool bytes_available(){
    if(read_pointer == (write_pointer-1) ||
       (write_pointer==data && read_pointer==data+BUFFER_SIZE-1)){
      return false;
    }
  }


  
  char *peek_byte();
  void clear();
  String get_n_bytes_as_string(int num_bytes);
  String get_all_bytes_as_string();
  void eat_bytes(int_num_bytes);
  add_byte
}


//Read the data coming an and try to find a "GET / HTTP/1.1" and return a hello world website.
"HTTP/1.1 200 OK\r\n\r\n\r\n\r\nHello World, I'm an IOT device!"

/*
  if( (millis() - last_output_time) > 5000){
    softPort.write(D("AT+CWMODE?\r\n"));
    last_output_time = millis();
  }
*/
//}

