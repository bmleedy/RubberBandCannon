/*!
 * @file ESP8266.cpp
 * 
 * @brief ESP8266 webserver class
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
 */
#include "ESP8266.h"

void print_ok(){
  Serial.println(F("[OK]"));
}

void print_fail(){
  Serial.println(F("[FAIL]"));
}

/*!
 * @brief Constructor: SoftwareSerial port must already be initialized 
 * 
 * @param port     This is a reference to an already-initialized AltSoftSerial port.
 * 
 * @param verbose  If this is set to true, we will spew data received over the serial port.
 * 
 * @param eeprom_address  This is the start location to read configuration info from the eeprom.
 *                        The EEPROM memory block we use looks
 *                        [char initialized][network_info network_info]
 *                         ^eeprom_address   ^eeprom_address + 1
 *                        if "initialized" is set  not exactly equal to 'y', 
 *                           then we load the default settings and write them to
 *                           the EEPROM.
 * 
 * @return         This is a constructor.
 */
ESP8266::ESP8266(AltSoftSerial *port, bool verbose, int eeprom_address){
  this->port = port;  //serial port
  serial_input_buffer = new CircularBuffer(SERIAL_INPUT_BUFFER_MAX_SIZE);
  this->verbose = verbose;
  PRINTSTRLN_IF_VERBOSE("| Dumping all reads and writes to the serial port!");

  this->eeprom_address=eeprom_address;

  if(EEPROM.read(eeprom_address) != 'y'){
    Serial.println(F("| ESP8266: Network settings not initialized - loading defaults"));
    // Set my own values to the defaults
    strcpy_P(station.ssid,PSTR("leedy"));         //default SSID
    strcpy_P(station.password,PSTR("teamgoat"));  //default password
    strcpy_P(station.macaddr, PSTR("dc:4f:22:11:e9:64"));
    strcpy_P(station.ip, PSTR("192.168.1.25"));
    // Save these settings to EEPROM
    update_eeprom();

  } else{
    //Read my settings from EEPROM, starting after the 'y'
    PRINTSTRLN_IF_VERBOSE("| Reading Settings from EEPROM.");
    EEPROM.get( (this->eeprom_address+1),this->station);
  }
  this->server.port = DEFAULT_PORT;
  this->server.maxconns = DEFAULT_MAXCONNS;

  this->setup_device();
}


void ESP8266::update_eeprom(){
  PRINTSTRLN_IF_VERBOSE("| ESP8266: Updating EEPROM");
  EEPROM.put(this->eeprom_address, 'y');
  EEPROM.put((this->eeprom_address+1), station);
}

/*!
 *  Read all data avalable on the serial port.  If I encounter
 *    a '\n'  write the data from the beginning of the input buffer
 *    up to and including the '\n' character and return TRUE.
 *    
 *  If I do not encounter a '\n' return FALSE and do not copy 
 *    anything.
 *  @param line_buffer
 *         pointer to the buffer which we will fill with the line data.
 *         
 *  @return TRUE if a line was read successfully
 */
bool ESP8266::read_line(char line_buffer[], unsigned int line_buffer_size){
  char latest_byte = '\0';
  while (this->port->available()) {
    latest_byte = read_port();

    // Add the byte I read to the input buffer
    serial_input_buffer->buf_put(latest_byte);

    // If I just read the end of line char, write out the line,
    //   clearing out that buffer space.
    if(latest_byte == '\n') {
      serial_input_buffer->read_buffer_to_string(line_buffer, line_buffer_size);
      return true;
    }
  }
  // No \n found.
  return false;
}


/*!
 *  Read one line from the ESP until timeout.
 *  
 *  @param line_buffer
 *         pointer to the buffer which we will fill with the line data.
 *  @param timeout_ms
 *         number of milliseconds to wait until the line has been read.
 *         
 *  @returns TRUE if a line was read successfully
 */
bool ESP8266::read_line(char line_buffer[], unsigned int line_buffer_size, unsigned int timeout_ms){
  unsigned int start_time = millis();

  while( (millis() - start_time) <= timeout_ms ){
    if(read_line(line_buffer, line_buffer_size)){
      return true;
    }
    else{
      delay(1);  //chill for 1 ms
    }
  }
  Serial.println(F("| read_line timeout"));
  return false;
}


/*!
 *  Empty the input ring buffer.
 */
