/*!
 * @file ESP8266.h
 * 
 * @brief Interface class and helper classes to interface with the ESP8266
 * 
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

#ifndef ESP8266_H
#define ESP8266_H

//#include <SoftwareSerial.h>
#include <AltSoftSerial.h>
#include <Arduino.h>
#include "HardwareSerial.h"
#include "webserver_constants.h"

/*! @def SERIAL_INPUT_BUFFER_MAX_SIZE
 *  This is the size of the buffer I will use to read data from the ESP8266.
 *  Since I read one line at a time, this value needs to be larger than the 
 *  length of the longest line that I'll read from the device. 
 *  Lines always end in '\n'.
 *
 *  I want to minimize the size of this buffer, as it occupies a fixed 
 *  amount of space in my class, whether or not it is used.*/
#define SERIAL_INPUT_BUFFER_MAX_SIZE 300
/*! @def PREFETCH_OUTPUT_BUFFER_SIZE
 * the fixed size we allocate to prefetching data for pages.  
 *  This is a buffer of data I use to queue up dynamic strings to be written 
 *    to the ESP8266 serial port.  It was originally used when I needed to prefetch
 *    dynamic data for my website, but now it's used for any thing the current 
 *    method needs to queue up before sending an HTTP response.*/
#define PREFETCH_OUTPUT_BUFFER_SIZE  100  //! @def
/*! @def MAX_RESPONSE_LINE_LEN
 * the longest single line we expect in a response, including "\r\n\0".  
 *  This is the longest length of line I expect to receive back from the 
 * ESP8266 in response to my command.  It does not include the length of web
 * requests, which might have longer line lengths.*/
#define MAX_RESPONSE_LINE_LEN 50
/*! @def MAX_OUTPUT_QUEUE_LENGTH
 *  My output queue is an array of pointers to elements of data I will output
 *  via the ESP8266 serial port.  Minimize this to save on class memory footprint.*/
#define MAX_OUTPUT_QUEUE_LENGTH  20
/*! @def MAX_SSID_LENGTH
 *  number of characters, not counting string null terminator.*/
#define MAX_SSID_LENGTH 32
/*! @def MAX_PASSWORD_LENGTH
 *  number of characters, not counting string null terminator.*/
#define MAX_PASSWORD_LENGTH 32
/*! @def COMMAND_BUFFER_SIZE
 *  size of buffer used for constructing commands to the ESP8266. Should be 
 *   The largest string length command you might send.*/
#define COMMAND_BUFFER_SIZE 50
/*! @def MAC_ADDRESS_LENGTH
 *  ASCII-encoded MAC address. Number of characters, not counting string null terminator.
 *  e.g. "DE:AD:BE:EF:AB:BA" */
#define MAC_ADDRESS_LENGTH 17
/*! @def IP_ADDRESS_LENGTH
 *  max length of an ASCII-encoded IP address. Number of characters, not counting string 
 *  null terminator.  e.g. 192.168.320.089"*/
#define IP_ADDRESS_LENGTH 12
/*! @def DEFAULT_PORT
 *  Default webserver port, if not loaded from anywhere else.*/
#define DEFAULT_PORT 8080
/*! @def DEFAULT_MAXCONNS
 *  Webserver will allow only up to this many incoming connections at once*/
#define DEFAULT_MAXCONNS 1

//! @todo put config page values in eeprom (instead of just loading defaults at start).

 
/*! 
 * @struct string_element
 * 
 * @brief Datatype for an element in the output queue
 * 
 */
struct string_element{
  char * pointer;             ///<pointer to a string element
  unsigned int string_length; ///<length of the string element
  bool is_progmem;            ///<true if the string element is stored in progmem
};

/*!
 * @class OutputQueue
 * 
 * @brief Holds pointers to strings to be sent to the ESP serial port
 * 
 * Holds pointers to strings, their sizes, and a tally of the sizes
 * of all of the strings that need to be outputted.
 * 
 * This is a huge space saver compared to keeping an output buffer 
 * in dynamic memory, where it may topple your heap.  It's only 
 * dynamic memory usage is the compact array of pointers to strings 
 * and string lengths.
 * 
 *        
 * Usage:<pre>
 *    char string1[10] = "1234567890";
 *    char string2[3]  = "321";
 *    OutputQueue myqueue;
 *    myqueue.add_element(string1,sizeof(string1);
 *    myqueue.add_element(string2,sizeof(string2);
 *    //...etc, etc, up to max number of elements.
 *    string_element output;
 *    while(myqueue.get_element(&output)){
 *      my_method_to_use_the_output_strings(output);
 *    }</pre>
 *-----------------------------------------------------------------
 */
