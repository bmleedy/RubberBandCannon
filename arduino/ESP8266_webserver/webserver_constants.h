/*
*   webserver_constants.h
*   
*   This file contains static content for the website, all stored in PROGMEM
*   
*/

#ifndef webserver_constants_h
#define webserver_constants_h


/*!
 * @var http_200_start_line
 * 
 * @brief HTTP 200 response start line and trailing newlines.
 * 
 */
//! @todo: add content length header
const char http_200_start_line[] PROGMEM = "HTTP/1.1 200 OK\r\n\r\n";
#define HTTP_200_START_LINE_LEN 19 //! @def length of HTTP 200 start line

const char success_msg[] PROGMEM = "SUCCESS"; //! @var const char success_msg @brief returned on command success
const char failure_msg[] PROGMEM = "FAIL";    //! @var @brief returned on command failure

const char blank_website_text[] PROGMEM = "<!DOCTYPE html>\
<html>\
  success\
</html>";


// This file is generated from static_website.html by invoking the bash script:
//    'source generate_headers_from_html.bash'
// If you update the html file, you need to run the above command to refresh the 
//    generated header file.
#include "static_website.html.hh"
#include "config_website.html.hh"

#endif /* webserver_constants_h */
