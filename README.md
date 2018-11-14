# esp8266-wifi-ledcube
### About
These days every device needs to have WiFi. So do LED-cubes like the DIY-kits (featuring the STC12C5A60S2) sold on eBay.
It's a first try to make the cube controllable by a responsive web interface.

### Features
* Responsive Web-Interface made with bootstrap
* Built-in simple animations in the arduino-project
* Turn cube full on, off, pause and resume animations
* Capability to load animations made with [DotMatrixJava](https://github.com/tomazas/DotMatrixJava) from SPIFFS
* SPIFF-data includes all examples from DotMatrixJava
* You get a wifi-enabled LED-cube
* Wi-Fi connection and remote control

### ToDo
* Code cleaning
* More animations
* Possibility to let the cube cycle animations "on it's own"
* Maybe make it somehow possible to create animations via the web interface
* More animations
* Upload animations made with DotMatrixJava via web interface and list them
* More animations
* Get rid of unneccessary bootstrap features (loading times of the interface)

### How to Use

Check these projects:
* [ledcube8x8x8](https://github.com/tomazas/ledcube8x8x8)
* [DotMatrixJava](https://github.com/tomazas/DotMatrixJava)
* [arduino-esp8266fs-plugin](https://github.com/esp8266/arduino-esp8266fs-plugin)

To get your WifiCube up and running, the firmware v2 out of the first project needs to be flashed to the STC microcontroller.
When that's done, the cube is controllable via UART with the ESP8266(and with your PC and the second project mentioned above). In order to do this, set up your adafruit mqtt username and key in the Arduino sketch, get the sketch out of this project on your ESP, flash the data-folder to the SPIFF, connect TX, RX, & GND from the ESP and enjoy.
For the first boot, it will raise a Soft-AP named "ESPCube" for you to use. The standard IP should be 192.168.4.1. You can set up Wi-Fi SSID & PSK and reboot the device. Once it connected to the Wi-Fi, the LED will blink three times and continue to connect MQTT server, it will then blink twice if the connection was succcessful.
To check the IP Address of your ESP, using any arp software to detect the IP or figure it out on your router.


Let me know if you are in trouble, but also if you have fun :)

### Remote Control
You need to setup your adafruit account and create a new dashboard. In the dashboard, create a new text field named "espcube". Futhermore, you can also ifttt account connecting to your adafruit account. After that, you can remote control the device with multiable services, such as Google Assistance and Siri.
