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
#include "HardwareSerial.h"

//
#define SERIAL_INPUT_BUFFER_MAX_SIZE 200
#define MAX_OUTPUT_QUEUE_LENGTH  10  //number of pointers to strings I'll store

// pointers and lengths for static or progmem strings to be sent
struct string_element{
  char * pointer;         //this should be ptr to a static char. Dynamic data may mutate before use.
  unsigned int string_length;
};

// 
class OutputQueue{
  private:
  string_element queue[MAX_OUTPUT_QUEUE_LENGTH];
  unsigned char queue_len;
  int read_position;
  unsigned int total_size;

  public:
  OutputQueue();
  void add_element(char * string, unsigned char string_len);
  //reset queue position and length. Automatic when you get the last element.
  void clear_elements();
  //gets the element
  bool get_element(string_element * output);//returns false if none are available.
  unsigned int get_total_size();
};



class ESP8266{
private:
    SoftwareSerial *port;
    String serial_input_buffer;  //todo replace with char ring buffer
    bool verbose;
    OutputQueue output_queue;
    
public:
    ESP8266(SoftwareSerial *port, bool verbose);
    bool check_for_request(String matchtext);
    void send_http_200(unsigned char channel,String page_data);//deprecated
    void send_http_200_static(unsigned char channel,char page_data[],unsigned int page_data_len);
    
    
private:
    bool expect_response_to_command(String command,
                                    String response,
                                    unsigned int timeout_ms=2000);
    bool print_response_to_command(String command,
                                   unsigned int timeout_ms=2000);
    bool setup_device();
    void send_data(unsigned char channel, String data);//deprecated
    void send_output_queue(unsigned char channel);
};



#endif