void ESP8266::clear_buffer(){
  serial_input_buffer->buf_reset();
}


/*!
 *  Read all bytes from the serial port, drop them on the floor, and flush the serial input buffer
 *  
 *  
 *  @param timeout
 *         number of milliseconds to run the purge
 */
void ESP8266::purge_serial_input(unsigned int timeout){
  unsigned int start_time = millis();
  while (this->port->available() || ((millis()-start_time) >= timeout)) {
    //read the port and do nothing 
    if(this->port->available())
      read_port();
  }
  clear_buffer();
}

/*!
 *  Send a command and expect a string in response.
 *  
 *  Used for setting up the ESP8266
 *  
 *  @param command
 *         pointer to the string containing an ESP8266 AT command
 *  @param command_len
 *         length of the command string
 *  @param desired_response
 *         String to expect in response to the command
 *  @param response_len
 *         length of the response string
 *  @param timeout_ms
 *         amount of time to wait for the command to respond.
 *         
 *  @return TRUE if we received the expected response
 */
bool ESP8266::expect_response_to_command(const char * command, unsigned int command_len,
                                const char * desired_response,
                                unsigned int timeout_ms){
  char response_line[MAX_RESPONSE_LINE_LEN] = "";

  // Write the command
  this->write_port((char *)command, command_len);
  
  // Spin for timeout_ms
  unsigned int start_time = millis();
  while((millis() - start_time) < timeout_ms){
    if(this->read_line(response_line,MAX_RESPONSE_LINE_LEN)){
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
  Serial.println(F("| expect_response: Timeout"));
  return false;
}


/*!
 *  
 *  Setup the ESP8266 as a webserver
 *  
 *  @return True if setup was successful
 *  
 */
bool ESP8266::setup_device(){
    char request_buffer[COMMAND_BUFFER_SIZE]; 
    char response_buffer[COMMAND_BUFFER_SIZE];
    int setup_stage_delay=1000;
   
    // Get a response from anyone
    Serial.print(F("| ESP8266 - Waiting for a response from the Wifi Device..."));
    while(!expect_response_to_command("AT\r\n",4,"OK",2000)){
        delay(1000);
    }
    print_ok();
    
    Serial.print(F("| ESP8266 - Checking the device CWMODE..."));
    // Set myself up as a client of an access point.
    strncpy_P(request_buffer,PSTR("AT+CWMODE?\r\n"), COMMAND_BUFFER_SIZE);
    strncpy_P(response_buffer,PSTR("+CWMODE:3"),COMMAND_BUFFER_SIZE);
    while(true){
      purge_serial_input(setup_stage_delay);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,2000)){
          print_ok();
          break;
      } else {
        Serial.print (F("\n| ESP8266 -    Setting the mode to 'client mode'"));
        strncpy_P(request_buffer,PSTR("AT+CWMODE_DEF=3\r\n"), COMMAND_BUFFER_SIZE);
        strncpy_P(response_buffer,PSTR("OK"), COMMAND_BUFFER_SIZE);
        if(expect_response_to_command(request_buffer,
                                      strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                      response_buffer,2000)){
            print_ok();
            break;
        }
        else {
            print_fail();
        }
      }
    }

    // configure the cannon AP
    Serial.print(F("| ESP8266 - configuring my own access point..."));
    strncpy_P(request_buffer,PSTR("AT+CWSAP_DEF?\r\n"), COMMAND_BUFFER_SIZE);
    snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("+CWSAP_DEF:\"cannon_ap\",\"%s\",1,3"),"cannon_pass_!@#$");
    while(true){
      purge_serial_input(setup_stage_delay);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,2000)){
          print_ok();
          break;
      } else {
        Serial.print (F("\n| ESP8266 -     Access point not set up.  Setting up now..."));
        snprintf_P(request_buffer,COMMAND_BUFFER_SIZE,PSTR("AT+CWSAP_DEF=\"cannon_ap\",\"%s\",1,3,4,0\r\n"),"cannon_pass_!@#$");
        strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
        if(expect_response_to_command(request_buffer,
                                      strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                      response_buffer,
                                      10000)){
            print_ok();
            break;
        }
        else {
            print_fail();
        }
      }
    }


    
    // configure the cannon AP
    Serial.print(F("| ESP8266 - configuring my ip address on cannon_ap network to 192.168.4.1..."));
    strncpy_P(request_buffer,PSTR("AT+CIPAP_DEF?\r\n"), COMMAND_BUFFER_SIZE);
    snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("+CIPAP_DEF:ip:\"192.168.4.1\""),"192.168.4.1","192.168.4.1");
    while(true){
      purge_serial_input(setup_stage_delay);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,2000)){
          print_ok();
          break;
      } else {
        Serial.print (F("\n| ESP8266 -  ip address on cannon_ap not set up.  Setting up now..."));
        snprintf_P(request_buffer,COMMAND_BUFFER_SIZE,PSTR("AT+CIPAP_DEF=\"%s\",\"%s\",\"255.255.255.0\"\r\n"),"192.168.4.1","192.168.4.1");
        strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
        if(expect_response_to_command(request_buffer,
                                      strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                      response_buffer,
                                      10000)){
            print_ok();
            break;
        }
        else {
            print_fail();
        }
      }
    }

    
    // Now join the house access point
    Serial.print(F("| ESP8266 - Checking that we are on the correct network..."));
    strncpy_P(request_buffer,PSTR("AT+CWJAP?\r\n"), COMMAND_BUFFER_SIZE);
    snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("+CWJAP:\"%s\""),station.ssid);
    while(true){
      purge_serial_input(setup_stage_delay);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,2000)){
          print_ok();
          break;
      } else {
        Serial.print (F("\n| ESP8266 -     Not on the correct network. Fixing the WiFi settings..."));
        snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CWJAP_DEF=\"%s\",\"%s\"\r\n"),station.ssid,station.password);
        strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
        if(expect_response_to_command(request_buffer,
                                      strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                      response_buffer,
                                      10000)){
            print_ok();
            break;
        }
        else {
            print_fail();
        }
      }
    }
