// This webserver displays results from "Eltako Multisensor MS" (http://www.eltako.com/fileadmin/downloads/en/_datasheets/Datasheet_MS.pdf) as html-file.
// You need to connect your arduino hardware serial interface to the RS485 bus of the Eltako via a RS 485 converter (e.g. MAX485) chip.
// You may hook three indicator leds to your arduino to check the internal state
// Yellow LED (pin 3): data is recieved on the serial interface
// Green LED (pin 2): transmitted checksum == calculated checksum
// Red LED (pin 5): transmitted checksum != calculated checksum, throw away measurement results

//sample output:
//RawData.value W+02.5000000J00000.6J?151515151515?1841
//CheckSum.value 1841
//RecvSum.value 1841
//Temperature.value 2.50
//Daylight.value 0
//Dawn.value 1
//Rain.value 1
//Windspeed.value 0.60
//SunSouth.value 0
//SunWest.value 0
//SunEast.value 0

#define DEBUG // output via hardware serial bus for debugging purposes, can only be used on arduinos with two hardware serial bus (e.g. arduino mega)

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print (x)
#else
  #define DEBUG_PRINT(x)
#endif

#ifdef DEBUG
  #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
  #define DEBUG_PRINTLN(x)
#endif

#include <Ethernet.h>

#define greenledPIN 2
#define yellowledPIN 3
#define redledPIN 5

#define dlytime 10
#define interval 1000 // check serial interface every 1000 milliseconds

#define BUFFER_SIZE 64
#define MSG_SIZE 40

byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x01 };
//DHCP, if you want to define a static ip then add: IPAddress ip(192,168,1,152);
EthernetServer server(80);


char recv_str[BUFFER_SIZE];

float temp_multisens = 0;
int sun_south = 0;
int sun_west = 0;
int sun_east = 0;
boolean dawn = false;
int daylight = 0;
float windspeed = 0;
boolean rain = false;
int recv_sum = 0;
int checksum = 0;

long previousMillis = 0;

boolean check_checksum(char *string) {
// true: calculated checksum equals transferred checksum
// false: calculated checksum != transferred checksum
    
  char buffer[5];
  boolean ret_val;
  checksum = 0;
  recv_sum = 0;
  for (int i=0; i<MSG_SIZE-5; i++) checksum+=string[i]; // calculate the checksum
  strncpy (buffer, string+MSG_SIZE-5, 4);
  buffer[4] = '\0';
  recv_sum = atoi(buffer); // convert transferred checksum to integer
  DEBUG_PRINTLN (checksum);
  DEBUG_PRINTLN (recv_sum);
  if (recv_sum == checksum) {
    ret_val = true;
    digitalWrite (greenledPIN, HIGH); // everythings fine, calculated checksum equals transferred checksum --> blink green led
    delay (dlytime);
    digitalWrite (greenledPIN, LOW);
    delay (dlytime);
  } 
  else {
    ret_val = false;
    digitalWrite (redledPIN, HIGH);  // data transferred seems corrupt, calculated checksum != transferred checksum --> blink red led
    delay (dlytime);
    digitalWrite (redledPIN, LOW);
    delay (dlytime);
  }
  return (ret_val);
}


void convert_rcvstr (char *string)
// split string into single values
{
 char buffer[6];
 if (check_checksum (string)) {
   strncpy (buffer, string+1, 5);
   buffer[5] = '\0';
   temp_multisens = atof (buffer);
   DEBUG_PRINTLN (temp_multisens);
   strncpy (buffer, string+6, 2);
   buffer[2] = '\0';
   sun_south = atoi (buffer);
   DEBUG_PRINTLN (sun_south);
   strncpy (buffer, string+8, 2);
   buffer[2] = '\0';
   sun_west = atoi (buffer);
   DEBUG_PRINTLN (sun_west);
   strncpy (buffer, string+10, 2);
   buffer[2] = '\0';
   sun_east = atoi (buffer);
   DEBUG_PRINTLN (sun_east);
   string [12] == 'J' ? dawn = true : dawn = false;
   DEBUG_PRINTLN (dawn);
   strncpy (buffer, string+13, 3);
   buffer[3] = '\0';
   daylight = atoi (buffer);
   DEBUG_PRINTLN (daylight);
   strncpy (buffer, string+16, 4);
   buffer[4] = '\0';
   windspeed = atof (buffer);
   DEBUG_PRINTLN (windspeed);
   string [20] == 'J' ? rain = true : rain = false;
   DEBUG_PRINTLN (rain);
 }
}

void read_serial() {
  int index;
  DEBUG_PRINT ("Serial Buffer: ");
  DEBUG_PRINTLN (Serial.available());
  if (Serial.available() > 0) {
    index=0;
    digitalWrite (yellowledPIN, HIGH); // there is some data on the serial bus --> turn on yellow led
   
    while (Serial.available() > 0) { 
      char inByte = Serial.read();
      if (index < BUFFER_SIZE-1) {
        recv_str[index] = inByte;
        index++;
      }
      else break;  
    } 
    delay (dlytime);
    digitalWrite (yellowledPIN, LOW); // data transmission has ended --> turn off yellow led
    delay (dlytime);
    recv_str[index] = '\0';
    DEBUG_PRINTLN (recv_str);
    convert_rcvstr (recv_str);
  } 
}


void listenForWebClients() {
  //listen for incoming web clients
  char buffer[90];
  
  EthernetClient client = server.available();
  if (client) {
    DEBUG_PRINTLN ("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.print("HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\nRefresh: 5\n\n<!DOCTYPE HTML>");
          snprintf (buffer,90,"<html>\nRawData.value %s<br>\n",recv_str);
          client.print(buffer);
          snprintf (buffer,90,"CheckSum.value %d<br>\nRecvSum.value %d<br>Temperature.value ",checksum,recv_sum);
          client.print(buffer);
          client.print(temp_multisens);
          snprintf (buffer,90,"<br>Daylight.value %d<br>\nDawn.value %d<br>\nRain.value %d<br>\nWindspeed.value ",daylight,dawn,rain);
          client.print(buffer);
          client.print (windspeed);
          snprintf (buffer,90,"<br>SunSouth.value %d<br>\nSunWest.value %d<br>\nSunEast.value %d<br>",sun_south,sun_west,sun_east);
          client.print(buffer);
          client.print("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void setup() {
  Serial.begin(19200);
  
  Ethernet.begin(mac);
  server.begin();
  DEBUG_PRINT ("server is at ");
  DEBUG_PRINTLN (Ethernet.localIP()); 
  
  pinMode (redledPIN, OUTPUT);
  pinMode (yellowledPIN, OUTPUT);
  pinMode (greenledPIN, OUTPUT);
  digitalWrite (redledPIN, HIGH);
  delay (500);
  digitalWrite (redledPIN, LOW);
  delay (500);
  digitalWrite (yellowledPIN, HIGH);
  delay (500);
  digitalWrite (yellowledPIN, LOW);
  delay (500);
  digitalWrite (greenledPIN, HIGH);
  delay (500);
  digitalWrite (greenledPIN, LOW);
  delay (500);
}

void loop() {
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis > interval) { //  read the serial bus every second
    previousMillis = currentMillis;   
    read_serial(); // read the serial bus
  }
  listenForWebClients(); //listen for clients and output html-text
}

