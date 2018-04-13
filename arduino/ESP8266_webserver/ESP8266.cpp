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

/* ESP8266()
 * Constructor: SoftwareSerial port must already be initialized 
 */
ESP8266::ESP8266(AltSoftSerial *port, bool verbose){
  this->port = port;  //serial port
  serial_input_buffer = new CircularBuffer(SERIAL_INPUT_BUFFER_MAX_SIZE);
  this->verbose = verbose;
  this->dump_reads = this->verbose;
  this->dump_writes= this->verbose;
  //todo: load the following values from eeprom instead of hard-coded defaults
  sprintf_P(station.ssid,PSTR("leedy"));         //default SSID
  sprintf_P(station.password,PSTR("teamgoat"));  //default password
  this->server.port = DEFAULT_PORT;
  this->server.maxconns = DEFAULT_MAXCONNS;

  this->setup_device();
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
                                const char * desired_response, unsigned int response_len,
                                unsigned int timeout_ms){
  char response_line[MAX_RESPONSE_LINE_LEN] = "";

  // Write the command
  this->port->write(command, command_len);
  
  // Spin for timeout_ms
  unsigned int start_time = millis();
  while((millis() - start_time) < timeout_ms){
    if(this->read_line(response_line)){
      // a line is found
      if(strstr(response_line,desired_response) != NULL) {
        //line has response string, return success
        return true;
      } else {
        // line does not have response string
        // continue reading more lines
      }
    }//if(read_line)
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
    char request_buffer[COMMAND_BUFFER_SIZE]; 
    char response_buffer[COMMAND_BUFFER_SIZE];
  
    // Get a response from anyone
    Serial.print(F("ESP8266 - Waiting for a response from the Wifi Device..."));
    while(!expect_response_to_command("AT\r\n",4,"OK",2)){
        delay(1000);
    }
    Serial.println(F("[OK]"));
    
    Serial.print(F("ESP8266 - Checking the device CWMODE..."));
    // Set myself up as a client of an access point.
    snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CWMODE?\r\n"));
    snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("+CWMODE:1"));
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,
                                  strnlen(response_buffer,COMMAND_BUFFER_SIZE))){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -    Setting the mode to 'client mode'"));
        snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CWMODE=1\r\n"));
        snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("OK"));
        if(expect_response_to_command(request_buffer,
                                      strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                      response_buffer,
                                      strnlen(response_buffer,COMMAND_BUFFER_SIZE))){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Now join the house access point
    Serial.print(F("ESP8266 - Checking that we are on the correct network..."));
    snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CWJAP?\r\n"));
    snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("+CWJAP:\"%s\""),station.ssid);
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,
                                  strnlen(response_buffer,COMMAND_BUFFER_SIZE))){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -     Not on the correct network.  Changing the WiFi settings to join network..."));
        snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CWJAP=\"%s\",\"%s\"\r\n"),station.ssid,station.password);
        snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("OK"));
        if(expect_response_to_command(request_buffer,
                                      strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                      response_buffer,
                                      strnlen(response_buffer,COMMAND_BUFFER_SIZE),
                                      10000)){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }
    
    // Set ourselves up to mux connections into our little server
    Serial.print(F("ESP8266 - Checking the CIPMUX Settings..."));
    snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CIPMUX?\r\n"));
    snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("+CIPMUX:1"));
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,
                                  strnlen(response_buffer,COMMAND_BUFFER_SIZE))){
        Serial.println(F("[OK]"));
    } else {
        Serial.print (F("\nESP8266 -    Server not enabled yet. Setting CIPMUX=1..."));
        snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CIPMUX=1\r\n"));
        snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("OK"));
        if(expect_response_to_command(request_buffer,
                                      strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                      response_buffer,
                                      strnlen(response_buffer,COMMAND_BUFFER_SIZE),
                                      10000)){
            Serial.println(F("[OK]"));
        }
        else {
            Serial.println(F("[FAIL]"));
            return false;
        }
    }

    // Now setup the CIP Server
    Serial.print(F("ESP8266 - Setting server maxconns..."));
    snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CIPSERVERMAXCONN=%d\r\n"),server.maxconns);
    snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("OK"));
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,
                                  strnlen(response_buffer,COMMAND_BUFFER_SIZE),
                                  10000)){
        Serial.println(F("[OK]"));
    } else {
        Serial.println(F("[FAIL]"));
        return false;
    }
    
    // Now setup the CIP Server
    Serial.print(F("ESP8266 - Configuring my server on port 8080..."));
    snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CIPSERVER=1,%d\r\n"),server.port);
    snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("OK"));
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,
                                  strnlen(response_buffer,COMMAND_BUFFER_SIZE),
                                  10000)){
        Serial.println(F("[OK]"));
    } else {
        Serial.println(F("[FAIL]"));
        return false;
    }
}


/* send_output_queue()
 *  
 *  Send the contents of my output queue to a specific channel
 *  
 */
