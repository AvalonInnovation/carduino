# carduino
Autonomous toy racing cars
Sponsored by [Avalon Innovation AB](www.avaloninnovation.com)

This project is about making hardware and software for autonomous 
Scalectrix-compatible cars and tracks, 

Currently, this repository is intended to contain all code and documentation for the project

The parts of the project are:

##Cars
* Electronics for the card boards
* Firmware for car boards
* Firmware in ESP8266 modules acting as traanscievers for the cars

##Track
Track boards are placed under the track, have powerline communication to the 
cards and IR transcievers that can communicate with the cars.

* Electronics for the car boards
* Firmware in track boards 
* Firmware for the ESP8266 boards acting as transcievers for the track boards

##Server
Software which communicates with the cars and tracks, and shows race scores 
and telematics for the cars.

* Software for the web server components
