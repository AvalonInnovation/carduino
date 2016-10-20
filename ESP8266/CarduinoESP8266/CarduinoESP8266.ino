#include <WebSockets.h>
#include <WebSocketsClient.h>
#include <WebSocketsServer.h>

#include <WiFiManager.h>
#include <WebSocketsClient.h>
#include <ESP8266mDNS.h>

// Protocol: First line on serial port starts with ">>", followed by ID, followed by "<<\n"
// Transciever responds with OK\n or NOK\n. If NOK, client resends.
// after that, all serial data is sent transparently

// For now Telnet doesn't work for unknown reasons. The telnetServer.hasClient() call never 
// returns true

// There are two state machines - one for reading the serial port, and one for WiFi

//how many clients should be able to telnet to this ESP8266
#define MAX_SRV_CLIENTS 5
#define MAX_ID_LENGTH 128
#define SERIAL_BUFFER_SIZE 1024
#define DEBUG 0

#define min( a, b ) ( (a) < (b) ? (a) : (b))

// Avalon Innovation AB (www.avaloninnovation.com)
// Carduino 
// ESP8266 / WebSocket / UART transceiver

typedef enum {
  STATE_WIFI_CONNECT,              // 0
  STATE_WIFI_REGISTER_MDNS,        // 1
  STATE_WIFI_RESOLVE_MASTER,       // 2
  STATE_WIFI_TELNET_SETUP,         // 3
  STATE_WIFI_WEBSOCKET_CONNECT,    // 4
  STATE_WIFI_WEBSOCKET_CONNECTING, // 5
  STATE_WIFI_WEBSOCKET_RUNNING     
} WiFiState;

/* 
 *  static function prototypes
 */
static bool hasWiFiClient( void );
static void processNewTelnetConnections( void );

/*
 * global variables
 */
static WebSocketsClient webSocketClient;
// Telnet server
static WiFiServer telnetServer(23);
static WiFiClient serverClients[MAX_SRV_CLIENTS];

WiFiState wifiState;
// SerialState serialState;
IPAddress master;
int idLength;
char id[ 13 ]; // mac
int clientCount;
char *idDetails;

int serialBufferLength;
char serialBuffer[ SERIAL_BUFFER_SIZE ];

/*
 * start of code
 */
void setup() {
  String s;

  idLength = 0;
  id[idLength] = '\0';
  clientCount = 0;
  byte mac[6];
  
  WiFi.macAddress( mac );

  sprintf( id, "%02x%02x%02x%02x%02x%02x", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0] );
  
  // put your setup code here, to run once:
  // Serial.begin(115200);  
  Serial.begin(250000);  
  // debugPrintln("setup(): entering");

  // Serial.begin(250000);  // Per wants 250 kbps

    //WiFiManager
    //reset saved settings
    //wifiManager.resetSettings();
    
    //set custom ip for portal
    //wifiManager.setAPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration

    wifiState = STATE_WIFI_CONNECT;
    // serialState = STATE_SERIAL_WAIT_PREAMBLE_1;
    
    webSocketClient.onEvent(webSocketEvent);

    //if you get here you have connected to the WiFi
    // debugPrintln("setup(): leaving");
}

void debugPrintln( const char *string )
{
#if DEBUG
  Serial.println( string );
#endif
}

