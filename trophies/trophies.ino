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

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xEF
};
IPAddress ip(192, 168, 1, 178);

#define MAGNET_PIN1 (7)
#define MAGNET_PIN2 (8)

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);


void setup(void) {
  #ifndef ESP8266
    while (!Serial); // for Leonardo/Micro/Zero
  #endif
  Serial.begin(115200);
  Serial.println("Started");

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  pinMode(MAGNET_PIN1, INPUT);
  pinMode(MAGNET_PIN2, INPUT);
  
  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  
  // Display some basic information about the card
  Serial.println("Magnet status");
  Serial.print("  Magnet 1: ");
  Serial.println(digitalRead(MAGNET_PIN1)==LOW);
  Serial.print("  Magnet 2: ");
  Serial.println(digitalRead(MAGNET_PIN2)==LOW);
  Serial.println("");
  Serial.flush();    

  delay(200);

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        // send a standard http response header

        //client.println("POST /api/peripheral/update HTTP/1.1");
        //client.println("Host: etr.looklisten.com");
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Cache-Control: no-cache");
        
        client.println();
        client.println("{");
        client.println("\"identifier\":\"magnet\",");
        if(digitalRead(MAGNET_PIN1) == LOW) {
          client.println("\"State_1\":\"0\",");
        } else {
          client.println("\"State_1\":\"1\",");
        }
        if(digitalRead(MAGNET_PIN2) == LOW) {
          client.println("\"State_2\":\"0\",");
        } else {
          client.println("\"State_2\":\"1\",");
        }
        client.println("}");
        break;
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

