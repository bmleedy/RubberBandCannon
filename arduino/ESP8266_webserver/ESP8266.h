#ifndef ESP8266_H
#define ESP8266_H

//#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <Arduino.h>
#include "HardwareSerial.h"
#include "webserver_constants.h"

#define SERIAL_INPUT_BUFFER_MAX_SIZE 300  //maximum line length that I can handle
#define PREFETCH_OUTPUT_BUFFER_SIZE  100
#define MAX_RESPONSE_LINE_LEN 50          //the longest single line we expect in a response, including "\r\n\0"
#define MAX_OUTPUT_QUEUE_LENGTH  20       //number of pointers to strings I'll store
#define MAX_SSID_LENGTH 32  //number of characters, not counting string null terminator
#define MAX_PASSWORD_LENGTH 32 //number of characters, not counting string null terminator
#define COMMAND_BUFFER_SIZE 50 //size of buffer used for constructing commands to the ESP8266
#define MAC_ADDRESS_LENGTH 17  //size of MAC address string, not including null terminator e.g. "DE:AD:BE:EF:AB:BA"
#define IP_ADDRESS_LENGTH 12   //size of ip address string, not including null terminator e.g. 192.168.320.089"
#define DEFAULT_PORT 8080      //server listens on this port by default
#define DEFAULT_MAXCONNS 1     //allow this many incoming connections at once

//todo: put config page values in eeprom (instead of just loading defaults at start.

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
  bool is_progmem;
};


class OutputQueue{
  private:
  string_element queue[MAX_OUTPUT_QUEUE_LENGTH];
  unsigned int queue_len;
  int read_position;
  unsigned int total_size;

  public:
  OutputQueue();
  void add_element(char * string, unsigned int string_len, bool is_progmem);
  //reset queue position and length. Automatic when you get the last element.
  void clear_elements();
  //gets the element
  bool get_element(string_element * output);//returns false if none are available.
  unsigned int get_total_size();
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
  char * buf;
  unsigned int head;
  unsigned int tail;
  unsigned int buf_size; //of the buffer
  
public:
  CircularBuffer(int buf_size);
  void buf_reset();
  bool buf_put(char data);
  bool buf_get(char * data);
  bool is_empty();
  bool is_full();
  void read_buffer_to_string(char string[]);
  
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
struct network_info{
  char ssid[MAX_SSID_LENGTH+1];
  char password[MAX_PASSWORD_LENGTH+1];
  char ip[IP_ADDRESS_LENGTH+1];
  char macaddr[MAC_ADDRESS_LENGTH+1];
};

struct server_info{
  unsigned int port;
  unsigned char maxconns;
};

class ESP8266{
private:
    bool dump_reads,dump_writes;  //verbosity flags
    AltSoftSerial *port;       //Initialized outside of this class
    CircularBuffer * serial_input_buffer; //todo: Replace with char ring buffer
    bool verbose;               
    OutputQueue output_queue;   //Does not hold data, just pointers to data
    char prefetch_output_buffer[PREFETCH_OUTPUT_BUFFER_SIZE];
    unsigned int prefetch_output_buffer_len;
    network_info station;
    server_info server;
    void query_network_ssid();
    void query_ip_and_mac();
    bool is_network_connected();

public:
    ESP8266(AltSoftSerial *port, bool verbose);
    void send_http_200_static(unsigned char channel,char page_data[],unsigned int page_data_len);
    void send_http_200_with_prefetch(unsigned char channel,char page_data_0[], unsigned int page_data_0_len,
                                                           char page_data_2[], unsigned int page_data_2_len,
                                                           const char prefetch_data_fields[][7], unsigned int num_prefetch_data_fields);

    bool read_line(char line_buffer[]);
    void clear_buffer();
    
private:
    bool expect_response_to_command(const char * command, unsigned int command_len,
                                    const char * desired_response, unsigned int response_len,
                                    unsigned int timeout_ms=2000);
    bool setup_device();
    void send_output_queue(unsigned char channel);
    char read_port();
    void write_port(char * write_string, unsigned int len);
};



#endif
