// var mdns = require('mdns');
var mdns = require('multicast-dns')()

var os = require('os');
var ws = require('ws');

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
var WebSocketServer = require('ws').Server

wss = new WebSocketServer({ port: 80 });

wss.on('connection', function connection(ws) {
	ws.on('message', function incoming(message) {
		console.log('received: %s', message);
	    });

	ws.send('something');
    });
