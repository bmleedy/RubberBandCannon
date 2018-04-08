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

/*
 *       myself = document.getElementById(\"pan_left\");\
      myval = myself.id;\
      myurl = window.location.href;\
 */
const char blank_website_text[] PROGMEM = "<!DOCTYPE html>\
<html>\
  <body>\
      Command Sent successfully\
  </body>\
</html>";


const char static_website_text[] PROGMEM = "<!DOCTYPE html>\
<html>\
  <head>\
    <link rel=\"icon\" type=\"image/png\" href=\"https://s3-us-west-2.amazonaws.com/rubberbandcannon/Cannon_icon_64.png\">\
    <title>Brett's Rubber Band Cannon</title>\
    <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>\
    <script language=\"javascript\">\
    function doFunction(myarg) {\
      geturl = window.location;\
      var baseUrl = geturl.protocol + \"//\" + geturl.host + \"/\" + myarg;\
      $.post(baseUrl);\
    }\
    </script>\
  </head>\
  <body>\
    <img src=\"https://s3-us-west-2.amazonaws.com/rubberbandcannon/rubberband.jpg\" alt=\"rubber band image\">\
    <br>\
    <h1>Command Buttons</h1>\
        <table style=\"width:100%\">\
          <tr height=\"80\">\
            <td></td>\
            <td><input id=\"tilt_up\" style=\"width: 100%;height: 50px;\" type=\"button\" onclick=\"doFunction('tilt_up')\" value=\"Up\"></td>\
            <td></td>\
          </tr>\
          <tr>\
           <td><input id=\"pan_left\" style=\"width: 100%;height: 50px;\" type=\"button\" onclick=\"doFunction('pan_left')\" value=\"Left\"></td>\
           <td></td>\
           <td><input id=\"pan_right\" style=\"width: 100%;height: 50px;\" type=\"button\" onclick=\"doFunction('pan_right')\" value=\"Right\"></td>\
         <tr>\
           <td></td>\
           <td><input id=\"tilt_down\" style=\"width: 100%;height: 50px;\" type=\"button\" onclick=\"doFunction('tilt_down')\" value=\"Down\"></td>\
           <td><input id=\"fire\" style=\"width: 100%;height: 50px;background-color: red;\" type=\"button\" onclick=\"doFunction('fire')\" value=\"FIRE\"></td>\
         </tr>\
       </table>\
     <h2>Status</h2>\
       <div id=\"status_text\">Button Command Status</div>\
  </body>\
</html>";
#endif /* webserver_constants_h */
