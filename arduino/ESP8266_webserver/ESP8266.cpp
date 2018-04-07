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
 ------------------------------------------------------------------------------*/
#include "ESP8266.h"

// Constructor: SoftwareSerial port must be initialized
ESP8266::ESP8266(AltSoftSerial *port, bool verbose){
  this->port = port;
  serial_input_buffer = new CircularBuffer(SERIAL_INPUT_BUFFER_MAX_SIZE);
  this->setup_device();
  this->verbose = verbose;
  this->dump_reads = false;
  this->dump_writes= false;
}

/* read_line() 
 *  Read all data avalable on the serial port.  If I encounter
 *    a '\n'  write the data from the beginning of the input buffer
 *    up to and including the '\n' character and return TRUE.
 *    
 *  If I do not encounter a '\n' return FALSE and do not copy 
 *    anything.
 */

bool ESP8266::read_line(char line_buffer[]){
  //todo: add max length to copy for buffer as an arg.
  char latest_byte = '\0';
  while (this->port->available()) {
    latest_byte = read_port();

    // Add the byte I read to the input buffer
    serial_input_buffer->buf_put(latest_byte);

    // If I just read the end of line char, write out the line,
    //   clearing out that buffer space.
    if(latest_byte == '\n') {
      serial_input_buffer->read_buffer_to_string(line_buffer);
      return true;
    }
  }
  // No \n found.
  return false;
}


/* clear_buffer()
 *  Empty the input ring buffer.
 */
void ESP8266::clear_buffer(){
  serial_input_buffer->buf_reset();
}


/* expect_response_to_command()
 *  Send a command and expect a string in response.
 *  
 *  Used for setting up the ESP8266
 */
bool ESP8266::expect_response_to_command(const char * command, unsigned int command_len,
                                const char * response, unsigned int response_len,
                                unsigned int timeout_ms){
  char input_buffer[100] = "";

  // Write the command
  this->port->write(command, command_len);
  
  // Spin for timeout_ms
  unsigned int start_time = millis();
  char rv = -1;
  int bytes_received = 0;
  while((millis() - start_time) < timeout_ms){
    // Read 1 char off the serial port.
    rv = read_port();
    if (rv != -1) {
      input_buffer[bytes_received] = rv;
      input_buffer[bytes_received+1] = '\0';
      bytes_received++;
      if(strstr(input_buffer,response) != NULL) {
        return true;
      }
    }//if(rv != 1)
  }//while(millis...)
  Serial.println(F("Response timed out"));
  return false;
}


/* setup_device()
 *  
 *  Setup the ESP8266 as a webserver
 *  
 */
bool ESP8266::setup_device(){
    // Get a response from anyone
    Serial.print(F("ESP8266 - Waiting for a response from the Wifi Device..."));
    while(!expect_response_to_command("AT\r\n",4,"OK",2)){
        delay(1000);
    }
    Serial.println(F("[OK]"));
    
    Serial.print(F("ESP8266 - Checking the device CWMODE..."));
    // Set myself up as a client of an access point.
    if(expect_response_to_command("AT+CWMODE?\r\n",12,"+CWMODE:1",9)){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -    Setting the mode to 'client mode'"));
        if(expect_response_to_command("AT+CWMODE=1\r\n",13,"OK",2)){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Now join the house access point
    Serial.print(F("ESP8266 - Checking that we are on the 'leedy' network..."));
    if(expect_response_to_command("AT+CWJAP?\r\n",11,"+CWJAP:\"leedy\"",14)){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -     Not on the 'leedy' network.  Changing the WiFi settings to join network..."));
        if(expect_response_to_command("AT+CWJAP=\"leedy\",\"teamgoat\"\r\n",29,"OK",2,10000)){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Set ourselves up to mux connections into our little server
    Serial.print(F("ESP8266 - Checking the CIPMUX Settings..."));
    if(expect_response_to_command("AT+CIPMUX?\r\n",12,"+CIPMUX:1",9)){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -    Server not enabled yet. Setting CIPMUX=1..."));
        if(expect_response_to_command("AT+CIPMUX=1\r\n",13,"OK",2,10000)){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Now setup the CIP Server
    Serial.print(F("ESP8266 - Configuring my server on port 8080..."));
    if(expect_response_to_command("AT+CIPSERVER=1,8080\r\n",21,"OK",2,10000)){
        Serial.println(F("[OK]"));
    } else {
        Serial.println(F("[FAIL]"));
        return false;
    }
}


/* send_output_queue()
 *  Send the contents of my output queue to a specific channel
 */
void ESP8266::send_output_queue(unsigned char channel){
    const char loc_write_buf_len = 25;
    // Command the esp to listen for n bytes of data
    char write_command_string[loc_write_buf_len];
    snprintf_P((write_command_string),loc_write_buf_len,PSTR("AT+CIPSEND=%d,%d\r\n"),
                                                         channel,
                                                         output_queue.get_total_size());
    write_port(write_command_string,strlen(write_command_string));
    delay(20);

    string_element data_to_write;
    //while we can retrieve things from the output queue
    while(output_queue.get_element(&data_to_write)){
      if(data_to_write.is_progmem){
        for(int i=0;i<data_to_write.string_length;i++){
          char byte_to_write = pgm_read_byte(data_to_write.pointer + i);
          write_port(&byte_to_write, 1);
        }
      }else{
        write_port(data_to_write.pointer,data_to_write.string_length);
      }
    }
    
    // Send the command to terminate the data stream, 
    //   even though it should have been terminated based on length.
    write_port((char *)"+++\r\n",5);

    // Wait 500 ms before sending any other command, per the Espressif ICD, this should be 1s
    delay(500); 
    
    // Now close the connection  
    // todo: this is hacky - just crushes all connections.
    snprintf_P((write_command_string),loc_write_buf_len,PSTR("AT+CIPCLOSE=%d\r\n"),
                                                           channel);
    write_port(write_command_string,strlen(write_command_string));
    // todo: validate that these commands actually worked.  Right now they're open loop.
}


/* send_http_200_static()
 *  Send a static website as an HTTP 200 response.
 */
void ESP8266::send_http_200_static(unsigned char channel, char page_data[], unsigned int page_data_len){

  int total_page_size = 0;

  //todo: add Content_Length header so I don't have to close the connection. 
  //      Or, maybe I want to close the connection anyway.
  // https://www.w3.org/Protocols/HTTP/Response.html

  // start-line per https://tools.ietf.org/html/rfc2616#page-31 
  this->output_queue.add_element((char *)http_200_start_line, HTTP_200_START_LINE_LEN, true);

  //content-length header
  //this->output_queue.add_element(PSTR("Content-Length: 1947\r\n\r\n"),24,true);
  //todo: put in content length header

  // Now enqueue the website page data
  this->output_queue.add_element(page_data, page_data_len,true);
  
  // Send!
  this->send_output_queue(channel);
}

void ESP8266::write_port(char * write_string, unsigned int len){
  //write to port here
  this->port->write(write_string, len);
  if(this->dump_writes)
    Serial.write(write_string,len);
}

char ESP8266::read_port(){
  char rv = this->port->read();
  if(this->dump_reads)
    Serial.write(rv);
  return rv;
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
CircularBuffer::CircularBuffer(int buf_size){
  this->buf = new char[buf_size];
  this->buf_reset();
  this->buf_size = buf_size;
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

void CircularBuffer::read_buffer_to_string(char string[]){
  int iter = 0;
  while(!this->is_empty()){
    this->buf_get(string+iter);
    iter++;
  }
  string[iter]='\0';//todo: this assumes the target is as big as input_max_size plus one.  not safe!
}

