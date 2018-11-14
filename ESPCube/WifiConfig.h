// If you don't provide credentials, a SoftAP will be opened with the given password (can be empty)
//const char *ssid = "CJ Place";
//const char *password = "adadadadadadadadadad123456";

#include <EEPROM.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* EEPROM *********************************/
//  0 - 32: SSID
// 32 - 96: PSK
// 96 - 128: AIO_USERNAME
// 128 - 160: AIO_KEY

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "your_adafruit_id"
#define AIO_KEY         "you_adafruit_key"
const char *softapssid = "ESPCube";

#define LED 2    // LED for indicating WiFi Status

bool ap_mode = true;    //indicate if ESP needs AP mode
bool sta_mode = false;    //indicate if ESP needs STA mode

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

ESP8266WebServer server ( 80 );

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
//Adafruit_MQTT_Publish ledcube_pub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ledcube");

// Setup feeds for subscribing to changes.
Adafruit_MQTT_Subscribe ledcube_sub = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ledcube");


/*
 * Clear EEPROM, which store all settings
 */
void clearEEPROM(int st, int ed) {
    for (int i = st; i < ed; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    //Serial.println("EEPROM Cleared!");
    Serial.printf("clearEEPROM (%d, %d).\n", st, ed);
}

/*
 * Read content from certain part of the EEPROM
 */
char *readEEPROM(int st, int ed) {
    String str;
    int block;
    Serial.printf("readEEPROM (%d, %d):", st, ed);
    for (int i = st; i < ed; i++) {
        block = EEPROM.read(i);
        Serial.print(char(block));
        if (block == 0) {
            break;
        }
        str += char(block);
    }
    Serial.println();
    char *chr = new char[str.length() + 1];
    strcpy(chr, str.c_str());
    return chr;
}

/*
 * Write content to certain part of the EEPROM
 */
void writeEEPROM(int st, int ed, const char *chr) {
    for (int i = 0; i < strlen(chr); i++) {
      EEPROM.write(i + st, chr[i]);
    }
    EEPROM.commit();
    Serial.printf("writeEEPROM (%d, %d): %s\n", st, ed, chr);
}

/*
 * Blink the LED
 * pin: the pin the LED used
 * num: times to blink
 */
void blinkLED(int pin, int num, int interval) {
    //Set the lED to be off at first
    digitalWrite(pin, 1);
    delay(interval);
    for (int i = 0; i < num; i++) {
        digitalWrite(pin, 0);
        delay(interval);
        digitalWrite(pin, 1);
        delay(interval);
    }
}

/*
 * Generate an unique SSID based on MAC Address for ESP's Access Point
 */
char *getSSID() {
    String ssidStr = "ESP_";
    Serial.printf("MAC: %s\n", WiFi.softAPmacAddress().c_str());
    String macStr = WiFi.softAPmacAddress();
    macStr.replace(":", "");
    ssidStr += macStr.substring(6, 12);
    char *chr = new char[ssidStr.length() + 1];
    strcpy(chr, ssidStr.c_str());
    return chr;
}


/*
 * Start AP
 * ap_mode should be set to true because ESP is running an AP
 */
void enableAP() {
  ap_mode = true;
  Serial.println("Access Point is enabled.");
}

/*
 * Stop AP
 * ap_mode should be set to false because we do not need AP anymore
 */
void disableAP() {
  ap_mode = false;
  Serial.println("Access Point is disabled.");
}

/*
 * Connet to WiFi
 */
void connectWiFi() {
  WiFi.disconnect();
  Serial.println("Connecting to WiFi...");
  WiFi.begin ( readEEPROM(0, 32), readEEPROM(32, 96) );
}


// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void connectMQTT() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");
  
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  if (mqtt.connected()) {
    Serial.println("MQTT Connected!");
    blinkLED(LED, 2, 200);
  }
}


/*
 * Check WiFi Connection
 */
void checkWiFi() {
if (ap_mode && sta_mode) {
  digitalWrite(LED, 0);
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
  }
  else {
    // If ESP connected to the Home Wifi
    // 1. disaply its IP address
    // 2. set esp to work only on STA mode
    // 3. disable AP mode
    // 4. turn on the LED: Home Wifi is connected
    // 5. Connect to mqtt server
    Serial.println();
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    WiFi.mode(WIFI_STA);
    disableAP();
    //Blink the LED 3 times to notify users it has connected to the Home WiFi
    blinkLED(LED, 3, 200);
    //Start SocketIO, connect server
    connectMQTT();
    }
  } else if (ap_mode) {
    digitalWrite(LED, 0);
  } else if (sta_mode) {
    // If ESP somehow dropped the Home Wifi connection
    // 1. enale AP mode;
    // 2. turn off the LED: Home Wifi is disconnected
    digitalWrite(LED, 1);
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi dropped, reconnecting...");
      WiFi.mode(WIFI_AP_STA);
      enableAP();
      //Turn on the LED to notify users the connection is dropped
      digitalWrite(LED, 0);
       //end SocketIO server, disconnect server
    }
    else {
      connectMQTT();
    }
  }
}