void ESP8266::send_output_queue(unsigned char channel){
    // Command the esp to listen for n bytes of data
    char write_command_string[COMMAND_BUFFER_SIZE];
    char response_buffer[COMMAND_BUFFER_SIZE];
    
    // Put the ESP int send mode
    snprintf_P(write_command_string, COMMAND_BUFFER_SIZE, 
                                     PSTR("AT+CIPSEND=%d,%d\r\n"),
                                     channel);
    write_port(write_command_string,strnlen(write_command_string,COMMAND_BUFFER_SIZE));

    // Wait a bit to let it get ready
    delay(20);

    // Write all of the elements in my output queue
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
    //  even though it should have been terminated based on length.
    write_port((char *)"++++++\r\n",8);

    // Wait 500 ms before sending any other command, per the Espressif ICD, this should be 1s
    delay(500); 
    
    // Now close the connection, printing a warning if I don't see an "OK" in response
    snprintf_P(write_command_string, COMMAND_BUFFER_SIZE, PSTR("AT+CIPCLOSE=%d\r\n"),channel);
    snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("OK"));
    if(expect_response_to_command(write_command_string,
                                  strnlen(write_command_string,COMMAND_BUFFER_SIZE),
                                  response_buffer,
                                  strnlen(response_buffer,COMMAND_BUFFER_SIZE))){
        //Closed successfully - no action needed
    } else {
        //Print a warning - if this happens often, potentially put this into a loop to make sure we close channels
        Serial.print(F("| WARNING Failed to close connection to the ESP8266 on channel ")); Serial.println(channel,DEC);
    }
}


/* send_http_200_static()
 *  
 *  Send a static website as an HTTP 200 response.
 *  
 */
void ESP8266::send_http_200_static(unsigned char channel, char page_data[], unsigned int page_data_len){

  // Add start-line per https://tools.ietf.org/html/rfc2616#page-31 
  this->output_queue.add_element((char *)http_200_start_line, HTTP_200_START_LINE_LEN, true);

  //todo: add Content_Length header so I don't have to close the connection forcibly

  // Now enqueue the website page data, which is stored in progmem
  this->output_queue.add_element(page_data, page_data_len,true);
  
  // Send!
  this->send_output_queue(channel);
}

/* send_http_200_with_prefetch()
 *  
 *  Sends an http 200 response, populating a javascript map variable 
 *    with data fetched about the device's configuration.  This was 
 *    created for the config page, which should display the current settings
 *    when you load the page.
 *  
 */
void ESP8266::send_http_200_with_prefetch(unsigned char channel,
                                          char page_data_0[], unsigned int page_data_0_len,
                                          char page_data_2[], unsigned int page_data_2_len,
                                          const char prefetch_data_fields[][7], unsigned int num_prefetch_data_fields){

  // start-line per https://tools.ietf.org/html/rfc2616#page-31 
  this->output_queue.add_element((char *)http_200_start_line, HTTP_200_START_LINE_LEN, true);

  // todo: add content-length header

  // Now enqueue the first section of website page data (in progmem)
  this->output_queue.add_element(page_data_0, page_data_0_len,true);

  // Add each field to a prefetch buffer, that I'll put in the output queue
  strncpy_P(prefetch_output_buffer,PSTR("//begin prefetched data\n"),PREFETCH_OUTPUT_BUFFER_SIZE);

  // For each prefetch data field, find the matching data and add it
  //   to the buffer string.
  prefetch_output_buffer_len = 0; //clear the output buffer
  bool have_queried_mac = false;  //only queried to get mac and IP
  int buffer_size_remaining = PREFETCH_OUTPUT_BUFFER_SIZE;
  for(int i=0; i<num_prefetch_data_fields; i++){

    // check
    buffer_size_remaining = PREFETCH_OUTPUT_BUFFER_SIZE - prefetch_output_buffer_len;
    if(buffer_size_remaining < 20){
      Serial.println(F("| WARNING: prefetch output buffer is running out of space!"));
    }
    
    if(strstr_P(prefetch_data_fields[i], PSTR("ssid__"))){
      this->query_network_ssid();
      snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len),
                  buffer_size_remaining,
                  "%s:%s,\n",
                  prefetch_data_fields[i],
                  station.ssid);
      prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
    } else if(strstr_P(prefetch_data_fields[i], PSTR("conctd"))){
      if(this->is_network_connected()){
        snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len),
                    buffer_size_remaining,
                    PSTR("conctd:true,\n"));
      } else {
        snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len), 
                   buffer_size_remaining,
                   PSTR("conctd:false,\n"));
      }
      prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
    }else if( (strstr_P(prefetch_data_fields[i], PSTR("ipaddr")) ||
              strstr_P(prefetch_data_fields[i],  PSTR("macadr"))) && !have_queried_mac){
      this->query_ip_and_mac();
      have_queried_mac = true; //only do one query to refresh IP and mac
      //todo: make this an add_string_to_buffer method
      snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len), 
                  buffer_size_remaining,
                  PSTR("ipaddr:%s,\n"),station.ip);
      prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
      snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len),
                  buffer_size_remaining,
                  PSTR("macaddr:%s,\n"),station.macaddr);
      prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
    }else if(strstr_P(prefetch_data_fields[i], PSTR("port__"))){
      //todo: make this an add_int_to_buffer method
      snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len), 
                 buffer_size_remaining,
                 PSTR("port__:%d,\n"),server.port);
      prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
    }else{
      Serial.print(F("Prefetch field not found: "));Serial.write(prefetch_data_fields[i],7);
    } 
  }//for(prefetch_data_fields)
  this->output_queue.add_element(prefetch_output_buffer, prefetch_output_buffer_len, false);

  // Add the last chunk of website page data
  this->output_queue.add_element(page_data_2, page_data_2_len,true);
  
  // Send!
  this->send_output_queue(channel);
}


