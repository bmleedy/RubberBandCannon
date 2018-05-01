/*!
 * @file CircularBuffer.cpp
 * 
 * @brief Ring buffer implementation for the ESP8266 serial input buffer
 *    
 */
 #include "CircularBuffer.h"

/*!
 * Simple ring buffer to handle serial output that we want to scan for string matches.
 * 
 * @param buf_size
 *        How much space to allocate for the buffer.
 */
// Constructor just clears the buffer
CircularBuffer::CircularBuffer(int buf_size){
  this->buf = new char[buf_size];
  this->buf_reset();
  this->buf_size = buf_size;
}


/*!
 * Clear the buffer
 */
void CircularBuffer::buf_reset(){
    this->head = 0;
    this->tail = 0;
}


/*! 
 * returns false if we overflowed and lost data.
 * 
 * @param data
 *        One character to insert into the buffer
 *        
 * @return True if the buffer was not full, false otherwise
 */
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

/*!
 * returns true if the buffer had a value to get
 * 
 * @param data
 *        pointer to the data character that we read.
 *        
 * @return True if the buffer is not empty and we can return a character
 */
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


/*!
 * returns true if the buffer's empty
 * 
 * @return True if the buffer is empty
 */
bool CircularBuffer::is_empty(){
  return (head == tail);
}

/*!
 * returns true if the buffer's full
 * 
 * @return True if the buffer is full
 */
bool CircularBuffer::is_full(){
  return ((head + 1) % buf_size) == tail;
}

/*!
 * reads the contents of my buffer to a string pointer provided.
 * 
 * @param string
 *        the string to which we're copying
 */
void CircularBuffer::read_buffer_to_string(char string[], unsigned int max_size){
  unsigned int iter = 0;
  while(!this->is_empty() && iter < max_size){
    // only read out as far as the length of the buffer that I'm writing.
    this->buf_get(string+iter);
    iter++;
  }
  string[iter]='\0';
}

