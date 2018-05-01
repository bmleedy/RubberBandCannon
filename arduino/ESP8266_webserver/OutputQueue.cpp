/*!
 * @file OutputQueue.h
 * 
 * @brief Helper Class to hold ESP8266 class serial output data
 * 
 *    
 */
 #include "OutputQueue.h"
 #include <Arduino.h>

/*! 
 * Holds pointers to strings, their sizes, and a tally of the sizes
 * of all of the strings that need to be outputted.
 * 
 * (see header file for usage)
 */
OutputQueue::OutputQueue(){
  this->clear_elements();
}


/*!
 * Add an element to this queue.
 * 
 * @param string
 *        pointer to the string to be written
 *        
 * @param string_len
 *        length of string to be written
 *        
 * @param is_progmem
 *        set to true if this is a progmem string
 */
void OutputQueue::add_element(char * string, unsigned int string_len, bool is_progmem){
  if(queue_len >= MAX_OUTPUT_QUEUE_LENGTH){
    Serial.println(F("| OutputQueue::add_element: Max output queue length exceeded! Check your code!"));
  }else if(read_position != 0){
    Serial.println(F("| OutputQueue::add_element: Cannot add elements to an output queue that's partially read!"));
  }else{
    queue[queue_len].pointer = string;
    queue[queue_len].string_length = string_len;
    queue[queue_len].is_progmem = is_progmem;
    total_size += string_len;  //tally up size of referenced strings
    queue_len++;
  }
}


/*!
 * Clear the queue and the read position at the same time
 */
void OutputQueue::clear_elements(){
  //no need to actually erase the data, just reset list position
  queue_len = 0;
  read_position = 0;
  total_size = 0;
}


/*!
 * returns false if none are available.
 * resets the queue and returns false when there is nothing left to read
 * 
 * @param output
 *        A pointer to the data field to which we'll copy the next field. 
 *        
 * @return TRUE of an element is available
 */
bool OutputQueue::get_element(string_element * output){
  if(read_position >= queue_len){
    //nothing to read
    this->clear_elements();
    return false;
  } else {
    *output = queue[read_position];
    read_position++;
  }
  return true;
}


/*!
 * Returns the sum of the lengths of the enqueued strings
 * 
 * @return the sum of the lengths of the strings in the output buffer.
 */
unsigned int OutputQueue::get_total_size(){
  return this->total_size;
}

