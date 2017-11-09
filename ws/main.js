const WebSocket = require('ws');
const NATS = require('nats');

const wss = new WebSocket.Server({ port: 8080 });
const nats = NATS.connect();

subs = [];

nats.subscribe('s.http.pluspole.se', function(msg) {
    for (var i = 0; i < subs.length; i++) {
        try {
            subs[i].send(msg);
        } catch (e) {
            subs.splice(i, 1);
        }
    }
});

wss.on('connection', function connection(ws) {
    console.log("connect");
    var sid = nats.subscribe(ws.protocol, function(msg, reply, subject) {
        console.log("msg recv");
        ws.send(
            JSON.stringify({subject: subject, reply: reply, message: msg}),
            function(error) {
                if (error !== undefined) {
                    nats.unsubscribe(sid);
                    console.log("unsub: ", error);
                }
            }
        );
    });
});
