/*!
 * @file OutputQueue.h
 * 
 * @brief Helper Class to hold ESP8266 class serial output data
 * 
 *    
 */
#ifndef OUTPUT_QUEUE_H
#define OUTPUT_QUEUE_H


/*! @def MAX_OUTPUT_QUEUE_LENGTH
 *  My output queue is an array of pointers to elements of data I will output
 *  via the ESP8266 serial port.  Minimize this to save on class memory footprint.*/
#define MAX_OUTPUT_QUEUE_LENGTH  20

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

#endif
