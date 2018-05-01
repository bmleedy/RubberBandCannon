/*!
 * @file CircularBuffer.h
 * 
 * @brief Ring buffer implementation for the ESP8266 serial input buffer
 *    
 */

/*!
 * @class CircularBuffer
 * 
 * @brief Ring buffer Implementation
 * 
 * Simple ring buffer to handle serial output that we want to scan for string matches.
 * 
 *-----------------------------------------------------------------
 */
#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

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
  unsigned int get_buf_size(){return buf_size;}
  void read_buffer_to_string(char string[], unsigned int max_size);
  
};

#endif
