# Home Assistant Remote Display

**Info and code provided for Educational use only**

This project makes use of an ESP8266 and a SSD1306 128x32 display to show information about a sensor. The sensor value is stored on Home Assistant ([https://github.com/home-assistant](https://github.com/home-assistant)) and the REST API is used to retrieve the information. 

**Please note that with the current code the display screen remains on always and this will shorten its lifespan !! (you will need to implement an off button or timeout etc.)**

The project is meant to work with RadonEye or any type of Radon measurements (in Bq/m^3) and provide a sort of warning when a set limit has been reached.

Make the changes to the code and test it out on your development board.

I will highlight below the changes that need to be made for you to get your system up and running: 

```
...
#define AUTHORZ "Bearer HOME_ASSISTANT_LONG_TOKEN" // set the Home Assistant Long Token here 
...
int RADON_WARNING_LIMIT = 145; // can update this
...
WiFiMulti.addAP("SSID", "WIFI_PSK"); // set the WIFI SSID and PASSPHRASE here
...
if (http.begin(client, "https://XXXXXXXXXXX/api/states/sensor.XXXXXX")) {  // HTTP, set the IP or host name of the Home Assistant and the sensor name
...

```

What you will need :

* The Wifi SSID and passphrase 
* You will need a Long Term Token from Home Assisntant.
* The URL to the Home Assistant REST API of the sensor you will be reading in. You can test this first using CURL check [https://developers.home-assistant.io/docs/api/rest/](https://developers.home-assistant.io/docs/api/rest/)

Display and ESP8266 Wiring:

```
GPIO4 (D2) = SDA
GPIO5 (D1) = SCL 
```