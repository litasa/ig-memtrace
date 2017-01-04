connected = false;
server = null;
var enc = require('..\\build\\Release\\Encryption');

window.onload = function() {
  initChart();
  console.log(enc.Decoder().printas());
  }

ipcRenderer.once('server-init', function(event, serverData) {
  server = {};
  server.address = serverData.address;
  server.port = serverData.port;

  console.log('address: ' + server.address + ' port: ' + server.port);
})

ipcRenderer.once('connection-established', function() {
  connected = true;
})

ipcRenderer.on('event-done', function(event, data) {
  console.log('data recieved: ')
})