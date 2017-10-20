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

#define CONTROL_PIN1 (7)
#define CONTROL_PIN2 (8)


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xF1
};
IPAddress ip(192, 168, 0, 220);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);


bool is_open = false;
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

  pinMode(CONTROL_PIN1, OUTPUT);
  pinMode(CONTROL_PIN2, OUTPUT);

  close_drawer();

}

void loop(void) {
  if(is_open) {
    open_drawer();
  } else {
    close_drawer();
  }
  
  delay(2);
  
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    //Serial.println("new client");
    while (client.connected()) {
      if (client.available()) {
        while(client.available()) {
          delay(3);  
          string = client.readString();
          //Serial.println(string);
  
          if(string.indexOf(" /drawer_open") != -1) {
            is_open = true;
            //open_drawer();
            break;
          } else if(string.indexOf(" /drawer_close") != -1) {
            is_open = false;
            //close_drawer();
            break;
          }
          if(string.indexOf("keep-alive") != -1) {
            break;
          }
        }

        while(client.available()) {
          client.read();
        }

        //Serial.println(is_open);

        // send a standard http response header
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Cache-Control: no-cache");
        
        client.println();
        client.println("{");
        client.println("\"identifier\":\"drawer\",");

        if(is_open) {
          client.println("\"status\":\"open\"");
        } else {
          client.println("\"status\":\"closed\"");
        }
        client.println("}");
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

void open_drawer() {
  digitalWrite(CONTROL_PIN1, LOW);
  digitalWrite(CONTROL_PIN2, HIGH);
//  delay(1);
//  digitalWrite(CONTROL_PIN1, LOW);
//  digitalWrite(CONTROL_PIN2, LOW);
}

void close_drawer() {
  digitalWrite(CONTROL_PIN1, HIGH);
  digitalWrite(CONTROL_PIN2, LOW);
//  delay(2);
//  digitalWrite(CONTROL_PIN1, LOW);
//  digitalWrite(CONTROL_PIN2, LOW);
}