class OutputQueue{
  private:
  string_element queue[MAX_OUTPUT_QUEUE_LENGTH];  ///<A list of pointers to elements to output
  unsigned int queue_len;                         ///<Number of elements in the queue
  unsigned int read_position;                     ///<Index of most recently read element
  unsigned int total_size;                        ///<Total number of characters in the buffer

  public:
  OutputQueue();
  void add_element(char * string, unsigned int string_len, bool is_progmem);
  //reset queue position and length. Automatic when you get the last element.
  void clear_elements();
  //gets the element
  bool get_element(string_element * output);//returns false if none are available.
  unsigned int get_total_size();
};


/*!
 * @class CircularBuffer
 * 
 * @brief Ring buffer Implementation
 * 
 * Simple ring buffer to handle serial output that we want to scan for string matches.
 * 
 *-----------------------------------------------------------------
 */

class CircularBuffer{
private:
  char * buf;             ///<pointer to the data buffer
  unsigned int head;      ///<current location of the newest data
  unsigned int tail;      ///<current location of the oldest data
  unsigned int buf_size;  ///<current distance between the head and tail
  
public:
  CircularBuffer(int buf_size);
  void buf_reset();
  bool buf_put(char data);
  bool buf_get(char * data);
  bool is_empty();
  bool is_full();
  void read_buffer_to_string(char string[]);
  
};



/*! 
 * @struct network_info
 * 
 * @brief All of the info needed to specify the ESP8266 network connectivity parameters.
 * 
 */
struct network_info{
  char ssid[MAX_SSID_LENGTH+1];        ///< SSID string
  char password[MAX_PASSWORD_LENGTH+1];///< Password string
  char ip[IP_ADDRESS_LENGTH+1];        ///< IP Address string
  char macaddr[MAC_ADDRESS_LENGTH+1];  ///< MAC Address string
};

/*! 
 * @struct server_info
 * 
 * @brief All of the info needed to specify the ESP8266 server parameters.
 * 
 */
struct server_info{
  unsigned int port;       ///< Port on which the server should listen
  unsigned char maxconns;  ///< The maximum number of connections the server will support
};

/*!
 * @class ESP8266
 *
 * @brief Interface class with the ESP8266 device.
 *
 * This class is an ESP8266 for the purposes of my IOT work.  On creation,
 * it will set the device to:
 *  * Only act as a client of an access point, not an access point itself
 *  * Connect to the house access point
 *  * Serve a multi-connection TCP server on port 8080
 *<pre>
 * USAGE:
 *   ESP8266 * myesp;
 *   myesp = new ESP8266(&serial_port, verbose_flag);
 *   while(1){
 *     if(myesp->check_for_request(F("GET"))){
 *       Serial.println(F("got a request!!!"));
 *       myesp->send_http_200_static(0,(char*)static_website_text,
 *                                   sizeof(static_website_text));
 *</pre>
 */
class ESP8266{
private:
    bool dump_reads,dump_writes;            ///<verbosity flags
    AltSoftSerial *port;                    ///<Initialized outside of this class
    CircularBuffer * serial_input_buffer;   ///<Pointer to the buffer we use for input
    bool verbose;                           ///<overall verbosity
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
                                                           const char prefetch_data_fields[][7], 
                                                           unsigned int num_prefetch_data_fields);
    void send_networks_list(unsigned char channel);
    bool read_line(char line_buffer[]);
    bool read_line(char line_buffer[], unsigned int timeout_ms);
    void clear_buffer();
    bool set_station_ssid__(char new_ssid[]);
    bool set_station_passwd(char new_password[]);
    
private:
    bool expect_response_to_command(const char * command, unsigned int command_len,
                                    const char * desired_response,
                                    unsigned int timeout_ms=2000);
    bool setup_device();
    void send_output_queue(unsigned char channel);
    char read_port();
    void write_port(char * write_string, unsigned int len);
};



#endif
