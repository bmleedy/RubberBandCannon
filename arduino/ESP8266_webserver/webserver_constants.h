/*
//  webserver_constants.h
    http://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html
    https://playground.arduino.cc/Main/PROGMEM
    
//  
//
//  Created by Leedy, Brett on 3/21/18.
*/

#ifndef webserver_constants_h
#define webserver_constants_h

#include "webserver_constants.h"




// https://tools.ietf.org/html/rfc2616#page-31
const char http_200_start_line[] PROGMEM = "HTTP/1.1 200 OK\r\n\r\n";
#define HTTP_200_START_LINE_LEN 19


const char static_website_text[] PROGMEM = "<!DOCTYPE html>\
<html>\
  <head><title>Brett's IOT Device</title></head>\
  <body>\
    <h1>Hello, fine world.</h1>\
      <p>I am Brett. :-)</p>\
    <h1>Arduino Stats.</h1>\
      <p>todo: put some stats in here.</p>\
    <h1>Button Test Area</h1>\
      <h2>The Button Element</h2>\
        <p><button type=\"button\" onclick=\"alert('Hello world!')\">Click Me!</button></p>\
      <h2>The post button</h2>\
        <table style=\"width:100%\">\
          <tr>\
            <td></td>\
            <td><form action=\"tilt_up\" method=\"post\">\
                <button type=\"submit\">Up</button>\
            </form></td>\
            <td></td>\
          </tr>\
          <tr>\
           <td><form action=\"pan_left\" method=\"post\">\
             <button type=\"submit\">Left</button>\
           </form></td>\
           <td></td>\
           <td><form action=\"pan_right\" method=\"post\">\
             <button type=\"submit\">Right</button>\
           </form>\</td>\
         <tr>\
           <td></td>\
           <td><form action=\"tilt_down\" method=\"post\">\
             <button type=\"submit\">Down</button>\
           </form></td>\
           <td></td>\
         </tr>\
       </table>\
  </body>\
</html>";
#endif /* webserver_constants_h */
