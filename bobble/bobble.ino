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
#include <Adafruit_PN532.h>

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ1   (2)
#define PN532_RESET1 (3)  // Not connected by default on the NFC Shield
#define PN532_IRQ2   (5)
#define PN532_RESET2 (6)  // Not connected by default on the NFC Shield
#define PN532_IRQ3   (17)
#define PN532_RESET3 (16)  // Not connected by default on the NFC Shield
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};
IPAddress ip(192, 168, 1, 210);
IPAddress myDns(192, 168, 1, 1);

#define MAGNET_PIN1 (4)
#define MAGNET_PIN2 (7)
#define MAGNET_PIN3 (15)

uint8_t last_uid1[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t last_uidLength1 = 1;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
unsigned long last_millis1 = millis();

uint8_t last_uid2[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t last_uidLength2 = 1;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
unsigned long last_millis2 = millis();

uint8_t last_uid3[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t last_uidLength3 = 1;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
unsigned long last_millis3 = millis();

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

IPAddress etr_server(192,168,1,200);

// Or use this line for a breakout or shield with an I2C connection:
Adafruit_PN532 nfc1(PN532_IRQ1, PN532_RESET1);
Adafruit_PN532 nfc2(PN532_IRQ2, PN532_RESET2);
Adafruit_PN532 nfc3(PN532_IRQ3, PN532_RESET3);

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
// also change #define in Adafruit_PN532.cpp library file
   #define Serial SerialUSB
#endif

String string = String("");
bool magnet_state1 = false;
bool magnet_state2 = false;
bool magnet_state3 = false;


void setup(void) {
  #ifndef ESP8266
    long start_time = millis();
    while (!Serial && millis() - start_time < 2000); // for Leonardo/Micro/Zero
  #endif
  Serial.begin(115200);
  Serial.println("Started");

  nfc1.begin();
  nfc2.begin();
  nfc3.begin();

  uint32_t versiondata = nfc1.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board1");
    while (1); // halt
  }
  versiondata = nfc2.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board2");
    while (1); // halt
  }
  versiondata = nfc3.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board3");
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
  
  // configure board to read RFID tags
  nfc1.SAMConfig();
  nfc1.setPassiveActivationRetries(5);

  nfc2.SAMConfig();
  nfc2.setPassiveActivationRetries(5);

  nfc3.SAMConfig();
  nfc3.setPassiveActivationRetries(5);

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());

  pinMode(MAGNET_PIN1, INPUT);
  pinMode(MAGNET_PIN2, INPUT);
  pinMode(MAGNET_PIN3, INPUT);
  
  Serial.println("Waiting for an ISO14443A Card ...");
}

