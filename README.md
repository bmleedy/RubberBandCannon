# RubberBandTerminator
The sole intention of this project is to shoot things with rubber bands using the coolest technology possible.

# Intro Video
Link to youtube video of the system working.

# Directory Structure
## /arduino
All Arduino projects go under here
### /arduino/ESP8266_webserver
This is the main application for the rubber band shooting webservice from the arduino and ESP8266.

### /arduino/ESP8266_setup
This is a separate application that, when downloaded to the arduino, scans and configures the ESP8266 to the correct baud rate and other default settings.

### /arduino/RubberBandTerminator
This is a simple standalone application to operate the turret with an IR remote - included in case I want to use the IR remote.


## /mechanical
Mechanical drawings go under here.  Expect to find:

* Sketchup drawings of the launcher mechanicals
* STL files to for the launcher mechanicals
* Parts list of all hardware in the system


## /electrical
Electrical schematics go under here.  Expect to find: 
* Electrical drawins of the system
* Calculation and notes


## /documents
Documentation goes under here

# Libraries
This project depends on the following Arduino Libraries:

* [AltSoftSerial](https://github.com/PaulStoffregen/AltSoftSerial)
* [SoftwareSerial](https://github.com/PaulStoffregen/SoftwareSerial)
* [ServoTimer2](https://github.com/nabontra/ServoTimer2)
* [MemoryUsage](https://github.com/Locoduino/MemoryUsage)

## Maybe later:
* [ESP8266_SoftwareSerial](https://github.com/Circuito-io/ESP8266_SoftwareSerial)

# License
See the LICENSE file.