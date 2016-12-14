const net = require('net');
const fs = require('fs');
const stream = require('stream');

var internal_socket;
// external_server
var external_server = net.createServer(function (external_socket) {
  var total_data_recieved = 0;
  var first_connect = true;
  var start_time = 0;
  var last_time = 0;

  var date = new Date();
  var time = String(date.getDay()) + "-" + String(date.getHours()) + "-" + String(date.getMinutes()) + "-" + String(date.getSeconds())
  var name =  "./database/test_" + time + ".db";
  //var unmodified_stream = fs.createWriteStream(name);

    //write input data to file
    external_socket.pipe(internal_socket);
    //external_socket.pipe(unmodified_stream);

    external_socket.on('close', function(data) {
      console.log("socket close")
    })

    external_socket.on('connect', function(data) {
      console.log("socket connected")
    })

    external_socket.on('data', function (data) {

      var start = performance.now();
      var daff = performance.now() - last_time;
      //console.log("data recieved, time since last: " + daff);
      if(data === undefined) {
        throw "undefined data recieved"
      }

      if (first_connect) {
        first_connect = false;
        sendToChart('first-data-recieved');
      }

      total_data_recieved += data.length;
      var diff = performance.now() - start;
      console.log("data ended with process time: " + diff);
      last_time = performance.now();
      sendToChart('event-done');
    })

    external_socket.on('drain', function(error) {
      console.log("socket drain")
    })

    external_socket.on('end', function(data) {
      var diff = performance.now() - start_time;
      console.log("connection ended in: " + diff);
      var data = {};
      data.channel = 'stream-end';
      ipcRenderer.send('to-internal-server', data)
    })

    external_socket.on('error', function(error) {
      console.log("socket error");
    })

    external_socket.on('lookup', function(error) {
      console.log("socket lookup")
    })

    external_socket.on('timeout', function(error) {
      console.log("socket timeout")
    })

})
.listen(8181);

external_server.on('close', function() {
  console.log("server closed at time: " + performance.now());
});

external_server.on('connection', function(socket) {
  console.log('connection to Memtrace::InitSocket (or open file) done');
});

external_server.on('listening', function() {
  require('dns').lookup(require('os').hostname(), function (err, add, fam) {
    sendToChart('server-init', {address: add, port: external_server.address().port})
  });
})

external_server.on('error', function(error) {
  console.log("server error: " + error)
})

ipcRenderer.on('please-connect', function(event, data) {
  internal_socket = net.createConnection(data.addr);
})