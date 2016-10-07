# ESP8266 Transcievers

The car and track boards have [Atmel](http://www.atmel.com) processors which
communicate via serial ports.

We will use ESP8266 WiFi boards to connect to the microcontroller serial port
and then communicate information over WiFi.

The ESP boards remember the SSID and password for the latest used WiFi network.
If they fail to connect, they will create a new WiFi network, named "carduino-car-01"
or similar. Joining that network and browsing to any web page will bring you
to the configuration web site, where you can enter SSID and password for the 
network that the cars should connect to.

The ESP modules will register host names with mdns (like "carduino-car-01.local").
They will then attempt to connect ot a websocket server running on the lost
"carduino-master", port 80 (TBD: Can we have both web server and websocket 
communication on the same port?).

The plan is to have a JSON-style protocol between the cars and the master.
