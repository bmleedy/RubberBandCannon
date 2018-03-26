/*
*   webserver_constants.h
*   
*   This file contains static content for the website, all stored in PROGMEM
*   
*/
   


#ifndef webserver_constants_h
#define webserver_constants_h

// https://tools.ietf.org/html/rfc2616#page-31
const char http_200_start_line[] PROGMEM = "HTTP/1.1 200 OK\r\n\r\n";
#define HTTP_200_START_LINE_LEN 19

//  http://www.nongnu.org/avr-libc/user-manual/group__avr__pgmspace.html
const char static_website_text[] PROGMEM = "<!DOCTYPE html>\
<html>\
  <head><title>Brett's IOT Device</title></head>\
  <body>\
    <h1>Hello, fine world.</h1>\
      <p>I am Brett. :-)</p>\
    <h1>Arduino Stats.</h1>\
      <p>todo: put some stats in here.</p>\
    <h1>Button Test Area</h1>\
      <h2>The post button</h2>\
        <table style=\"width:100%\">\
          <tr height=\"80\">\
            <td></td>\
            <td><form action=\"tilt_up\" method=\"post\">\
                <button style=\"width: 100%;height: 50px;\" type=\"submit\">Up</button>\
            </form></td>\
            <td></td>\
          </tr>\
          <tr>\
           <td><form action=\"pan_left\" method=\"post\">\
             <button style=\"width: 100%;height: 50px;\" type=\"submit\">Left</button>\
           </form></td>\
           <td></td>\
           <td><form action=\"pan_right\" method=\"post\">\
             <button style=\"width: 100%;height: 50px;\" type=\"submit\">Right</button>\
           </form>\</td>\
         <tr>\
           <td></td>\
           <td><form action=\"tilt_down\" method=\"post\">\
             <button style=\"width: 100%;height: 50px;\" type=\"submit\">Down</button>\
           </form></td>\
           <td><form action=\"fire\" method=\"post\">\
             <button style=\"width: 100%;height: 50px;\" type=\"submit\">FIRE</button>\
           </form></td>\
         </tr>\
       </table>\
  </body>\
</html>";

#endif /* webserver_constants_h */
