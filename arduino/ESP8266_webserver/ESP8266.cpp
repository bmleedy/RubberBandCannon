/*
 ESP8266
 
 This class is an ESP8266 for the purposes of my IOT work.  On creation,
 it will set the device to:
  * Only act as a client of an access point, not an access point itself
  * Connect to the house access point
  * Serve a multi-connection TCP server on port 8080
 
 You then interact with it's public methods to use the server:
  * Check_for_requests(bool verbose) - reads one line from the SoftwareSerial
    port and returns TRUE if a line contains a string specified by the caller.
 
*/
#include "ESP8266.h"



ESP8266::ESP8266(SoftwareSerial *port, bool verbose){
    this->port = port;
    serial_input_buffer.reserve(SERIAL_INPUT_BUFFER_MAX_SIZE);
    this->setup_device();
    this->verbose = verbose;
}

//todo: implement a get_line method to put the next line in the input buffer

bool ESP8266::check_for_request(String matchtext){
    char latest_byte = '\0';
    while (this->port->available()) {
        latest_byte = this->port->read();
        if(verbose){Serial.write(latest_byte);}
        //Need to replace this with a proper ring buffer or long lines could crash me. :-O //todo//
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

void ESP8266::send_data(unsigned char channel, String write_data){
    
    // Command the esp to listen for n bytes of data
    String write_command = String(String("AT+CIPSEND=")+//todo: see if I can use the append method
                                  String(channel)+
                                  String(",")+
                                  String(write_data.length())+
                                  String("\r\n"));//todo: figure out if I need to do all the string conversions.
    Serial.println("--------------------------");
    Serial.println(write_data);
    Serial.println("--------------------------");
    Serial.println(write_command);
    Serial.println("--------------------------");
    port->write(write_command.c_str());
    delay(20);
    // Now write the data
    port->write(write_data.c_str());
    delay(20);
    // Now close the connection
    write_command = String(String("AT+CIPCLOSE=")+
                           String(channel)+
                           String("\r\n"));
    // todo: validate that these commands actually worked.  Right now they're open loop.
    port->write(write_command.c_str());
    //https://www.youtube.com/watch?v=ETLDW22zoMA&t=9s
}

void ESP8266::send_http_200(unsigned char channel, String page_data){
    String content = "HTTP/1.1 200 OK\r\n\r\n";
    
    //todo: maybe don't use string (not sure what happens under there) in favor of a fixed-size char[] response buffer.
    
    //todo: add Content_Length header so I don't have to close the connection. Or, maybe I want to close the connection anyway.
    // https://www.w3.org/Protocols/HTTP/Response.html
    
    Serial.println("--------------------------");
    Serial.println(content);
    
    content = String(content + page_data); //content is not propagating here - out of memory //bml - left off here.
    
    Serial.println("--------------------------");
    Serial.println(content);
    this->send_data(channel, content);
}
