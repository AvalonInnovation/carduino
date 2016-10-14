# SlotCarduino ESP8266 Transcievers

The car and track boards have [Atmel](http://www.atmel.com) processors which
communicate via serial ports.

We will use ESP8266 WiFi boards to connect to the microcontroller serial port
and then communicate information over WiFi.

The ESP expects a 250 kbps serial connection.

It waits for a string starting with `>>ID:` followed `<type>-<id>[:details]` 
followed by `<<`.

The ESP responds with `OK\n` or `NOK\n<error>\n`. If the device gets NOK or 
doesn't get OK in some timeout, the device should resend the ID string.

When the ESP gets the ID, it attempts to connect to a WiFi network with the 
previously settings for SSID and password. While it is doing this, it is 
buffering data from the serial port (at most 1024 bytes).

If it fails, it becomes an access point for a new WiFi network, called 
`<type>-<id>` as received from the device. Connecting to this network, you 
will get a web page where you can enter the SSID details.

When it succeeds in connecting to the WiFi network, it registers its host 
name as `<type>-<id>.local` with mdns.

It uses mdns to look up the host carduino-master.local with mdns. If it fails, 
it repeats this until it can resolve the host.

Then it creates a websocket connection to carduino-master.local, port 80. It 
will send `ID:<id>\n` as its first string on the websocket. 

At this point, any data received on the serial port is sent to the web socket,
and any data received on the web socket is sent to the serial port.

If it loses the websocket connection, it should attempt to reconnect, and buffer
serial data while it is doing so (not sure if this works now).

If it loses the WiFi, it should attempt to re-connect.

There are still bugs and crashes.

There is a node-js program in the `test/nodetest` which registers as 
`carduino-master.local` on mdns, and starts a websocket program, which can be
used to test the ESP connections.
