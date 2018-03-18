/* Software Serial Passthrough 
 *  
 */
#include <SoftwareSerial.h>


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
 */

#define PRINT_SERIAL_STREAM false

bool expect_response_to_command(String * command, 
                                String * response, 
                                SoftwareSerial *port, 
                                unsigned int timeout_ms=2000){
  String input_buffer = "";
  input_buffer.reserve(100);

  // Send a basic query every __ seconds till I get an "OK" response back
  // todo: turn this functionality into a function for any string and response.
  //bool searching = true;
  //while(searching){
      
    // Write the command
    port->write(String(*command + "\r\n").c_str());
    if(PRINT_SERIAL_STREAM){Serial.println(String(">>>" + *command));}
    
    // Spin for timeout_ms
    unsigned int start_time = millis();
    char rv = -1;
    while((millis() - start_time) < timeout_ms){

      // Read 1 char off the serial port.
      rv = port->read();
      if (rv != -1) {
        if(PRINT_SERIAL_STREAM){Serial.write(rv);}
        input_buffer = String(input_buffer + String(rv));
        if(input_buffer.indexOf(*response) != -1) {
          return true;
        } 
      }//if(rv != 1)
      
    }//while(millis...)
  return false;
}

bool print_response_to_command(String * command, 
                                SoftwareSerial *port, 
                                unsigned int timeout_ms=2000){
  String input_buffer = "";
  input_buffer.reserve(100);
      
  // Write the command
  port->write(String(*command + "\r\n").c_str());
  if(PRINT_SERIAL_STREAM){Serial.println(String(">>>" + *command));}
  
  // Spin for timeout_ms
  unsigned int start_time = millis();
  char rv = -1;
  while((millis() - start_time) < timeout_ms){

    // Read 1 char off the serial port.
    rv = port->read();
    if (rv != -1) {
      Serial.write(rv);
      input_buffer = String(input_buffer + String(rv));
      if(input_buffer.indexOf("\r\n\r\nOK") != -1) {
        return true;
      } 
    }//if(rv != 1)
    
  }//while(millis...)
  return false;
}



#define SERIAL_BAUD_RATE 57600
#define SERIAL_INPUT_BUFFER_MAX_SIZE 500



int last_output_time = 0;

struct web_page {
  String content;
  unsigned char channel;
}rv;

void render_page_for_channel(unsigned char channel,SoftwareSerial *port){  //todo: fix to pass pointer to rv instead
  rv.content = "";

  //todo: maybe don't use string (not sure what happens under there) in favor of a fixed-size char[] response buffer.
  
  //HTTP response
  rv.content.concat(String("HTTP/1.1 200 OK\r\n\r\n"));
  //todo: add Content_Length header so I don't have to close the connection. Or, maybe I want to close the connection anyway.
  // https://www.w3.org/Protocols/HTTP/Response.html

  //todo: use a CDN to source images, and possibly other page content for this device
  // e.g.  <img src="w3schools.jpg" alt="W3Schools.com" width="104" height="142"> 
  // https://www.w3schools.com/html/html_basic.asp

  

  // Just give it a super-simple html structure
  // todo: make this an encapsulating class
  rv.content.concat(String(F("<!DOCTYPE html>\r\n")));
  rv.content.concat(String(F("<html>\r\n")));
  rv.content.concat(String(F("<head>\r\n")));
  rv.content.concat(String(F("<title>Brett's IOT Device</title>\r\n")));
  rv.content.concat(String(F("</head>\r\n")));
  rv.content.concat(String(F("<body>\r\n")));  //make all this stringifying and concatenation a cleaner class method
  rv.content.concat(String(F("<h1>Hello, fine world.</h1>\r\n")));
  rv.content.concat(String(F("<p>I am Brett. :-)</p>\r\n")));
  rv.content.concat(String(F("<h1>Arduino Stats.</h1>\r\n")));
  rv.content.concat(String(F("<p>todo: put some stats in here.</p>\r\n")));
  rv.content.concat(String(F("</body>\r\n")));
  rv.content.concat(String(F("</html>\r\n")));
  
  rv.channel = channel;

  
  String write_command = String(String("AT+CIPSEND=")+//todo: see if I can use the append method
                                String(rv.channel)+
                                String(",")+
                                String(rv.content.length())+
                                String("\r\n"));//todo: figure out if I need to do all the string conversions.
  port->write(write_command.c_str());
  delay(20);
  port->write(rv.content.c_str());
  delay(20);
  write_command = String(String("AT+CIPCLOSE=")+
                         String(rv.channel)+
                         String("\r\n"));

  //todo: verify that all of this succeeded.
  port->write(write_command.c_str());
  //https://www.youtube.com/watch?v=ETLDW22zoMA&t=9s
}


