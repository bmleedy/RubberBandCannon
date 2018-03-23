/*------------------------------------------------------------------------------
 * ESP8266
 *  
 * This class is an ESP8266 for the purposes of my IOT work.  On creation,
 * it will set the device to:
 *  * Only act as a client of an access point, not an access point itself
 *  * Connect to the house access point
 *  * Serve a multi-connection TCP server on port 8080
 * 
 * You then interact with it's public methods to use the server:
 *  * Check_for_requests(bool verbose) - reads one line from the SoftwareSerial
 *    port and returns TRUE if a line contains a string specified by the caller.
 * 
 * TODO: 
 *   * replace all String class instances to reduce heap fragmentation 
 ------------------------------------------------------------------------------*/
#include "ESP8266.h"

// Constructor: SoftwareSerial port must be initialized
ESP8266::ESP8266(SoftwareSerial *port, bool verbose){
  this->port = port;
  serial_input_buffer.reserve(SERIAL_INPUT_BUFFER_MAX_SIZE);
  this->setup_device();
  this->verbose = verbose;
}


// check_for_request: read a line from the port and respond whether it matches a pattern.
bool ESP8266::check_for_request(String matchtext){
  char latest_byte = '\0';
  while (this->port->available()) {
    latest_byte = this->port->read();
    if(verbose){Serial.write(latest_byte);}
    //Need to replace this with a proper ring buffer or long lines could crash me. :-O //todo//
    // todo: de-stringify.
    serial_input_buffer = String(serial_input_buffer + String(latest_byte)); //todo: probably really inefficient
    if(serial_input_buffer.indexOf("\n") != -1) {
      if(serial_input_buffer.indexOf(matchtext) != -1){
        //render_page_for_channel(0,&softPort); //todo deleteme
        this->serial_input_buffer = "";  //todo: leave the input buffer alone instead of clearing it.
        return true;
      }
      else{
        return false;
      }
    }
  }
}

// check_for_request: read a line from the port and respond whether it matches a pattern.
bool ESP8266::read_line(String * line_buffer, int * line_length){
  char latest_byte = '\0';
  while (this->port->available()) {
    latest_byte = this->port->read();
    if(verbose){Serial.write(latest_byte);}
    //Need to replace this with a proper ring buffer or long lines could crash me. :-O //todo//
    // todo: de-stringify.
    *line_buffer = String(*line_buffer + String(latest_byte)); //todo: probably really inefficient
    if(line_buffer->indexOf("\n") != -1) {
      //Serial.println("Line Received!!!!!!!");
      Serial.print("Line:   '");Serial.print(*line_buffer);Serial.println("'");
      //return true if we found a newline
      return true;
    }
  }
  return false;
}

void ESP8266::clear_buffer(){
  serial_input_buffer = String("");
}

// Send a command and expect a string in response
bool ESP8266::expect_response_to_command(String command,
                                String response,
                                unsigned int timeout_ms){
  String input_buffer = "";
  input_buffer.reserve(100);

  // Write the command
  this->port->write(String(command + "\r\n").c_str());
  if(this->verbose){Serial.println(String(">>>" + command));}
  
  // Spin for timeout_ms
  unsigned int start_time = millis();
  char rv = -1;
  while((millis() - start_time) < timeout_ms){
    
    // Read 1 char off the serial port.
    rv = this->port->read();
    if (rv != -1) {
      if(this->verbose){Serial.write(rv);}
      input_buffer = String(input_buffer + String(rv));
      if(input_buffer.indexOf(response) != -1) {
        return true;
      }
    }//if(rv != 1)
  }//while(millis...)
  return false;
}