/*
    // Now setup the CIP Server
    Serial.print(F("ESP8266 - Setting server maxconns..."));
    snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CIPSERVERMAXCONN=%d\r\n"),server.maxconns);
    strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
    while(true){
      purge_serial_input(setup_stage_delay);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,
                                    10000)){
        print_ok();
        break;
      } else {
        print_fail();
      }
    }
  */  
    // Set ourselves up to mux connections into our little server
    Serial.print(F("| ESP8266 - Checking the CIPMUX Settings..."));
    strncpy_P(request_buffer,PSTR("AT+CIPMUX?\r\n"), COMMAND_BUFFER_SIZE);
    strncpy_P(response_buffer,PSTR("+CIPMUX:1"),COMMAND_BUFFER_SIZE);
    while(true){
      purge_serial_input(setup_stage_delay);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,2000)){
          print_ok();
          break;
      } else {
          Serial.print(F("\n| ESP8266 -    Server not enabled yet. Setting CIPMUX=1..."));
          strncpy_P(request_buffer,PSTR("AT+CIPMUX=1\r\n"), COMMAND_BUFFER_SIZE);
          strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
          if(expect_response_to_command(request_buffer,
                                        strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                        response_buffer,
                                        10000)){
              print_ok();
              break;
          }
          else {
              print_fail();
          }
      }
    }
    
    // Now setup the CIP Server
    Serial.print(F("| ESP8266 - Configuring my server on port 8080..."));
    snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CIPSERVER=1,%d\r\n"),server.port);
    strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
    while(true){
      purge_serial_input(setup_stage_delay);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,
                                    10000)){
          print_ok();
          break;
      } else {
          print_fail();
          return false;
      }
    }
    
  return true;
}


/*!
 *  
 *  Send the contents of my output queue to a specific channel
 *  
 *  @param channel
 *         The channel to which this send will be transmitted.
 */
