#ifndef ESP8266_H
#define ESP8266_H
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
#include <SoftwareSerial.h>

#include <Arduino.h>
// include types & constants of Wiring core API
//#include "WConstants.h"
// include Wiring Program language
//#include "WProgram.h"
// include Serial as well
#include "HardwareSerial.h"

#define SERIAL_INPUT_BUFFER_MAX_SIZE 500

class ESP8266{
private:
    SoftwareSerial *port;
    String serial_input_buffer;  //todo replace with char ring buffer
    bool verbose;
    
public:
    ESP8266(SoftwareSerial *port, bool verbose);
    bool check_for_request(String matchtext);
    void send_http_200(unsigned char channel,String page_data);
    
    
private:
    bool expect_response_to_command(String command,
                                    String response,
                                    unsigned int timeout_ms=2000);
    bool print_response_to_command(String command,
                                   unsigned int timeout_ms=2000);
    bool setup_device();
    void send_data(unsigned char channel, String data);
};



#endif