void webSocketEvent( WStype_t type, uint8_t * payload, size_t len ) 
{
    switch(type) 
    {
        case WStype_DISCONNECTED:
          if( (wifiState == STATE_WIFI_WEBSOCKET_RUNNING) || 
              (wifiState == STATE_WIFI_WEBSOCKET_CONNECTING) )
            wifiNextState( STATE_WIFI_WEBSOCKET_CONNECT );
            
          //  webSocketClient.begin("carduino-master.local", 8080);
          // Serial.printf("[WSc] Disconnected!\n");
          break;
            
        case WStype_CONNECTED:
          {
            String s;
#if DEBUG
            Serial.printf("[WSc] Connected to url: %s\n",  payload);
#endif
            // We may have to wait for serial ID here
#if 0
            s = "Identify: ";
            s = s + NAME;
            webSocketClient.sendTXT(s /* "Identify: " + NAME */ );
#endif
            wifiNextState( STATE_WIFI_WEBSOCKET_RUNNING );
          }
          break;
          
        case WStype_TEXT:
#if DEBUG
            Serial.printf("[WSc] get text: %s\n", payload);
#endif
      // send message to server
            Serial.print( (const char *) payload );
            
            // webSocketClient.sendTXT("message here");
            // webSocketClient.sendBIN( payload, len );
            break;
        case WStype_BIN:
#if DEBUG
            Serial.printf("[WSc] get binary lenght: %u\n", len);
            hexdump(payload, len);
#endif
            Serial.write( payload, len );
            // send data to server
            // webSocket.sendBIN(payload, lenght);
            break;
    }

}

void wifiNextState( WiFiState newState )
{
  WiFiClient client;

#if DEBUG
  Serial.printf( "Transition from wifi state %d to state %d\n", wifiState, newState );
#endif
  wifiState = newState;

  switch( wifiState )
  {
    case STATE_WIFI_TELNET_SETUP:
      telnetServer.begin();
      telnetServer.setNoDelay(true);

      // code below was to test telnet, but still doesn't work.
      // Does Websocket library screw things up?
#if 0
      Serial.println( "Waiting for telnet connection" );
      do
      {
        client = telnetServer.available();
        delay( 100 );
      } while( client == NULL );
      Serial.println( "Got telnet connection" );
#endif
      wifiNextState( STATE_WIFI_RESOLVE_MASTER );
      // wifiNextState( STATE_WIFI_WEBSOCKET_RUNNING );
      break;
  }
}
#if 0
static void serialRxLoop()
{
  size_t len = Serial.available();

  if( len > 0 )
  {
    uint8_t sbuf[len];
    int i;
    // int c = Serial.read();
    
    Serial.readBytes(sbuf, len);
    // String s;
         
    // s = c;

    if( state == STATE_SERIAL_RUNNING )
      webSocketClient.sendTXT( sbuf, len );

    for(i = 0; i < MAX_SRV_CLIENTS; i++)
    {
      if (serverClients[i] && serverClients[i].connected())
      {
        serverClients[i].write(sbuf, len);
        // delay(1); // why delay in original code?
      }
    }
  }
}
#endif
static void processNewTelnetConnections()
{
  // We get here, but never get hasClient
  if( telnetServer.hasClient())
  {
    int i;
#if DEBUG
    Serial.println( "processNewTelnetConnections: hasClient\n" );
#endif
    for(i = 0; i < MAX_SRV_CLIENTS; i++)
    {
      //find free/disconnected spot
      if (!serverClients[i] || !serverClients[i].connected())
      {
        if(serverClients[i] != NULL) 
          serverClients[i].stop();
          
        serverClients[i] = telnetServer.available();

        break;
        // Serial1.print("New client: "); Serial1.print(i);
        // continue;
      }
    }

    if( i == MAX_SRV_CLIENTS )
    {
      //no free/disconnected spot so reject
      WiFiClient serverClient = telnetServer.available();
      serverClient.stop();
    }
    
    // if( serialState == STATE_SERIAL_BUFFERING )
    //  serialState = STATE_SERIAL_SEND_BUFFER;
  } // end of if( telnetServer.hasClients() ) ...
}

void processTelnetRx()
{
  int i;
  
  //check clients for data
  for(i = 0; i < MAX_SRV_CLIENTS; i++){
    if (serverClients[i] && serverClients[i].connected()){
      if(serverClients[i].available()){
        //get data from the telnet client and push it to the UART
        while(serverClients[i].available()) Serial.write(serverClients[i].read());
      }
    }
  }  
}