void ESP8266::send_output_queue(unsigned char channel){
    // Command the esp to listen for n bytes of data
    char write_command_string[COMMAND_BUFFER_SIZE];
    char response_buffer[COMMAND_BUFFER_SIZE];
    
    // Put the ESP int send mode
    snprintf_P(write_command_string, COMMAND_BUFFER_SIZE, 
                                     PSTR("AT+CIPSEND=%d,%d\r\n"),
                                     channel,
                                     output_queue.get_total_size());
    write_port(write_command_string,strnlen(write_command_string,COMMAND_BUFFER_SIZE));

    // Wait a bit to let it get ready
    delay(20);

    // Write all of the elements in my output queue
    string_element data_to_write;
    //while we can retrieve things from the output queue
    while(output_queue.get_element(&data_to_write)){
      PRINTSTR_IF_VERBOSE("| Outputting queue element of size: ");
      PRINTLN_IF_VERBOSE(data_to_write.string_length);
      if(data_to_write.is_progmem){
        for(unsigned int i=0;i<data_to_write.string_length;i++){
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
    strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
    if(expect_response_to_command(write_command_string,
                                  strnlen(write_command_string,COMMAND_BUFFER_SIZE),
                                  response_buffer,1000)){
        //Closed successfully - no action needed
    } else {
        //Print a warning - if this happens often, potentially put this into a loop to make sure we close channels
        Serial.print(F("| WARNING Failed to close connection to the ESP8266 on channel ")); Serial.println(channel,DEC);
    }
}


/*!
 *  Send a static website as an HTTP 200 response.
 *  
 *  @param channel
 *         The channel on which we send this response
 *  @param page_data
 *         A pointer to the page data to be transmitted
 *  @param page_data_len
 *         Number of characters in the page data to be transmitted
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


/*!
 *  Sends an http 200 response, populating a javascript map variable 
 *    with data fetched about the device's configuration.  This was 
 *    created for the config page, which should display the current settings
 *    when you load the page.
 *    
 *    @param channel
 *           the channel to which we will send this request
 *    @param page_data_0
 *           pointer to the first page data element to write (before the dynamic data)
 *    @param page_data_0_len
 *           length of the element above
 *    @param page_data_2
 *           pointer to the first page data element to write (after the dynamic data)
 *    @param page_data_2_len
 *           length of the element above
 *    @param prefetch_data_fields
 *           this is a list of 7-character ID's
 *    @param num_prefetch_data_fields
 *           the number of 7-character fields to retrieve
 */
void ESP8266::send_http_200_with_prefetch(unsigned char channel,
                                          char page_data_0[], unsigned int page_data_0_len,
                                          char page_data_2[], unsigned int page_data_2_len,
                                          const char* const prefetch_data_fields[], unsigned int num_prefetch_data_fields){

  // start-line per https://tools.ietf.org/html/rfc2616#page-31 
  this->output_queue.add_element((char *)http_200_start_line, HTTP_200_START_LINE_LEN, true);

  // Now enqueue the first section of website page data (in progmem)
  this->output_queue.add_element(page_data_0, page_data_0_len,true);

  // Add each field to a prefetch buffer, that I'll put in the output queue
  strncpy_P(prefetch_output_buffer,PSTR("//begin prefetched data\n"),PREFETCH_OUTPUT_BUFFER_SIZE);

  // For each prefetch data field, find the matching data and add it
  //   to the buffer string.
  prefetch_output_buffer_len = 0; //clear the output buffer
  bool have_queried_mac = false;  //only queried to get mac and IP
  int buffer_size_remaining;
  
  for(unsigned int i=0; i<=num_prefetch_data_fields; i++){

    // check
    buffer_size_remaining = PREFETCH_OUTPUT_BUFFER_SIZE - prefetch_output_buffer_len;
    if(buffer_size_remaining < 20){
      Serial.println(F("| WARNING: prefetch output buffer is running out of space!"));
      Serial.println(PREFETCH_OUTPUT_BUFFER_SIZE);
      Serial.println(prefetch_output_buffer_len);
    }

    char prefetch_field_name[10];

    //ref: https://www.arduino.cc/reference/en/language/variables/utilities/progmem/
    strncpy_P(prefetch_field_name, (char*)pgm_read_word(&(prefetch_data_fields[i])), 7);
    prefetch_field_name[7] = '\0';

    if(strstr_P(prefetch_field_name, PSTR("ssid__"))){
//      //this->query_network_ssid();
      snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len),
                  buffer_size_remaining,
                  PSTR("%s:\"%s\","),
                  prefetch_field_name,
                  station.ssid);
      prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
    } else if(strstr_P(prefetch_field_name, PSTR("conctd"))){
      if(this->is_network_connected()){
        strncpy_P( (prefetch_output_buffer+prefetch_output_buffer_len),
                    PSTR("conctd:true,"),
                    buffer_size_remaining);
      } else {
        strncpy_P( (prefetch_output_buffer+prefetch_output_buffer_len),
                   PSTR("conctd:false,"), 
                   buffer_size_remaining);
      }
      prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
    }else if( (strstr_P(prefetch_field_name, PSTR("ipaddr")) ||
              strstr_P(prefetch_field_name,  PSTR("macadr")))){
      if(!have_queried_mac){
        //this->query_ip_and_mac();//todo: re-enable
        have_queried_mac = true; //only do one query to refresh IP and mac
        snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len), 
                    buffer_size_remaining,
                    PSTR("ipaddr:\"%s\","),station.ip);
        prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
        snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len),
                    buffer_size_remaining,
                    PSTR("macadr:\"%s\","),station.macaddr);
        prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
      }
    }else if(strstr_P(prefetch_field_name, PSTR("port__"))){
      Serial.println(F("| writing port"));
      snprintf_P( (prefetch_output_buffer+prefetch_output_buffer_len), 
                 buffer_size_remaining,
                 PSTR("port__:%d"),server.port);
      prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
    }else{
      Serial.print(F("| Prefetch field not found: "));Serial.write(prefetch_field_name,7);Serial.println("");
    } 
  }//for(prefetch_data_fields)
  
  //add the prefetch output buffer to the output queue
  this->output_queue.add_element(prefetch_output_buffer, prefetch_output_buffer_len, false);

  // Add the last chunk of website page data
  this->output_queue.add_element(page_data_2, page_data_2_len,true);
  
  // Send!
  this->send_output_queue(channel);
}


