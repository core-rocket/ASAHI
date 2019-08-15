var request = new XMLHttpRequest();
var acc_ctx = document.getElementById('acc-chart').getContext('2d');
var gyro_ctx= document.getElementById('gyro-chart').getContext('2d');

function acc_onrefresh(chart){
	request.open("GET", "/data/acc", false);
	request.send(null);
	if(request.readyState != 4 || request.status != 200)
		return;
	var data = JSON.parse(request.responseText);

	const client_time = Date.now();
	var datasets = chart.config.data.datasets;
	datasets[0].data.push({
		x: client_time,
		y: data.x
	});
	datasets[1].data.push({
		x: client_time,
		y: data.y
	});
	datasets[2].data.push({
		x: client_time,
		y: data.z
	});
}

function gyro_onrefresh(chart){
	request.open("GET", "/data/gyro", false);
	request.send(null);
	if(request.readyState != 4 || request.status != 200)
		return;
	var data = JSON.parse(request.responseText);

	const client_time = Date.now();
	var datasets = chart.config.data.datasets;
	datasets[0].data.push({
		x: client_time,
		y: data.x
	});
	datasets[1].data.push({
		x: client_time,
		y: data.y
	});
	datasets[2].data.push({
		x: client_time,
		y: data.z
	});
}

var acc_chart = new Chart(acc_ctx, {
	type: 'line',
	data: {
		datasets: [{
			label: "x",
			borderColor: 'rgba(200, 0, 0)',
			fill: false,
			data: []
		}, {
			label: "y",
			borderColor: 'rgba(0, 200, 0)',
			fill: false,
			data: []
		}, {
			label: "z",
			borderColor: 'rgba(0, 0, 200)',
			fill: false,
			data: []
		}]
	},
	options: {
		scales: {
			xAxes: [{
				type: 'realtime',
				realtime: {
					duration: 10000,
					refresh: 10,
					delay: 0,
					onRefresh: acc_onrefresh
				}
			}],
			yAxes: [{
				ticks: {
					min: -10,
					max: 10
				}
			}]
		},
	}
});

var gyro_chart = new Chart(gyro_ctx, {
	type: 'line',
	data: {
		datasets: [{
			label: "x",
			borderColor: 'rgba(200, 0, 0)',
			fill: false,
			data: []
		}, {
			label: "y",
			borderColor: 'rgba(0, 200, 0)',
			fill: false,
			data: []
		}, {
			label: "z",
			borderColor: 'rgba(0, 0, 200)',
			fill: false,
			data: []
		}]
	},
	options: {
		scales: {
			xAxes: [{
				type: 'realtime',
				realtime: {
					duration: 10000,
					refresh: 10,
					delay: 0,
					onRefresh: gyro_onrefresh
				}
			}],
			yAxes: [{
				ticks: {
					min: -1000,
					max: 1000
				}
			}]
		}
	},
});
