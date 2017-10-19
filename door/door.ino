/**************************************************************************/
/*! 
    @file     readntag203.pde
    @author   KTOWN (Adafruit Industries)
    @license  BSD (see license.txt)

    This example will wait for any NTAG203 or NTAG213 card or tag,
    and will attempt to read from it.

    This is an example sketch for the Adafruit PN532 NFC/RFID breakout boards
    This library works with the Adafruit NFC breakout 
      ----> https://www.adafruit.com/products/364
 
    Check out the links above for our tutorials and wiring diagrams 
    These chips use SPI or I2C to communicate.

    Adafruit invests time and resources providing this open source code, 
    please support Adafruit and open-source hardware by purchasing 
    products from Adafruit!
*/
/**************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>

#define DOOR_PIN (7)
#define LIGHT_PIN (8)


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xCE, 0xEF, 0xFE, 0xF1
};
IPAddress ip(192, 168, 0, 221);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);


bool door_on = true;
bool light_on = false;
String string = String("");

void setup(void) {
//  #ifndef ESP8266
//    long start_time = millis();
//    while (!Serial && millis() - start_time < 2000); // for Leonardo/Micro/Zero
//  #endif
  //Serial.begin(115200);
  //Serial.println("Started");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  //Serial.print("server is at ");
  //Serial.println(Ethernet.localIP());

  pinMode(DOOR_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);

  turn_door_on();
  turn_light_off();

}

void loop(void) {
  if(door_on) {
    turn_door_on();
  } else {
    turn_door_off();
  }

  if(light_on) {
    turn_light_on();
  } else {
    turn_light_off();
  }
  
  delay(2);
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    client.setTimeout(20);
    //Serial.println("new client");
    while (client.connected()) {
      if (client.available()) {
        // send a standard http response header
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Cache-Control: no-cache");
        client.println("Connection: close");
        
        client.println();
        
        while(client.available()) {
          delay(3);  
          //Serial.println("READ");
          string = client.readString();
          //Serial.println(string);

          
          

  
          if(string.indexOf(" /door_on") != -1) {
            door_on = true;
            turn_door_on();
            //Serial.print("door on");
            client.println("door on");
            break;
          } else if(string.indexOf(" /door_off") != -1) {
            door_on = false;
            turn_door_off();
            //Serial.println("door off");
            client.println("door off");
            break;
          }
          if(string.indexOf(" /light_on") != -1) {
            light_on = true;
            turn_light_on();
            //Serial.println("light on");
            client.println("light on");
            break;
          } else if(string.indexOf(" /light_off") != -1) {
            light_on = false;
            turn_light_off();
            //Serial.println("light off");
            client.println("light off");
            break;
          }

          if(string.indexOf("keep-alive") != -1) {
            break;
          }
        }

        while(client.available()) {
          client.read();
        }
        //Serial.println("success");  
        client.println("success");
        client.println();
        break;
      }
     
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    //Serial.println("client disconnected");
  }
}

void turn_door_on() {
  digitalWrite(DOOR_PIN, LOW);
}
void turn_door_off() {
  digitalWrite(DOOR_PIN, HIGH);
}

void turn_light_on() {
  digitalWrite(LIGHT_PIN, HIGH);
}

void turn_light_off() {
  digitalWrite(LIGHT_PIN, LOW);
}