/*!
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
    if(this->read_line(input_buffer,MAX_RESPONSE_LINE_LEN)){
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


/*!
 * Make a call to the ESP8266 serial port and check that it responds happily
 * 
 * @return TRUE if the network is connected
 */
bool ESP8266::is_network_connected(){
  char request_buffer[COMMAND_BUFFER_SIZE];
  char response_buffer[COMMAND_BUFFER_SIZE];
  strcpy_P(request_buffer, PSTR("AT+CWJAP?\r\n"));
  sprintf_P(response_buffer,PSTR("+CWJAP:\"%s\""),station.ssid);
  return expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,2000);
}


/*!
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
    if(this->read_line(input_buffer, MAX_RESPONSE_LINE_LEN)){
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

/*!
 * Simply writes to the ESP serial port, with logging if needed.
 * 
 * @param write_string
 *        a char array to write out to the ESP serial port
 *        
 * @param len
 *        how many characters to write 
 */
void ESP8266::write_port(char * write_string, unsigned int len){
  //write to port here
  this->port->write(write_string, len);
  WRITE_IF_VERBOSE(write_string,len);
}

/*!
 * Simply reads from the ESP serial port, with logging if needed.
 * 
 * @return a char from the serial port;
 */
char ESP8266::read_port(){
  char rv = this->port->read();
  WRITE_IF_VERBOSE(&rv,1);
  return rv;
}


/*!
 * Sends the command to set the SSID to the ESP8266 and returns true if it succeeds.
 * 
 * @param new_ssid
 *        c-string array containing the new SSID
 *     
 * @return TRUE if we successfully set the SSID we connect to.
 */
bool ESP8266::set_station_ssid_and_passwd(char new_ssid_and_passwd[]){

  char command_to_send[(MAX_SSID_LENGTH+MAX_PASSWORD_LENGTH+18)];
  char desired_response[] = "OK";
  unsigned int max_attempts = 3;

  Serial.print(F("| Setting new ssid: ["));Serial.write(new_ssid_and_passwd,strlen(new_ssid_and_passwd));Serial.println("]");

  snprintf_P(command_to_send,
             (MAX_SSID_LENGTH+MAX_PASSWORD_LENGTH+18), 
             PSTR("AT+CWJAP_DEF=%s\r\n"), 
             new_ssid_and_passwd);
  
  for(unsigned int i=0; i<max_attempts; i++){
    if(expect_response_to_command(command_to_send, 
                                     strlen(command_to_send),
                                     desired_response,10000)){
      //update my class variables
      char* read_pointer = strtok(new_ssid_and_passwd,",\"");
      strncpy(this->station.ssid,read_pointer,MAX_PASSWORD_LENGTH+1);
      read_pointer = strtok(NULL,",\"");
      strncpy(this->station.password,read_pointer,MAX_SSID_LENGTH+1);

      //Store the new settings in eeprom
      update_eeprom();
      
      //return success
      return true;
    }else{
      Serial.print(F("| Attempt "));Serial.print(i,DEC);Serial.println(F(" to set SSID failed."));
    }
  }

  return false; //we timed out and did not succeed.
}




