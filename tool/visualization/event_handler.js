
initChart = function () {
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
				responsive: false,
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
}
/*
Required for a chart object
			name: heap.id,
			legendText: global.SeenStrings.get(heap.nameId),
			type: 'line',
			dataPoints: []
*/
Visualization = new function() {
	this.MaxHorizontal = 500;
	this.Scale = 100;

	this.dataToDisplay = [];
	this.allocatorsMap = new Array();

	this.update = function() {
		    this.chart.update();
	}

	this.chartDataUpdate = function (id, time, size){
		lineData.datasets[id].data.push({x: time, y: size});

		if(lineData.datasets[id].data.length > Visualization.MaxHorizontal){
			lineData.datasets[id].data.splice(0,1);
		}
	}

	this.chartNewDataset = function(id, time) {
		var dataset = {
			data: []
		}
		dataset.data.push({x: time, y:0})
		lineData.datasets.push(dataset);
	}
}

ipcRenderer.on('new-allocation', function(event, data) {
  Visualization.chartDataUpdate(data.id,data.timestamp,data.size);
})

ipcRenderer.on('heap-created', function(event, data) {
  Visualization.chartNewDataset(data.id,data.timestamp);
})

setInterval(function() {
	Visualization.update()
},200);