void wifiDoStateMachine()
{
  // Serial.printf( "wifiDoStateMachine: entering(), state=%d\n", wifiState );
  
  switch( wifiState )
  {
    case STATE_WIFI_CONNECT:
      // wait until we have the ID string from the serial port before doing auto-connect, so
      // we know what name to use for the AP.
      {
        //Local intialization. Once its business is done, there is no need to keep it around
        WiFiManager wifiManager;

        //fetches ssid and pass from eeprom and tries to connect
        //if it does not connect it starts an access point with the specified name
        //here  "AutoConnectAP"
        //and goes into a blocking loop awaiting configuration
        // any ID goes here
        wifiManager.autoConnect( id );
      }
      wifiNextState( STATE_WIFI_REGISTER_MDNS );
      break;

    case STATE_WIFI_REGISTER_MDNS:
      // TODO: Use serial ID string as wifi name
      if( MDNS.begin( id ) )
        wifiNextState( STATE_WIFI_TELNET_SETUP );
      else
      {
        Serial.println("Error setting up MDNS responder!");
        delay(1000); // no, will kill serial? TODO: Fix
      }
      break;

    case STATE_WIFI_RESOLVE_MASTER:
      master = MDNS.queryHost( "carduino-master" );
      if( master[0] != 0 )
      {
        wifiNextState( STATE_WIFI_WEBSOCKET_CONNECT );
#if DEBUG
        Serial.printf( "Resolved master: %d.%d.%d.%d\n", master[0], master[1], master[2], master[3] );
#endif
      }
      else
      {
        debugPrintln( "Failed to resolve carduin-master.local" );
        // delay( 1000 );
        processNewTelnetConnections();
        processTelnetRx();
      }
      break;

     case STATE_WIFI_WEBSOCKET_CONNECT:
       webSocketClient.begin( master.toString(), 80 );
       wifiNextState( STATE_WIFI_WEBSOCKET_CONNECTING );
       break;

     case STATE_WIFI_WEBSOCKET_CONNECTING:
       delay( 1 ); // should not need this, TODO: try to remove
       processNewTelnetConnections();
       processTelnetRx();
       break;

     case STATE_WIFI_WEBSOCKET_RUNNING:
       // serialRxLoop();
       processNewTelnetConnections();
       processTelnetRx();
       break;

     default:
       Serial.printf( "ERROR: Invalid state %d\n", wifiState );
       break;
  }  
  webSocketClient.loop();

  // Serial.printf( "wifiDoStateMachine: leaving(), state=%d\n", wifiState );
}

void serialDoStateMachine()
{
  if( hasWiFiClient() )
  {
      int len = Serial.available();

      if( len > 0 )
      {
        uint8_t sbuf[len];
        int i;
    
        Serial.readBytes(sbuf, len);

        // debugPrintln( "serial running got data" );
        
        if( wifiState == STATE_WIFI_WEBSOCKET_RUNNING )
          webSocketClient.sendBIN( sbuf, len );

        for(i = 0; i < MAX_SRV_CLIENTS; i++)
        {
          if (serverClients[i] && serverClients[i].connected())
          {
            serverClients[i].write(sbuf, len);
            // delay(1); // why delay in original code?
          }
        }  
      }
  } // end of if( hasWiFiConnection() )
}

void serialToWiFi( char *buffer, size_t len )
{
  int i;
  
  if( wifiState == STATE_WIFI_WEBSOCKET_RUNNING )
    webSocketClient.sendBIN( (uint8_t *) buffer, len );

  for(i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (serverClients[i] && serverClients[i].connected())
    {
      serverClients[i].write(buffer, len);
      // delay(1); // why delay in original code?
    }
  }
  
}

static bool hasWiFiClient()
{
  int i;
  
  // I hate less than comparison with state, fix later
  if( wifiState < STATE_WIFI_WEBSOCKET_CONNECT )
    return false;

  if( wifiState == STATE_WIFI_WEBSOCKET_RUNNING )
    return true;

  for(i = 0; i < MAX_SRV_CLIENTS; i++)
  {
    if (serverClients[i] && serverClients[i].connected())
      return true;
  }

  return false;
}

// Todo: detect if we lose WiFi connectivity
void loop() 
{
  wifiDoStateMachine();
  serialDoStateMachine();
  // Todo: if button pushed, redo configuration
}