/*!
 * Reads the list of networks that the ESP can see and writes it back 
 * in an HTTP response to the connection at the channel param.
 * 
 * @param channel
 *        The channel to which I should send the response.
 * 
 */
void ESP8266::send_networks_list(unsigned char channel){
  char command_to_write[COMMAND_BUFFER_SIZE];
  char response_line[MAX_RESPONSE_LINE_LEN];
  
  //Setup the access point list settings
  strncpy_P(command_to_write,PSTR("AT+CWLAPOPT=0,2\r\n"),COMMAND_BUFFER_SIZE);
  write_port(command_to_write,strnlen(command_to_write,COMMAND_BUFFER_SIZE));
  
  //Request the access point list
  strncpy_P(command_to_write,PSTR("AT+CWLAP\r\n"), COMMAND_BUFFER_SIZE);
  write_port(command_to_write,strnlen(command_to_write,COMMAND_BUFFER_SIZE));

  //now, parse through till i see the my response
  bool response_started = false;
  unsigned int buffer_index = 0;
  int buffer_size_remaining = PREFETCH_OUTPUT_BUFFER_SIZE;
  while(true){
    
    //Check my space remaining
    buffer_size_remaining = PREFETCH_OUTPUT_BUFFER_SIZE - prefetch_output_buffer_len;
    if(buffer_size_remaining < 20){
      Serial.println(F("| WARNING: prefetch output buffer is running out of space!"));
    }

    //Read a line and queue it up to send
    read_line(response_line, 2000);
    if(strstr_P(response_line,PSTR("+CWLAP")) != NULL){
      response_started = true;
      snprintf_P(prefetch_output_buffer+buffer_index,
                 buffer_size_remaining,
                 PSTR("%s\n"),
                 response_line);
      prefetch_output_buffer_len = strnlen(prefetch_output_buffer,PREFETCH_OUTPUT_BUFFER_SIZE);
    } else if(response_started && strstr_P(response_line,PSTR("OK"))){
      send_http_200_static(channel,prefetch_output_buffer,prefetch_output_buffer_len);
    }
  }//while()
}


/*!
 * @brief Gets the channel number from an ESP "IPD" line
 * 
 * Reads the input line with length len and returns the channel found, 
 * or zero if none is found.
 * 
 * @param line
 *        a pointer to the array containing a line from the ESP
 *        
 * @param len
 *        the number of characters in the line, including the '\n'
 *        
 * @return the channel found, or zero if one is not found
 */
char ESP8266::get_channel_from_string(char line[], int len){
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
  this->current_channel=rv;
  return rv;
  
}






/*!
 * @fn void process_settings(unsigned char channel, char input_line[])
 * 
 * @brief processes the settings command for the ESP
 * 
 * @param channel
 *        This is the channel on which the request for settings was transmitted.
 *        Send the response back on the same channel.
 *        
 * @param input_line[]
 *        This is the last line read from the ESP8266, which contains the path to
 *        the setting that we want to change.
 */
void ESP8266::process_settings(unsigned char channel, char input_line[],int input_line_size) {
  char* read_pointer = NULL;

  // Read the remaining lines, until I find my parameter, or I time out:
  if(strstr_P(input_line,PSTR("ssid__")) != NULL){
    Serial.println(F("| received an SSID setting request"));
    do{
      if(strstr_P(input_line,PSTR("ssid__=")) != NULL){
        //found the setting string
        read_pointer = strtok(input_line,"="); //up to the start of the SSID
        read_pointer = strtok(NULL,"="); //the SSID field
        read_pointer = strtok(read_pointer,"\n"); //trimming the trailing newline
        if(set_station_ssid_and_passwd(read_pointer)){
          //No point in sending a response - SSID change will break the connection.
          Serial.println(F("| Set Station SSID succeeded!"));
          break;
        } else {
          send_http_200_static(channel,(char *)failure_msg,(sizeof(failure_msg)-1));
        }
      }//if(ssid)
    }while(read_line(input_line,input_line_size, 10000));//while(read_line)
  } else {
    Serial.println(F("| received an unknown setting path."));
  }
}



