#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "WifiConfig.h"
#include "FS.h"

/************************* Animation *********************************/
enum animation {
  rotTowerLeft,
  rotTowerRight,
  wave,
  rain,
  off,
  fullOn,
  randomAnimation,
  test,
  binaryFront,
  fence,
  signalLight,
  example,
  string,
  COUNT
};

/************ Global State (you don't need to change this!) ******************/
int currentAnimationStep = 0;
int currentAnimationMaxSteps = 4;

bool animating = true;
animation currentAnimation = wave;
String currentExample;
String currentlyAnimatedString;
int currentStringDelay;
int maxRaindropsPerLevel= 3;
int rainDelay = 0;
bool cycleAll = false;

#define FACTORY_RESET 0  // Reset button

/************************* HTTP Sever *********************************/
void setupHTTPServer() {
  
  server.serveStatic("/bootstrap.min.css", SPIFFS, "/bootstrap.min.css");
  
  server.serveStatic("/", SPIFFS, "/ledcube.html");
  
  server.on ( "/rotTowerLeft", []() {
    currentAnimationStep = 0;
    currentAnimation = rotTowerLeft;
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/rotTowerRight", []() {
    currentAnimationStep = 0;
    currentAnimation = rotTowerRight;
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/off", []() {
    currentAnimationStep = 0;
    currentAnimation = off;
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/fullOn", []() {
    currentAnimationStep = 0;
    currentAnimation = fullOn;
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/wave", []() {
    currentAnimationStep = 0;
    currentAnimation = wave;
    server.send ( 200, "text/html", "" );
  });
  
  server.on ( "/rain", []() {
    maxRaindropsPerLevel = server.arg("drops").toInt();
    rainDelay = server.arg("delay").toInt();
    currentAnimation = rain;
    server.send ( 200, "text/html", "" );
  });
  
  server.on ( "/fence", []() {
    currentAnimationStep = 0;
    currentAnimation = fence;
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/random", []() {
    currentAnimationStep = 0;
    currentAnimation = randomAnimation;
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/test", []() {
    currentAnimationStep = 0;
    currentAnimation = test;
    server.send ( 200, "text/html", "" );
  });
  
  server.on ( "/signalLight", []() {
    currentAnimationStep = 0;
    currentAnimation = signalLight;
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/binaryFront", []() {
    currentAnimationStep = 0;
    currentAnimation = binaryFront;
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/toggleCycle", []() {
    cycleAll = !cycleAll;
    server.send ( 200, "text/html", "" );
  });


  server.on ( "/string", []() {
    currentAnimation = string;
    currentAnimationStep = 0;
    currentlyAnimatedString = server.arg("text");
    currentStringDelay = server.arg("delay").toInt();
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/config", []() {
    clearEEPROM(0, 96);
    writeEEPROM(0, 32, server.arg("ssid").c_str());
    writeEEPROM(32, 96, server.arg("psk").c_str());
    //writeEEPROM(96, 128, server.arg("username").c_str());
    //writeEEPROM(128, 160, server.arg("key").c_str());
    server.send ( 200, "text/html", "" );
    delay(200);
    ESP.restart();    //reboot ESP
  });

  server.on ( "/example", []() {
    currentAnimationStep = 0;
    currentAnimation = example;
    currentExample = server.arg(0);
    server.send ( 200, "text/html", "" );
  });

  server.on ( "/continue", []() {
    animating = true;
    server.send ( 200, "text/html", "" );
  });
  server.on ( "/pause", []() {
    animating = false;
    server.send ( 200, "text/html", "" );
  });
  server.begin();

  Serial.println ( "HTTP server started" );
}

/************************* Initilize Device *********************************/
void initDevice() {
  Serial.begin ( 9600 );
  EEPROM.begin(512);
  SPIFFS.begin();
  randomSeed(analogRead(0));   // Random seed
  
  pinMode(LED, OUTPUT);
  //Turn on the LED
  digitalWrite(LED, 0);

  pinMode(FACTORY_RESET, INPUT_PULLUP);
  if (digitalRead(FACTORY_RESET) == 0) {
    clearEEPROM(0, 512);
    Serial.println();
    Serial.println("All settings have been cleaned, rebooting device...");
    ESP.restart();    //reboot ESP
  }

  
  if (strlen(readEEPROM(0, 32)) == 0) {
    sta_mode = false;
  } else {
    sta_mode = true;
  }
  
  //WiFi.printDiag(Serial);
  currentAnimation = rain ;
}

/************************* Setup *********************************/
void setup ( void ) {
  
  initDevice();

  setupHTTPServer();

  WiFi.softAP(softapssid);
  if (sta_mode) {
    connectWiFi();
  }
  mqtt.subscribe(&ledcube_sub);
}

void loop ( void ) {
  if (currentAnimationStep >= currentAnimationMaxSteps) {
    currentAnimationStep = 0;
    if (cycleAll) {
      currentAnimation = static_cast<animation>(static_cast<int>(currentAnimation+1));
      if (currentAnimation == COUNT)
        currentAnimation = static_cast<animation>(0);
    }
  }
    

  if (animating) {
    switch (currentAnimation) {
      case rotTowerLeft:
        rotateTower(true);
        break;
      case rotTowerRight:
        rotateTower(false);
        break;
      case fullOn:
        turnCubeFullOn();
        break;
      case wave:
        doWave();
        break;
      case rain:
        doRain(maxRaindropsPerLevel, rainDelay);
        break;
      case off:
        turnCubeOff();
        break;
      case randomAnimation:
        doRandom();
        break;
      case fence:
        doFence();
        break;
      case test:
        doTest();
        break;
      case binaryFront:
        doBinaryFront();
        break;
      case signalLight:
        doSignalLight();
        break;
      case example:
        doExample();
        break;
      case string:
        animateString(currentlyAnimatedString, currentStringDelay);
        //animateString("HELLO WORLD", currentStringDelay);
        break;
    }
    currentAnimationStep++;
  }
  server.handleClient();

  // MQTT
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(50))) {
    if (subscription == &ledcube_sub) {
      currentAnimationStep = 0;
      String str = (char *)ledcube_sub.lastread;
      Serial.println();
      Serial.print(F("MQTT Received: "));
      Serial.println(str);
      if (str == "on")
        currentAnimation = fullOn;
      else if (str == "off")
        currentAnimation = off;
      else if (str == "wave")
        currentAnimation = wave;
      else if (str == "rain")
        currentAnimation = rain;
      else if (str == "fence")
        currentAnimation = fence;
      else if (str == "tower")
        currentAnimation = rotTowerLeft;
      else {
        currentAnimation = string;
        currentlyAnimatedString = str;
        currentStringDelay = 0;
      }
    }
  }

  checkWiFi();
}
