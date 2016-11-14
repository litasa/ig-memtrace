
total_data_handled = 0;
numEvents = 0;
// server
var server = require('net').createServer(function (socket) {
    console.log("connected");
    var total_data_recieved = 0;
	
	var ringBuffersize = 128*1024;
	var ringBuffer =  new CircularBuffer(ringBuffersize);
	
    socket.on('data', function (data) {
        var buffer = {
          data: Buffer.from(data,'hex'),
          index: 0,
          rollbackNeeded: false,
          rollback: 0
        };
		
        console.log("data recieved, Length: " + buffer.data.length);
        total_data_handled += data.length;
		
		ringBuffer.resetRead();
		var index = 0;
		do{
			index = ringBuffer.populate(buffer.data, index);
			
			do{
				if(!EventReader.oneEvent(ringBuffer)) {
					ringBuffer.read = ringBuffer.rollback;
				}
				else{
					numEvents++;
				}
			} while(ringBuffer.numRead < index);
			
		} while(index < buffer.data.length);

        console.log("done with buffer" + total_data_handled);
		console.log("Number of handled events: " + numEvents);
    })
})
.listen(8080);

function getTotalMemory(heapsalive)
{
  var ret = 0;
  for (var i = 0; i < heapsalive.size; i++) {
    if (heapsalive.has(i)) {
    for (var j  = 0; j < heapsalive.get(i).cores.length; j++) {
      ret += heapsalive.get(i).getMemoryUsage();
      }
    }
  }
  return ret;
}

server.on('close', function() {
  console.log("connection closed, hmm");
});

require('dns').lookup(require('os').hostname(), function (err, add, fam) {
  console.log('Address: '+ add + '\n socket: ' + server.address().port);
});
