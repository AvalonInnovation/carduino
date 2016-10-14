// var mdns = require('mdns');
var mdns = require('multicast-dns')()

var os = require('os');
var ws = require('ws');
const readline = require('readline');

var WebSocketServer = require('ws').Server
var devicesByConnection = {};
var devicesByTypeID = {};

// get my IP address
var ifaces = os.networkInterfaces();

var ipAddress

Object.keys(ifaces).forEach(function (ifname) {
	var alias = 0;

	ifaces[ifname].some(function (iface) {
		if ('IPv4' !== iface.family || iface.internal !== false) {
		    // skip over internal (i.e. 127.0.0.1) and non-ipv4 addresses
		    return false;
		}

		if (alias >= 1) {
		    // this single interface has multiple ipv4 addresses
		    console.log(ifname + ':' + alias, iface.address);
                    ipAddress = iface.address;
		    return true;
		} else {
		    // this interface has only one ipv4 adress
                    ipAddress = iface.address;
		    console.log(ifname, iface.address);
                    return true;
		}
		++alias;
	    });
    });

console.log( "ip: " + ipAddress );

//    ad = mdns.createAdvertisement(mdns.tcp('http'), 80, { "host": "carduino-master.local",
//                                                            "name": "carduino-master",
//                                                            "domain": "local",
//							  "networkInterface": "172.16.1.135" /* String(ipAddress) */
//	},
//	function () {
//	    console.log(arguments);
//	}
//)
//    ;
// ad.start();

mdns.on('query', function(query) 
{
    var questions = query[ "questions" ];

    // console.log('got a query packet:', query)
    // console.log('  questions:', questions )
    
    questions.forEach( function answer( question ) 
    { 
      if( (question["name"] == "carduino-master.local") && 
	  (question["type"] == "A"))
      {
	  var response = { "name": "carduino-master.local",
			   "type": "A",
			   "ttl": 60,
			   "data": String( ipAddress )
	  }

	  mdns.respond( { answers: [ response ] } );
      }
    });
});

// create websocket server

wss = new WebSocketServer({ port: 80 });

wss.on('connection', function connection(ws) {
	device = { 'connection': ws };
	devicesByConnection[ ws ] = device;

	ws.on('message', function handleMessage(message) {
		parseNetworkMessage( device, message );
		console.log('received: %s', message);
	    });

	// ws.send('something');
    });

// read from standard input
const rl = readline.createInterface({
	input: process.stdin,
	output: process.stdout
    });

function parseDevice( data )
{
    var device = devicesByTypeID[ data ];
    if( device )
	rl.question( 'Enter data for ' + data + ": ", function( message ) { sendData( device, message ) } );
    else
    {
	console.log( "No such device" );
	rl.question('Send to what device (type-id):', parseDevice );
    }

    // ws.send( data );
}

function sendData( device, data )
{
    device[ 'connection' ].send( data );
}

function startsWith( string1, string2 )
{
    return string1.substring(0, string2.length) == string2
}

function parseNetworkMessage( device, message )
{
    var type = device[ 'type' ];

    if( type )
	console.log( type + "-" + device[ 'id' ] + ": '" + message + "'" );
    else
    {
	if( startsWith( message, "ID:" ) )
        {
	    var restOfString;
	    var charIndex;
	    var typeID;

	    restOfString = message.substring( 3 );
	    charIndex = restOfString.indexOf( '-' );

	    device[ 'type' ] = restOfString.substring( 0, charIndex );
	    restOfString = restOfString.substring( charIndex + 1 );

	    charIndex = restOfString.indexOf( ':' );
	    
	    // using trim to trim off newlines at the end
	    if( charIndex == -1 )
		device[ 'id' ] = restOfString.trim();
	    else
	    {
		device[ 'id' ] = restOfString.substring( 0, charIndex );
		device[ 'details' ] = restOfString.substring( charIndex + 1 ).trim();
	    }

	    typeID = device[ 'type' ] + "-" + device[ 'id' ];
	    devicesByTypeID[ typeID ] = device;
	    
	    console.log( "Parsed ID " + typeID );
	}
	else
	    console.log( "Strange, message '" + message + "' doesn't start with ID:" );
    }
}

rl.question('Send to what device (type-id):', parseDevice );

//(answer) => {
	// TODO: Log the answer in a database
//	console.log('Thank you for your valuable feedback:', answer);

//	rl.close();
//    });