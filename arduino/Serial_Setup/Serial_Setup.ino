#include <AltSoftSerial.h>
#include <MemoryUsage.h>

#define SERIAL_BAUD_RATE 115200

/*! @def COMMAND_BUFFER_SIZE
 *  size of buffer used for constructing commands to the ESP8266. Should be 
 *   The largest string length command you might send.*/
#define COMMAND_BUFFER_SIZE 75

const unsigned long int serial_baud[] = {4800, 9600, 19200, 38400, 57600, 115200};
const int serial_baud_len = 6;

#define LINE_BUFFER_LENGTH 200
char serial_buffer[200];

#define SCAN_TIMEOUT_MS 1000
#define INTER_PHASE_DELAY_MS 1000

AltSoftSerial soft_port;

void print_ok(){
  Serial.println(F("[OK]"));
}

void print_fail(){
  Serial.println(F("[FAIL]"));
}

void purge_serial_input(unsigned int timeout){
  unsigned int start_time = millis();
  while (soft_port.available() || ((millis()-start_time) >= timeout)) {
    //read the port and do nothing 
    if(soft_port.available())
      soft_port.read();
  }
}


bool read_line(char line_buffer[]){
  char latest_byte = '\0';
  int buffer_iterator = 0;
  
  while (soft_port.available()) {
    latest_byte = soft_port.read();

    // Add the byte I read to the input buffer
    serial_buffer[buffer_iterator] = latest_byte;
    buffer_iterator++;
    serial_buffer[buffer_iterator] = '\0';
    if(latest_byte == '\n') {
      strcpy(line_buffer, serial_buffer);
      return true;
    }
    delay(10);
  }
  // No \n found.
  return false;
}

bool expect_response_to_command(const char * command, unsigned int command_len,
                                const char * desired_response,
                                unsigned int timeout_ms){
  char response_line[LINE_BUFFER_LENGTH] = "";

  // Write the command
  soft_port.write((char *)command, command_len);
  
  // Spin for timeout_ms
  unsigned int start_time = millis();
  while((millis() - start_time) < timeout_ms){
    if(read_line(response_line)){
      // a line is found
      if(strstr(response_line,desired_response) != NULL) {
        //line has response string, return success
        return true;
      } else {
        // line does not have response string
        // continue reading more lines
      }
    }//if(read_line)
  }//while(millis...)
  Serial.println(F("| expect_response: Timeout"));
  return false;
}