String serial_input_buffer = "";

/*
 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).
*/
//softPort(TX,RX)
SoftwareSerial softPort(8,9);

void setup() {

  serial_input_buffer.reserve(SERIAL_INPUT_BUFFER_MAX_SIZE);
  
  // Setup the debug serial port
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  
  Serial.println("");
  Serial.println(F("| Serial Port Initialized..."));
  Serial.println(F("| Initializing software serial..."));
  softPort.begin(SERIAL_BAUD_RATE);

  // Get a response from anyone
  Serial.print(F("| Waiting for a response from the Wifi Device..."));
  while(!expect_response_to_command(&String("AT"),&String("OK"),&softPort)){
    delay(100);
  }
  Serial.println("[OK]");

  Serial.print(F("| Checking the device CWMODE..."));
  // Set myself up as a client of an access point.
  if(expect_response_to_command(&String("AT+CWMODE?"),&String("+CWMODE:1"),&softPort)){
    Serial.println(F("[OK]"));
  } else {
    Serial.print (F("\n|    Setting the mode to 'client mode'"));
    if(expect_response_to_command(&String("AT+CWMODE=1"),&String("OK"),&softPort)){
      Serial.println(F("[OK]"));
    }
    else {
      Serial.println(F("[FAIL]"));
    }
  }


  // Now join the house access point
  Serial.print(F("| Checking that we are on the 'leedy' network..."));
  if(expect_response_to_command(&String("AT+CWJAP?"),&String("+CWJAP:\"leedy\""),&softPort)){
    Serial.println(F("[OK]"));
  } else {
    Serial.print (F("\n|    Not on the 'leedy' network.  Changing the WiFi settings to join network..."));
    if(expect_response_to_command(&String("AT+CWJAP=\"leedy\",\"teamgoat\""),&String("OK"),&softPort,10000)){
      Serial.println(F("[OK]"));
    }
    else {
      Serial.println(F("[FAIL]"));
    }
  }

  // Set ourselves up to mux connections into our little server
  Serial.print(F("| Checking the CIPMUX Settings..."));
  if(expect_response_to_command(&String("AT+CIPMUX?"),&String("+CIPMUX:1"),&softPort)){
    Serial.println(F("[OK]"));
  } else {
    Serial.print (F("\n|    Server not enabled yet. Setting CIPMUX=1..."));
    if(expect_response_to_command(&String("AT+CIPMUX=1"),&String("OK"),&softPort,10000)){
      Serial.println(F("[OK]"));
    }
    else {
      Serial.println(F("[FAIL]"));
    }
  }

  // Now setup the CIP Server
  Serial.print(F("| Configuring my server on port 8080..."));
  if(expect_response_to_command(&String("AT+CIPSERVER=1,8080"),&String("OK"),&softPort)){
    Serial.println(F("[OK]"));
  } else {
    Serial.println(F("[FAIL]"));
  }

  //Info Messages
  Serial.println(F("\n\nVERSION INFO---------------------"));
  print_response_to_command(&String("AT+GMR"), &softPort);
  Serial.println(F("\n\nIP Address INFO---------------------"));
  print_response_to_command(&String("AT+CIFSR"), &softPort);

  if(PRINT_SERIAL_STREAM)
    Serial.println(F("\n\nENTERING INTERACTIVE SERIAL PASSTHROUGH-------------------"));
  else
    Serial.println(F("\n\nREADY TO RECEIVE COMMANDS, BUT NOT ECHOING WIFI DATA------"));
}





void loop() {
  char latest_byte = ' ';
  web_page output_page;
  // put your main code here, to run repeatedly:
  if (softPort.available()) {
    latest_byte = softPort.read();
    if(PRINT_SERIAL_STREAM){Serial.write(latest_byte);}
    //Need to replace this with a proper ring buffer or long lines could crash me. :-O //todo//
    serial_input_buffer = String(serial_input_buffer + String(latest_byte)); //todo: probably really inefficient
    if(serial_input_buffer.indexOf("\n") != -1) {
      if(serial_input_buffer.indexOf("GET") != -1){
        render_page_for_channel(0,&softPort);
        Serial.println("| Rendered a page");
      }
      serial_input_buffer = "";
      } 
  }
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

