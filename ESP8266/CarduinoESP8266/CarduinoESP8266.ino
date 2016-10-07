#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>


#include <WiFiManager.h>
#include <WebSocketsClient.h>
#include <ESP8266mDNS.h>

#define NAME "Carduino-Track-01"
// Avalon Innovation AB (www.avaloninnovation.com)
// Carduino 
// ESP8266 / WebSocket / UART transceiver

WebSocketsClient webSocketClient;

typedef enum {
  STATE_CONNECT_WIFI,
  STATE_REGISTER_MDNS,
  STATE_RESOLVE_MASTER,
  STATE_WEBSOCKET_CONNECT,
  STATE_WEBSOCKET_CONNECTING,
  STATE_RUNNING
  } State;

State state;

IPAddress master;

void setup() {
  String s;
  
  // put your setup code here, to run once:
  Serial.begin(115200);

      //WiFiManager
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration

    state = STATE_CONNECT_WIFI;
    
#if 0
    wifiManager.autoConnect( NAME );
    //or use this for auto generated name ESP + ChipID
    //wifiManager.autoConnect();
    if (!MDNS.begin( NAME )) {
      Serial.println("Error setting up MDNS responder!");
      while(1) { 
        delay(1000);
      }
    }
    // resolve carduino-master.local
    for( int j = 0; j < 10; j++ )
    {
    int n = MDNS.queryService("gurka", "tcp");
      Serial.println("mDNS query done");
  if (n == 0) {
    Serial.println("no services found");
  }
  else {
    Serial.print(n);
    Serial.println(" service(s) found");
    for (int i = 0; i < n; ++i) {
      // Print details for each service found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(MDNS.hostname(i));
      Serial.print(" (");
      Serial.print(MDNS.IP(i));
      Serial.print(":");
      Serial.print(MDNS.port(i));
      Serial.println(")");
    }
  }
    }
#endif
  
  //  webSocketClient.begin("carduino-master.local", 8080);
    // webSocketClient.begin("192.168.0.204", 8080);
    webSocketClient.onEvent(webSocketEvent);

    //if you get here you have connected to the WiFi
    // Serial.println("connected...yeey :)");
}

int queryService2( const char *service, const char *protocol )
{
  
} 
void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {
    switch(type) {
        case WStype_DISCONNECTED:
          if( state == STATE_RUNNING )
            nextState( STATE_WEBSOCKET_CONNECT );
            
          //  webSocketClient.begin("carduino-master.local", 8080);
            Serial.printf("[WSc] Disconnected!\n");
            break;
            
        case WStype_CONNECTED:
          {
            String s;

            Serial.printf("[WSc] Connected to url: %s\n",  payload);

            s = "Identify: ";
            s = s + NAME;
            webSocketClient.sendTXT(s /* "Identify: " + NAME */ );

            nextState( STATE_RUNNING );
          }
          break;
          
        case WStype_TEXT:
            Serial.printf("[WSc] get text: %s\n", payload);

      // send message to server
            webSocketClient.sendTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[WSc] get binary lenght: %u\n", lenght);
            hexdump(payload, lenght);

            // send data to server
            // webSocket.sendBIN(payload, lenght);
            break;
    }

}

void nextState( State newState )
{
  Serial.printf( "Transition from state %d to state %d\n", state, newState );
  
  state = newState;  
}

// Todo: detect if we lose WiFi connectivity
void loop() {
  switch( state )
  {
    case STATE_CONNECT_WIFI:
      {
        //Local intialization. Once its business is done, there is no need to keep it around
        WiFiManager wifiManager;

        //fetches ssid and pass from eeprom and tries to connect
        //if it does not connect it starts an access point with the specified name
        //here  "AutoConnectAP"
        //and goes into a blocking loop awaiting configuration
        wifiManager.autoConnect( NAME );
      }
      nextState( STATE_REGISTER_MDNS );
      break;

    case STATE_REGISTER_MDNS:
      if( MDNS.begin( NAME ) )
        nextState( STATE_RESOLVE_MASTER );
      else
      {
        Serial.println("Error setting up MDNS responder!");
        delay(1000);
      }
      break;

    case STATE_RESOLVE_MASTER:
      
      master = MDNS.queryHost( "carduino-master" );
      if( master[0] != 0 )
        nextState( STATE_WEBSOCKET_CONNECT );
      else
      {
        Serial.println( "Failed to resolve carduin-master.local" );
        delay( 1000 );
      }
      break;

     case STATE_WEBSOCKET_CONNECT:
       webSocketClient.begin( master.toString(), 80 );
       nextState( STATE_WEBSOCKET_CONNECTING );
       break;

     case STATE_WEBSOCKET_CONNECTING:
       delay( 1 );
       break;

     case STATE_RUNNING:
       if( Serial.available() )
       {
         int c = Serial.read();
         String s;
         
         s = c;

         webSocketClient.sendTXT( s );
       }
       break;

     default:
       Serial.printf( "ERROR: Invalid state %d\n", state );
       break;
  }

  webSocketClient.loop();
  // Todo: if button pushed, redo configuration
  // Todo: Add name to settings
  // Todo: Make into library
}


