#ifndef ESP8266_H
#define ESP8266_H

#include <SoftwareSerial.h>
#include <Arduino.h>
#include "HardwareSerial.h"


#define SERIAL_INPUT_BUFFER_MAX_SIZE 200  //maximum line length that I can handle
#define MAX_OUTPUT_QUEUE_LENGTH  20       //number of pointers to strings I'll store


/*-----------------------------------------------------------------
 * OutputQueue
 * 
 * Holds pointers to strings, their sizes, and a tally of the sizes
 * of all of the strings that need to be outputted.
 * 
 * This is a huge space saver compared to keeping an output buffer 
 * in dynamic memory, where it may topple your heap.  It's only 
 * dynamic memory usage is the compact array of pointers to strings 
 * and string lengths.
 * 
 * //todo: rather than an array, a linked list could be prepended,
 *        which would be nice.
 *        
 * Usage:
 *    char string1[10] = "1234567890";
 *    char string2[3]  = "321";
 *    OutputQueue myqueue;
 *    myqueue.add_element(string1,sizeof(string1);
 *    myqueue.add_element(string2,sizeof(string2);
 *    //...etc, etc, up to max number of elements.
 *    string_element output;
 *    while(myqueue.get_element(&output)){
 *      my_method_to_use_the_output_strings(output);
 *    }
 *-----------------------------------------------------------------
 */
// Datatype for the output queue
struct string_element{
  char * pointer;         //this should be ptr to a static char. Dynamic data may mutate before use.
  unsigned int string_length;
};


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


/*---------------------------------------------------------------
 * ESP8266
 *
 * This class is an ESP8266 for the purposes of my IOT work.  On creation,
 * it will set the device to:
 *  * Only act as a client of an access point, not an access point itself
 *  * Connect to the house access point
 *  * Serve a multi-connection TCP server on port 8080
 *
 * USAGE:
 *   ESP8266 * myesp;
 *   myesp = new ESP8266(&serial_port, verbose_flag);
 *   while(1){
 *     if(myesp->check_for_request(F("GET"))){
 *       Serial.println(F("got a request!!!"));
 *       myesp->send_http_200_static(0,(char*)static_website_text,
 *                                   sizeof(static_website_text));
 *}
 ---------------------------------------------------------------*/

class ESP8266{
private:
    SoftwareSerial *port;       //Initialized outside of this class
    String serial_input_buffer; //todo: Replace with char ring buffer
    bool verbose;               
    OutputQueue output_queue;   //Does not hold data, just pointers to data
    
public:
    ESP8266(SoftwareSerial *port, bool verbose);
    bool check_for_request(String matchtext);
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

/*-----------------------------------------------------------------
 * CircularBuffer
 * 
 * Simple ring buffer to handle serial output that we want to scan for string matches.
 * 
 *-----------------------------------------------------------------
 */

class CircularBuffer{
private:
  char buf[SERIAL_INPUT_BUFFER_MAX_SIZE];
  unsigned int head;
  unsigned int tail;
  unsigned int buf_size; //of the buffer

  CircularBuffer();
  
public:
  void buf_reset();
  bool buf_put(char data);
  int buf_put_multiple(char data, unsigned int n);//not yet implemented
  bool buf_get(char * data);
  int buf_get_multiple(char * destination, unsigned int n);//not yet implemented
  bool is_empty();
  bool is_full();
  
};

#endif
