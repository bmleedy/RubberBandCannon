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
#include "CircularBuffer.h"
#include "OutputQueue.h"
#include <EEPROM.h>

// Devug definition
#define PRINT_SERIAL_STREAM true ///<If set, all data to and from the ESP8266 is dumped to the debug serial port.

#if PRINT_SERIAL_STREAM
#define PRINTSTR_IF_VERBOSE(x){Serial.print(F(x));}
#define PRINT_IF_VERBOSE(x){Serial.print(x);}
#define PRINTSTRLN_IF_VERBOSE(x){Serial.print(F(x));}
#define PRINTLN_IF_VERBOSE(x){Serial.println(x);}
#define WRITE_IF_VERBOSE(x,y){Serial.write(x,y);}
#else
#define PRINTSTR_IF_VERBOSE(x){}
#define PRINT_IF_VERBOSE(x){}
#define PRINTSTRLN_IF_VERBOSE(x){}
#define PRINTLN_IF_VERBOSE(x){}
#define WRITE_IF_VERBOSE(x,y){}
#endif

/*! @def SERIAL_INPUT_BUFFER_MAX_SIZE
 *  This is the size of the buffer I will use to read data from the ESP8266.
 *  Since I read one line at a time, this value needs to be larger than the 
 *  length of the longest line that I'll read from the device. 
 *  Lines always end in '\n'.
 *
 *  I want to minimize the size of this buffer, as it occupies a fixed 
 *  amount of space in my class, whether or not it is used.*/
#define SERIAL_INPUT_BUFFER_MAX_SIZE 400
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
#define MAX_RESPONSE_LINE_LEN 100
/*! @def MAX_SSID_LENGTH
 *  number of characters, not counting string null terminator.*/
#define MAX_SSID_LENGTH 32
/*! @def MAX_PASSWORD_LENGTH
 *  number of characters, not counting string null terminator.*/
#define MAX_PASSWORD_LENGTH 32
/*! @def COMMAND_BUFFER_SIZE
 *  size of buffer used for constructing commands to the ESP8266. Should be 
 *   The largest string length command you might send.*/
#define COMMAND_BUFFER_SIZE 75
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

#define INITIALIZED_LETTER 'z'

//! @todo put config page values in eeprom (instead of just loading defaults at start).

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

char *strnstr_P(char *haystack, PGM_P needle, size_t haystack_length);

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
    AltSoftSerial *port;                    ///<Initialized outside of this class
    CircularBuffer * serial_input_buffer;   ///<Pointer to the buffer we use for input
    bool verbose;                           ///<overall verbosity
    OutputQueue output_queue;   //Does not hold data, just pointers to data
    char prefetch_output_buffer[PREFETCH_OUTPUT_BUFFER_SIZE];
    unsigned int prefetch_output_buffer_len;
    network_info station;
    network_info ap;
    server_info server;
    void query_network_ssid();
    void query_ip_and_mac();
    bool is_network_connected();
    char current_channel;
    int eeprom_address;

public:
    ESP8266(AltSoftSerial *port, bool verbose, int eeprom_address);
    void send_http_200_static(unsigned char channel,char page_data[],unsigned int page_data_len);
    void send_http_200_with_prefetch(unsigned char channel,char page_data_0[], unsigned int page_data_0_len,
                                                           char page_data_2[], unsigned int page_data_2_len,
                                                           const char* const prefetch_data_fields[], 
                                                           unsigned int num_prefetch_data_fields);
    void send_networks_list(unsigned char channel);
    bool read_line(char line_buffer[], unsigned int line_buffer_size);
    bool read_line(char line_buffer[], unsigned int line_buffer_size, unsigned int timeout_ms);
    void clear_buffer();
    void purge_serial_input(unsigned int timeout);
    bool set_station_ssid_and_passwd(char new_ssid_and_passwd[]);
    bool set_ap_ssid_and_passwd(char new_ssid_and_passwd[]);
    char get_channel_from_string(char line[], int len);
    void process_settings(unsigned char channel, char input_line[], int input_line_size);
    
private:
    bool expect_response_to_command(const char * command, unsigned int command_len,
                                    const char * desired_response,
                                    unsigned int timeout_ms);
    bool setup_device();
    void send_output_queue(unsigned char channel);
    char read_port();
    void write_port(char * write_string, unsigned int len);
    void update_eeprom();
};



#endif