// Send a command and print the response
bool ESP8266::print_response_to_command(String command,
                               unsigned int timeout_ms){
  String input_buffer = "";
  input_buffer.reserve(100);
  
  // Write the command
  this->port->write(String(command + "\r\n").c_str());
  if(this->verbose){Serial.println(String(">>>" + command));}
  
  // Spin for timeout_ms
  unsigned int start_time = millis();
  char rv = -1;
  while((millis() - start_time) < timeout_ms){
      // Read 1 char off the serial port.
      rv = this->port->read();
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


// Setup the ESP8266 as a webserver
bool ESP8266::setup_device(){
    // Get a response from anyone
    Serial.print(F("ESP8266 - Waiting for a response from the Wifi Device..."));
    while(!expect_response_to_command(String("AT"),String("OK"))){
        delay(100);
    }
    Serial.println("[OK]");
    
    Serial.print(F("ESP8266 - Checking the device CWMODE..."));
    // Set myself up as a client of an access point.
    if(expect_response_to_command(String("AT+CWMODE?"),String("+CWMODE:1"))){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -    Setting the mode to 'client mode'"));
        if(expect_response_to_command(String("AT+CWMODE=1"),String("OK"))){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Now join the house access point
    Serial.print(F("ESP8266 - Checking that we are on the 'leedy' network..."));
    if(expect_response_to_command(String("AT+CWJAP?"),String("+CWJAP:\"leedy\""))){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -     Not on the 'leedy' network.  Changing the WiFi settings to join network..."));
        if(expect_response_to_command(String("AT+CWJAP=\"leedy\",\"teamgoat\""),String("OK"),10000)){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Set ourselves up to mux connections into our little server
    Serial.print(F("ESP8266 - Checking the CIPMUX Settings..."));
    if(expect_response_to_command(String("AT+CIPMUX?"),String("+CIPMUX:1"))){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -    Server not enabled yet. Setting CIPMUX=1..."));
        if(expect_response_to_command(String("AT+CIPMUX=1"),String("OK"),10000)){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Now setup the CIP Server
    Serial.print(F("ESP8266 - Configuring my server on port 8080..."));
    if(expect_response_to_command(String("AT+CIPSERVER=1,8080"),String("OK"),10000)){
        Serial.println(F("[OK]"));
    } else {
        Serial.println(F("[FAIL]"));
        return false;
    }
}


//Send the contents of my output queue to a specific channel
void ESP8266::send_output_queue(unsigned char channel){
    
    // Command the esp to listen for n bytes of data  // todo: make less stringy
    String write_command = String(String("AT+CIPSEND=")+
                                  String(channel)+
                                  String(",")+
                                  String(output_queue.get_total_size())+
                                  String("\r\n"));//todo: make this less stringy.
    port->write(write_command.c_str());
    delay(20);

    string_element data_to_write;
    //while we can retrieve things from the output queue
    while(output_queue.get_element(&data_to_write)){
      if(data_to_write.is_progmem){
        for(int i=0;i<data_to_write.string_length;i++){
          port->write(pgm_read_byte(data_to_write.pointer + i));
        }
      }else{
        port->write(data_to_write.pointer,data_to_write.string_length);
      }
      //delay(20);  //from Espressif ICD
    }
    Serial.println("Done Sending!!!");
    // TODO: This is still pretty brittle.  Repeat the cipclose command 
    //until I get some kind of response, either success or failure from 
    //the ESP.
    
    // Send the command to terminate the data stream, 
    //   even though it should have been terminated based on length.
    port->write("+++",3);

    // Wait 100 ms before sending any other command, per the Espressif ICD
    delay(100); 
    
    // Now close the connection  // todo: make less stringy
    write_command = String(String("AT+CIPCLOSE=")+
                           String(channel)+
                           String("\r\n"));
    // todo: validate that these commands actually worked.  Right now they're open loop.
    port->write(write_command.c_str());
    //https://www.youtube.com/watch?v=ETLDW22zoMA&t=9s
}


// Send a static website as an HTTP 200 response
void ESP8266::send_http_200_static(unsigned char channel, char page_data[], unsigned int page_data_len){

  int total_page_size = 0;

  //todo: add Content_Length header so I don't have to close the connection. 
  //      Or, maybe I want to close the connection anyway.
  // https://www.w3.org/Protocols/HTTP/Response.html

  // start-line per https://tools.ietf.org/html/rfc2616#page-31 
  this->output_queue.add_element((char *)http_200_start_line, HTTP_200_START_LINE_LEN, true);

  // Now enqueue the website page data
  this->output_queue.add_element(page_data, page_data_len,true);
  
  // Send!
  this->send_output_queue(channel);
}



/*-----------------------------------------------------------------
 * OutputQueue
 * 
 * Holds pointers to strings, their sizes, and a tally of the sizes
 * of all of the strings that need to be outputted.
 * 
 * (see header file for usage)
 *-----------------------------------------------------------------
 */

OutputQueue::OutputQueue(){
  this->clear_elements();
}


// Add an element to this queue.
void OutputQueue::add_element(char * string, unsigned int string_len, bool is_progmem){
  if(queue_len >= MAX_OUTPUT_QUEUE_LENGTH){
    Serial.println(F("| OutputQueue::add_element: Max output queue length exceeded! Check your code!"));
  }else if(read_position != 0){
    Serial.println(F("| OutputQueue::add_element: Cannot add elements to an output queue that's partially read!"));
  }else{
    queue[queue_len].pointer = string;
    queue[queue_len].string_length = string_len;
    queue[queue_len].is_progmem = is_progmem;
    total_size += string_len;  //tally up size of referenced strings
    queue_len++;
  }
}


// Clear the queue and the read position at the same time
void OutputQueue::clear_elements(){
  //no need to actually erase the data, just reset list position
  queue_len = 0;
  read_position = 0;
  total_size = 0;
}


//returns false if none are available.
//resets the queue and returns false when there is nothing left to read
bool OutputQueue::get_element(string_element * output){
  if(read_position >= queue_len){
    //nothing to read
    this->clear_elements();
    return false;
  } else {
    *output = queue[read_position];
    read_position++;
  }
  return true;
}


// Returns the sum of the lengths of the enqueued strings
unsigned int OutputQueue::get_total_size(){
  return this->total_size;
}


/*-----------------------------------------------------------------
 * CircularBuffer
 * 
 * Simple ring buffer to handle serial output that we want to scan for string matches.
 * 
 * (see header file for usage)
 *-----------------------------------------------------------------
 */

// Constructor just clears the buffer
CircularBuffer::CircularBuffer(){
  this->buf_reset();
  this->buf_size = SERIAL_INPUT_BUFFER_MAX_SIZE;
}


// Clear the buffer
void CircularBuffer::buf_reset(){
    this->head = 0;
    this->tail = 0;
}


//returns false if we overflowed and lost data.
bool CircularBuffer::buf_put(char data){
  bool rv;
  buf[head] = data;
  head = (head + 1) % buf_size;

  if(head == tail)
  {
      tail = (tail + 1) % buf_size;
      rv = false;
  }
  return rv;
}


int CircularBuffer::buf_put_multiple(char data, unsigned int n){} //todo: implement me


//returns true if the buffer had a value to get
bool CircularBuffer::buf_get(char * data){
    bool r = false;

    if(data && !this->is_empty())
    {
        *data = this->buf[tail];
        tail = (tail + 1) % buf_size;

        r = true;
    }

    return r;
}


//returns the number of values gotten, up to n
int CircularBuffer::buf_get_multiple(char * destination, unsigned int n){}//todo: implement me


//returns true if the buffer's empty
bool CircularBuffer::is_empty(){
  return (head == tail);
}


//returns true if the buffer's full
bool CircularBuffer::is_full(){
  return ((head + 1) % buf_size) == tail;
}