void loop(void) {
  uint8_t success;
  uint8_t uid1[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength1;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type
  uint8_t uid2[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength2;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  uint8_t uid3[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength3;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

  bool new_magnet_state1 = digitalRead(MAGNET_PIN1)==LOW;
  if(magnet_state1 != new_magnet_state1) {
    magnet_state1 = new_magnet_state1;
    httpRequest();
  }

  bool new_magnet_state2 = digitalRead(MAGNET_PIN2)==LOW;
  if(magnet_state2 != new_magnet_state2) {
    magnet_state2 = new_magnet_state2;
    httpRequest();
  }

  bool new_magnet_state3 = digitalRead(MAGNET_PIN3)==LOW;
  if(magnet_state3 != new_magnet_state3) {
    magnet_state3 = new_magnet_state3;
    httpRequest();
  }

  magnet_state1 = new_magnet_state1;
  magnet_state2 = new_magnet_state2;
  magnet_state3 = new_magnet_state3;
    
  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc1.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid1, &uidLength1);

  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength1, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc1.PrintHex(uid1, uidLength1);
    Serial.print("  UID Time: ");
    Serial.println(millis() - last_millis1);
    Serial.print("  Magnet: ");
    Serial.println(digitalRead(MAGNET_PIN1)==LOW);
    Serial.println("");

    for(int i = 0; i < 7; i++) {
      last_uid1[i] = uid1[i];
    }
    last_uidLength1 = uidLength1;
    last_millis1 = millis();
    
    Serial.flush();    
  }

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc2.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid2, &uidLength2);
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength2, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc2.PrintHex(uid2, uidLength2);
    Serial.print("  UID Time: ");
    Serial.println(millis() - last_millis2);
    Serial.print("  Magnet: ");
    Serial.println(digitalRead(MAGNET_PIN2)==LOW);
    Serial.println("");

    for(int i = 0; i < 7; i++) {
      last_uid2[i] = uid2[i];
    }
    last_uidLength2 = uidLength2;
    last_millis2 = millis();
    
    Serial.flush();    
  }

  // Wait for an NTAG203 card.  When one is found 'uid' will be populated with
  // the UID, and uidLength will indicate the size of the UUID (normally 7)
  success = nfc3.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid3, &uidLength3);

  
  
  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength3, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc3.PrintHex(uid3, uidLength3);
    Serial.print("  UID Time: ");
    Serial.println(millis() - last_millis3);
    Serial.print("  Magnet: ");
    Serial.println(digitalRead(MAGNET_PIN3)==LOW);
    Serial.println("");

    for(int i = 0; i < 7; i++) {
      last_uid3[i] = uid3[i];
    }
    last_uidLength3 = uidLength3;
    last_millis3 = millis();
    
    Serial.flush();    
  }

  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    while (client.connected()) {
      if (client.available()) {
        while(client.available()) {
          delay(3);  
          string = client.readString();
          Serial.println(string);
        }
        
        // send a standard http response header

        //client.println("POST /api/peripheral/update HTTP/1.1");
        //client.println("Host: etr.looklisten.com");
        client.println("HTTP/1.1 200 OK");
        client.println("Content-Type: application/json");
        client.println("Refresh: 1");
        client.println("Cache-Control: no-cache");
        
        client.println();
        client.println("{");
        client.println("\"identifier\":\"nfc_1\",");
        if(magnet_state1) {
          client.println("\"State_1\":\"0\",");
        } else {
          client.println("\"State_1\":\"1\",");
          client.print("\"Tag_1\":\"0x");
          for(int i = 0; i < last_uidLength1; i++) {
            client.print(last_uid1[i], HEX);
          }
          client.println("\",");
        }
        if(magnet_state2) {
          client.println("\"State_2\":\"0\",");
        } else {
          client.println("\"State_2\":\"1\",");
          client.print("\"Tag_2\":\"0x");
          for(int i = 0; i < last_uidLength2; i++) {
            client.print(last_uid2[i], HEX);
          }
          client.println("\",");
        }
        if(magnet_state3) {
          client.println("\"State_3\":\"0\"");
        } else {
          client.println("\"State_3\":\"1\",");
          client.print("\"Tag_3\":\"0x");
          for(int i = 0; i < last_uidLength3; i++) {
            client.print(last_uid3[i], HEX);
          }
          client.println("\"");
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
void httpRequest() {
  EthernetClient client;
  // close any connection before send a new request.
  // This will free the socket on the WiFi shield
  client.stop();

  // if there's a successful connection:
  if (client.connect(etr_server, 80)) {
    Serial.println("connected...");
    // send the HTTP GET request:
    client.println("POST /api/peripheral/update HTTP/1.1");
    client.println("Host: etr.looklisten.com");
    client.println("Content-Type: application/json");
    client.println("User-Agent: etr-device");
    //client.println("Connection: close");
    
    client.println();
    client.println("{");
    client.println("\"identifier\":\"nfc_1\",");
    if(magnet_state1) {
      client.println("\"State_1\":\"0\",");
    } else {
      client.println("\"State_1\":\"1\",");
      client.print("\"Tag_1\":\"0x");
      for(int i = 0; i < last_uidLength1; i++) {
        client.print(last_uid1[i], HEX);
      }
      client.println("\",");
    }
    if(magnet_state2) {
      client.println("\"State_2\":\"0\",");
    } else {
      client.println("\"State_2\":\"1\",");
      client.print("\"Tag_2\":\"0x");
      for(int i = 0; i < last_uidLength2; i++) {
        client.print(last_uid2[i], HEX);
      }
      client.println("\",");
    }
    if(magnet_state3) {
      client.println("\"State_3\":\"0\"");
    } else {
      client.println("\"State_3\":\"1\",");
      client.print("\"Tag_3\":\"0x");
      for(int i = 0; i < last_uidLength3; i++) {
        client.print(last_uid3[i], HEX);
      }
      client.println("\"");
    }
    client.println("}");
    client.println();
    
    delay(10);
    client.stop();
  } else {
    Serial.println("could not connect to server");
  }
}


