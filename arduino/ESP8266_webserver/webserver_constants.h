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
const char static_website_text[] PROGMEM = "<!DOCTYPE html>\
<html>\
  <head>\
    <title>Brett's Rubber Band Cannon</title>\
    <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>\
    <script>\
    $(document).ready(function(){\
      $(\"button\").click(function(){\
        myaction = $(this).attr(\"action\");\
        myvalue = $(this).attr(\"value\");\
        function(myaction,myvalue){\
            alert(\"Action: \" + myaction + \"\\nValue: \" + myvalue);\
        });\
//        $(\"#status_text\").post(\"url_to_post\");\
      });\
    });\
    </script>\
  </head>\
  <body>\
    <h1>Command Buttons</h1>\
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
           </form></td>\
         <tr>\
           <td></td>\
           <td><form action=\"tilt_down\" method=\"post\">\
             <button style=\"width: 100%;height: 50px;\" type=\"submit\">Down</button>\
           </form></td>\
           <td><form action=\"fire\" method=\"post\">\
             <button style=\"width: 100%;height: 50px;background-color: red;\" type=\"submit\">FIRE</button>\
           </form></td>\
         </tr>\
       </table>\
     <h2>Status</h2>\
       <div id=\"status_text\">Button Command Status</div>\
  </body>\
</html>";
#endif /* webserver_constants_h */