void setup() {
  // Setup the debug serial port
  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F(""));
  Serial.println(F("| Debug serial Port Initialized..."));
  Serial.println(F("| Searching the software serial port for an ESP8266..."));

  bool esp_found = false;
  int iterator = 0;
  while(!esp_found){
    iterator++;
    iterator = iterator % serial_baud_len;
    Serial.print(F("| Trying serial baud ")); Serial.println(serial_baud[iterator],DEC);
    
    soft_port.begin(serial_baud[iterator]);
    delay(1000);
    esp_found = expect_response_to_command("AT\r\n", 4,"OK", SCAN_TIMEOUT_MS);
    soft_port.end();
    if(esp_found){
      Serial.print(F("| ESP8266 found at baud rate "));Serial.println(serial_baud[iterator],DEC);
      Serial.println("");
      break;
    }
    Serial.print(F("| Attempt Done. Free Memory: "));Serial.println(mu_freeRam());
  }

  delay(INTER_PHASE_DELAY_MS);  // just for human-readability

  Serial.print(F("| Setting default serial baud rate to :"));Serial.println(SERIAL_BAUD_RATE);
  sprintf(serial_buffer,"AT+UART_DEF=%ld,8,1,0,0\r\n",SERIAL_BAUD_RATE);
  soft_port.write(serial_buffer,strlen(serial_buffer));
  Serial.println(F("| Command Sent to the ESP8266  Scanning again"));
  Serial.println("");
  
  delay(INTER_PHASE_DELAY_MS);  // just for human-readability

  esp_found = false;
  while(!esp_found){
    iterator++;
    iterator = iterator % serial_baud_len;
    Serial.print(F("| Trying serial baud ")); Serial.println(serial_baud[iterator],DEC);
    
    soft_port.begin(serial_baud[iterator]);
    delay(1000);
    esp_found = expect_response_to_command("AT\r\n", 4,"OK", SCAN_TIMEOUT_MS);
    soft_port.end();
    if(esp_found){
      Serial.print(F("| SUCCESS!  ESP8266 found at baud rate "));Serial.println(serial_baud[iterator],DEC);
      break;
    }
    Serial.print(F("| Attempt Done. Free Memory: "));Serial.println(mu_freeRam());
  }

  
  Serial.println(F("\n\nNow, setting up the ESP8266-------------------"));

  char request_buffer[COMMAND_BUFFER_SIZE]; 
  char response_buffer[COMMAND_BUFFER_SIZE];
  int setup_stage_delay=1000;
 
  // Get a response from anyone
  Serial.print(F("| ESP8266 - Waiting for a response from the Wifi Device..."));
  while(!expect_response_to_command("AT\r\n",4,"OK",2000)){
      delay(1000);
  }
  print_ok();
  
  Serial.print(F("| ESP8266 - Checking the device CWMODE..."));
  // Set myself up as a client of an access point.
  strncpy_P(request_buffer,PSTR("AT+CWMODE?\r\n"), COMMAND_BUFFER_SIZE);
  strncpy_P(response_buffer,PSTR("+CWMODE:3"),COMMAND_BUFFER_SIZE);
  while(true){
    purge_serial_input(setup_stage_delay);
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,2000)){
        print_ok();
        break;
    } else {
      Serial.print (F("\n| ESP8266 -    Setting the mode to 'client mode'"));
      strncpy_P(request_buffer,PSTR("AT+CWMODE_DEF=3\r\n"), COMMAND_BUFFER_SIZE);
      strncpy_P(response_buffer,PSTR("OK"), COMMAND_BUFFER_SIZE);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,2000)){
          print_ok();
          break;
      }
      else {
          print_fail();
      }
    }
  }

  // configure the cannon AP
  Serial.print(F("| ESP8266 - configuring my own access point..."));
  strncpy_P(request_buffer,PSTR("AT+CWSAP_DEF?\r\n"), COMMAND_BUFFER_SIZE);
  snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("+CWSAP_DEF:\"%s\",\"%s\",1,3"),"leedy","teamgoat");
  while(true){
    purge_serial_input(setup_stage_delay);
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,2000)){
        print_ok();
        break;
    } else {
      Serial.print (F("\n| ESP8266 -     Access point not set up.  Setting up now..."));
      snprintf_P(request_buffer,COMMAND_BUFFER_SIZE,PSTR("AT+CWSAP_DEF=\"%s\",\"%s\",1,3,4,0\r\n"),"leedy","teamgoat");
      strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,
                                    10000)){
          print_ok();
          break;
      }
      else {
          print_fail();
      }
    }
  }


  
  // configure the cannon AP
  Serial.print(F("| ESP8266 - configuring my ip address on cannon_ap network to 192.168.4.1..."));
  strncpy_P(request_buffer,PSTR("AT+CIPAP_DEF?\r\n"), COMMAND_BUFFER_SIZE);
  snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("+CIPAP_DEF:ip:\"%s\""),"192.168.4.1");
  while(true){
    purge_serial_input(setup_stage_delay);
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,2000)){
        print_ok();
        break;
    } else {
      Serial.print (F("\n| ESP8266 -  ip address on cannon_ap not set up.  Setting up now..."));
      snprintf_P(request_buffer,COMMAND_BUFFER_SIZE,PSTR("AT+CIPAP_DEF=\"%s\",\"%s\",\"255.255.255.0\"\r\n"),"192.168.4.1","192.168.4.1");
      strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,
                                    10000)){
          print_ok();
          break;
      }
      else {
          print_fail();
      }
    }
  }

  
  // Now join the house access point
  Serial.print(F("| ESP8266 - Checking that we are on the correct network..."));
  strncpy_P(request_buffer,PSTR("AT+CWJAP?\r\n"), COMMAND_BUFFER_SIZE);
  snprintf_P(response_buffer,COMMAND_BUFFER_SIZE,PSTR("+CWJAP:\"%s\""),"leedy");
  while(true){
    purge_serial_input(setup_stage_delay);
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,2000)){
        print_ok();
        break;
    } else {
      Serial.print (F("\n| ESP8266 -     Not on the correct network. Fixing the WiFi settings..."));
      snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CWJAP_DEF=\"%s\",\"%s\"\r\n"),"leedy","teamgoat");
      strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
      if(expect_response_to_command(request_buffer,
                                    strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                    response_buffer,
                                    10000)){
          print_ok();
          break;
      }
      else {
          print_fail();
      }
    }
  }

  // Now setup the CIP Server
  Serial.print(F("ESP8266 - Setting server maxconns..."));
  snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CIPSERVERMAXCONN=%d\r\n"),1);
  strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
  while(true){
    purge_serial_input(setup_stage_delay);
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,
                                  10000)){
      print_ok();
      break;
    } else {
      print_fail();
    }
  }

  // Set ourselves up to mux connections into our little server
  Serial.print(F("| ESP8266 - Checking the CIPMUX Settings..."));
  strncpy_P(request_buffer,PSTR("AT+CIPMUX?\r\n"), COMMAND_BUFFER_SIZE);
  strncpy_P(response_buffer,PSTR("+CIPMUX:1"),COMMAND_BUFFER_SIZE);
  while(true){
    purge_serial_input(setup_stage_delay);
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,2000)){
        print_ok();
        break;
    } else {
        Serial.print(F("\n| ESP8266 -    Server not enabled yet. Setting CIPMUX=1..."));
        strncpy_P(request_buffer,PSTR("AT+CIPMUX=1\r\n"), COMMAND_BUFFER_SIZE);
        strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
        if(expect_response_to_command(request_buffer,
                                      strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                      response_buffer,
                                      10000)){
            print_ok();
            break;
        }
        else {
            print_fail();
        }
    }
  }
  
  // Now setup the CIP Server
  Serial.print(F("| ESP8266 - Configuring my server on port 8080..."));
  snprintf_P(request_buffer, COMMAND_BUFFER_SIZE,PSTR("AT+CIPSERVER=1,%d\r\n"),"8080");
  strncpy_P(response_buffer,PSTR("OK"),COMMAND_BUFFER_SIZE);
  while(true){
    purge_serial_input(setup_stage_delay);
    if(expect_response_to_command(request_buffer,
                                  strnlen(request_buffer,COMMAND_BUFFER_SIZE),
                                  response_buffer,
                                  10000)){
        print_ok();
        break;
    } else {
        print_fail();
        return false;
    }
  }


}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}
