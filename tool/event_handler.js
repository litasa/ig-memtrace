
window.onload = function () {
		Chart.defaults.global.animation.duration = 0;
		var ctx = document.getElementById("myChart");
		lineData = {
			labels: [],
			datasets: [],
		};

		Visualization.chart = new Chart(ctx, {
			type: 'line',
			data: lineData,
			options: {
				responsive: true,
				animation: {
					duration: 0
				},
				tooltips: {
					enabled: true
				},
				scales: {
					xAxes: [{
						type: 'linear',
						position: 'bottom'
					}]
				},
				zoom: {
					enabled: true,
					drag: false,
					mode: 'x',
				},
				pan: {
					enabled: true,
					drag: true,
					mode: 'x'
				}
			}
		});
		count = 0;
		Visualization.chart.update();

			//update frequency

}
/*
Required for a chart object
			name: heap.id,
			legendText: global.SeenStrings.get(heap.nameId),
			type: 'line',
			dataPoints: []
*/
Visualization = new function() {
	this.MaxHorizontal = 1000;
	this.Scale = 100;

	this.dataToDisplay = [];
	this.allocatorsMap = new Array();

	this.update = function() {
		    this.chart.update();
	}

	this.addAllocator = function(alloc) {
		//TODO error checking
		this.allocatorsMap.push(alloc);
		alloc.cores = [];
		alloc.managedSize = 0;
		alloc.usedMemory = 0;

		this.chart.data.datasets.push({
				id: hexToInt(alloc.id),
				label: global.SeenStrings.get(alloc.nameId),
				fill: false,
				lineTension: 0,
				pointRadius: 4,
				steppedLine: false,
				//pointStyle: 'line',
				data: []
		});
	}

	var chartDataUpdate = function (allocator, core, time, size){

		//TODO Make sure that high values also get added
		allocator.usedMemory += size;
		//TODO Make sure that high values also get added
		core.usedMemory += size;

		lineData.datasets[allocator.id].data.push({x: time, y: core.usedMemory});

		if(lineData.datasets[allocator.id].data.length > Visualization.MaxHorizontal){
			lineData.datasets[allocator.id].data.splice(0,1);
		}
	}

	this.addCore = function(core) {
		//TODO error checking
		var allocator = getAllocator(core.id).value;
		allocator.cores.push(core);
		core.allocs = [];
		core.usedMemory = 0;
		allocator.managedSize += hexToInt(core.size);
		core.end = addBinToBin(core.start, core.size);
	}

	this.addAllocation = function(alloc) {
		var allocator = getAllocator(alloc.id).value;
		var core = getCore(allocator.cores,alloc.pointer).value;

		core.allocs.push(alloc);

		//TODO Use high value of timestamp as well
		var time = binToInt(alloc.head.timestamp) / binToInt(global.timerFrequency);

		//TODO use special container for visualization to be able to show different stuffs
		chartDataUpdate(allocator,core, time, hexToInt(alloc.size));
	}

	this.removeAllocation = function(alloc) {
		var allocator = getAllocator(alloc.id).value;
		var core = getCore(allocator.cores,alloc.pointer).value;
		var allocation = getAlloc(core.allocs,alloc.pointer);

		var time = binToInt(alloc.head.timestamp) / binToInt(global.timerFrequency);

		//negative size to remove
		chartDataUpdate(allocator,core, time, -hexToInt(allocation.value.size));

		core.allocs.splice(allocation.key,1);
	}

	var getAlloc = function(allocs, pointer){
		for(var i = 0; i < allocs.length; ++i) {
			if(binCompare(allocs[i].pointer,'equal',pointer)){
				return {
					key: i,
					value: allocs[i]}
			}
		}
		return null;
	}

	var getCore = function(cores,pointer) {
		for(var i = 0; i < cores.length; ++i){
			if(pointerInsideCore(pointer,cores[i])){
				return {
					index: i,
					value: cores[i]}
			}
		}
		return null;
	}

	var getAllocator = function(id){
		for(var i = 0; i < Visualization.allocatorsMap.length; ++i){
			var allocator = Visualization.allocatorsMap[i];
			if(allocator.id == id){
				return {
					index: i,
					value: allocator}
			}
		}
		return null;
	}
}