/* query_network_ssid()
 *  
 *  Make a call to the ESP8266 to get my own SSID and update my internal
 *    class variables with the values I read.
 * 
 */
void ESP8266::query_network_ssid(){
  char input_buffer[MAX_RESPONSE_LINE_LEN] = "";
  unsigned int timeout_ms = 10000;
  char * read_pointer = NULL;

  // Write the command
  this->port->print(F("AT+CWJAP_CUR?\r\n"));

  // Read lines from the ESP8266 until we time out or succeed
  unsigned int start_time = millis();
  while((millis() - start_time) < timeout_ms){
    if(this->read_line(input_buffer)){
      // a line is found
      if(strstr_P(input_buffer,PSTR("+CWJAP")) != NULL) {
          //Response is on this line, parse out the strings
          read_pointer = strtok(input_buffer,"\""); //up to the start of the SSID
          read_pointer = strtok(NULL,"\""); //the SSID field
          strncpy(station.ssid, read_pointer, MAX_SSID_LENGTH); //copy into my SSID string
          return; //done.  return before I time out.
      } else {
        // line does not have response string
        // continue reading more lines
      }
    }//if(read_line)
  }//while(millis...)
  
  Serial.println(F("Response timed out"));
}

/* is_network_connected()
 * 
 * Make a call to the ESP8266 serial port and check that it responds happily
 * 
 */
bool ESP8266::is_network_connected(){
  char request_buffer[COMMAND_BUFFER_SIZE];
  char response_buffer[COMMAND_BUFFER_SIZE];
  sprintf_P(request_buffer, PSTR("AT+CWJAP?\r\n"));
  sprintf_P(response_buffer,PSTR("+CWJAP:\"%s\""),station.ssid);
  return expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,
                                  strnlen(response_buffer,COMMAND_BUFFER_SIZE));
}

/* query_ip_and_mac()
 *  
 *  Make a call to the ESP8266 serial port to get the current station
 *    IP address and MAC address.  Save these in the class variables
 *    as my current station ip and MAC.
 *  
 */

void ESP8266::query_ip_and_mac(){
  char input_buffer[MAX_RESPONSE_LINE_LEN] = "";
  unsigned int timeout_ms = 2000;
  char * read_pointer = NULL;

  // Write the command
  this->port->print(F("AT+CIFSR\r\n"));

  unsigned int start_time = millis();
  while((millis() - start_time) < timeout_ms){
    if(this->read_line(input_buffer)){
      // a line is found
      if(strstr_P(input_buffer,PSTR("+CWJAP")) != NULL) {
        if(strstr_P(input_buffer,PSTR("STAIP")) != NULL){
          //IP Address is on this line, parse out the strings
          read_pointer = strtok(input_buffer,"\""); //up to the start of the IP Address
          read_pointer = strtok(NULL,"\""); //the SSID field
          strncpy(station.ip, read_pointer, IP_ADDRESS_LENGTH); //copy into my ip string
        } else if(strstr_P(input_buffer,PSTR("STAMAC")) != NULL){
                    //Response is on this line, parse out the strings
          read_pointer = strtok(input_buffer,"\""); //up to the start of the SSID
          read_pointer = strtok(NULL,"\""); //the SSID field
          strncpy(station.macaddr, read_pointer, MAC_ADDRESS_LENGTH); //copy into my macaddr string
          return; //done.  return before I time out. (this value should come last
        } 
      } else {
        // line does not have response string
        // continue reading more lines
      }
    }
  }
  Serial.println(F("| IP and mac Response timed out"));
}

/* write_port()
 * 
 * Simply writes to the ESP serial port, with logging if needed.
 * 
 */
void ESP8266::write_port(char * write_string, unsigned int len){
  //write to port here
  this->port->write(write_string, len);
  if(this->dump_writes)
    Serial.write(write_string,len);
}

/* read_port()
 * 
 * Simply reads from the ESP serial port, with logging if needed.
 * 
 */
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

