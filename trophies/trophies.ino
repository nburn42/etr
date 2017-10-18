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
  0xDE, 0xAD, 0xBE, 0xBF, 0xFE, 0x2F
};
IPAddress ip(192, 168, 0, 215);

#define MAGNET_PIN1 (7)
#define MAGNET_PIN2 (8)

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

IPAddress etr_server(192,168,0,200);


bool magnet_state1 = false;
bool magnet_state2 = false;

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
  

  httpRequest(true);
  httpRequest(false);
}


void loop(void) {
  
  bool new_magnet_state1 = digitalRead(MAGNET_PIN1)==LOW;
  if(magnet_state1 != new_magnet_state1) {
    magnet_state1 = new_magnet_state1;
    httpRequest(true);
  }

  bool new_magnet_state2 = digitalRead(MAGNET_PIN2)==LOW;
  if(magnet_state2 != new_magnet_state2) {
    magnet_state2 = new_magnet_state2;
    httpRequest(false);
  }

  magnet_state1 = new_magnet_state1;
  magnet_state2 = new_magnet_state2;

  delay(200);

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    while (client.connected()) {
      if (client.available()) {
        // send a standard http response header

        //client.println("POST /api/peripheral/update HTTP/1.1");
        //client.println("Host: etr.looklisten.com");
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Refresh: 1");
        client.println("Cache-Control: no-cache");
        
        client.println();
        client.println("{");
        if(magnet_state1) {
          client.println("\"magnet_1\":\"1\",");
        } else {
          client.println("\"magnet_1\":\"0\",");
        }
        if(magnet_state2) {
          client.println("\"magnet_2\":\"1\"");
        } else {
          client.println("\"magnet_2\":\"0\"");
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

// this method makes a HTTP connection to the server:
void httpRequest(bool use_magnet1) {
  EthernetClient client;
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(etr_server, 80)) {
    Serial.println("connected...");
    // send the HTTP GET request:
    client.println("POST /api/peripheral/update HTTP/1.1");
    client.println("Host: 192.168.0.200");
    client.println("Content-Type: application/json");
    client.println("cache-control: no-cache");
    client.println("Content-Length: 172");
   
    client.println();
    Serial.println();
    client.println("{");
    Serial.println("{");
    
    if(use_magnet1) {
      client.println("\t\"identifier\":\"magnet_1\",");
      Serial.println("\t\"identifier\":\"magnet_1\",");
      if(magnet_state1) {
        client.println("\t\"state\":\"1\"");
        Serial.println("\t\"state\":\"1\"");
      } else {
        client.println("\t\"state\":\"0\"");
        Serial.println("\t\"state\":\"0\"");
      }
    } else {
      client.println("\t\"identifier\":\"magnet_2\",");
      Serial.println("\t\"identifier\":\"magnet_2\",");
      if(magnet_state2) {
        client.println("\t\"state\":\"1\"");
        Serial.println("\t\"state\":\"1\"");
      } else {
        client.println("\t\"state\":\"0\"");
        Serial.println("\t\"state\":\"0\"");
      }
    }
    client.println("}");
    Serial.println("}");
    client.println();
    Serial.println();
    
    delay(500);

    while(client.available()) {
      delay(3);  
      String string = client.readString();
      Serial.println(string);
      
      
    }
    Serial.println("end");
    
    client.stop();
  } else {
    Serial.println("could not connect to server");
  }
}


