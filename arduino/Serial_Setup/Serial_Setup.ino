#include <AltSoftSerial.h>
#include <MemoryUsage.h>

#define SERIAL_BAUD_RATE 19200

const unsigned long int serial_baud[] = {4800, 9600, 19200, 38400, 57600, 115200};
const int serial_baud_len = 6;

#define LINE_BUFFER_LENGTH 200
char serial_buffer[200];

#define SCAN_TIMEOUT_MS 1000
#define INTER_PHASE_DELAY_MS 1000

AltSoftSerial soft_port;

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
  sprintf(serial_buffer,"AT+UART_DEF=%d,8,1,0,0\r\n",SERIAL_BAUD_RATE);
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

  
  Serial.println(F("\n\nDoing nothing after this message-------------------"));

}

void loop() {
  // put your main code here, to run repeatedly:
  delay(100);
}
