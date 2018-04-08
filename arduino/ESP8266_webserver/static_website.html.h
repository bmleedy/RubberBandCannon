////////////////////////////////////////////////
//GENERATED FILE -- DO NOT HAND-MODIFY!!!!!!!!!!
////////////////////////////////////////////////
const char static_website_text[] PROGMEM = "\
<!DOCTYPE html>\
<html>\
<head>\
<style>\
input {\
width: 100%;\
height: 100px;\
}\
</style>\
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
<tr>\
<td></td>\
<td><input id=\"tilt_up\" type=\"button\" onclick=\"doFunction('tilt_up')\" value=\"Up\"></td>\
<td></td>\
</tr>\
<tr>\
<td><input id=\"pan_left\" type=\"button\" onclick=\"doFunction('pan_left')\" value=\"Left\"></td>\
<td></td>\
<td><input id=\"pan_right\" type=\"button\" onclick=\"doFunction('pan_right')\" value=\"Right\"></td>\
<tr>\
<td></td>\
<td><input id=\"tilt_down\" type=\"button\" onclick=\"doFunction('tilt_down')\" value=\"Down\"></td>\
<td><input id=\"fire\" style=\"background-color: red;\" type=\"button\" onclick=\"doFunction('fire')\" value=\"FIRE\"></td>\
</tr>\
</table>\
</body>\
</html>\
";
