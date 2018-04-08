/*
*   webserver_constants.h
*   
*   This file contains static content for the website, all stored in PROGMEM
*   
*/

#ifndef webserver_constants_h
#define webserver_constants_h

// https://tools.ietf.org/html/rfc2616#page-31
//todo: add content length header
const char http_200_start_line[] PROGMEM = "HTTP/1.1 200 OK\r\n\r\n";
#define HTTP_200_START_LINE_LEN 19

//  http://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html
// https://www.w3schools.com/jquery/jquery_ajax_intro.asp
// https://www.w3schools.com/jquery/jquery_examples.asp


const char blank_website_text[] PROGMEM = "<!DOCTYPE html>\
<html>\
  success\
</html>";


// This file is generated from static_website.html by invoking the bash script:
//    'source escape_htmlfile.bash static_website.html'
// If you update the html file, you need to run the above command to refresh the 
//    generated header file.
#include "static_website.html.hh"

#endif /* webserver_constants_h */
