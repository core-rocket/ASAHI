var request = new XMLHttpRequest();
var acc_ctx = document.getElementById('acc-chart').getContext('2d');
var gyro_ctx= document.getElementById('gyro-chart').getContext('2d');
var temp_ctx= document.getElementById('temperature-chart').getContext('2d');
var gps_ctx = document.getElementById('gps-chart').getContext('2d');

var last_acc_time = 0.0;
var last_gyro_time= 0.0;
var last_temp_time= 0.0;
var last_gps_time = 0.0;

var lat_min	= 0.0;
var lat_max = 0.0;

function acc_onrefresh(chart){
	request.open("GET", "/data/acc", false);
	request.send(null);
	if(request.readyState != 4 || request.status != 200){
		last_acc_time = 0.0;
		return;
	}
	var data = JSON.parse(request.responseText);

	var dataset = chart.config.data.datasets;
	for(var i=0;i<Object.keys(data).length;i++){
		const client_time = Date.now();
		if(last_acc_time >= data[i].time) continue;
		last_acc_time = data[i].time;
		dataset[0].data.push({ x: client_time, y: data[i].x });
		dataset[1].data.push({ x: client_time, y: data[i].y });
		dataset[2].data.push({ x: client_time, y: data[i].z });
	}
	document.getElementById("acc_time").innerHTML = "<h4>time: " + last_acc_time + "</h4>";
}

function gyro_onrefresh(chart){
	request.open("GET", "/data/gyro", false);
	request.send(null);
	if(request.readyState != 4 || request.status != 200){
		last_gyro_time = 0.0;
		return;
	}
	var data = JSON.parse(request.responseText);

	var dataset = chart.config.data.datasets;
	for(var i=0;i<Object.keys(data).length;i++){
		const client_time = Date.now();
		if(last_gyro_time >= data[i].time) continue;
		last_gyro_time = data[i].time;
		dataset[0].data.push({ x: client_time, y: data[i].x });
		dataset[1].data.push({ x: client_time, y: data[i].y });
		dataset[2].data.push({ x: client_time, y: data[i].z });
	}
	document.getElementById("gyro_time").innerHTML = "<h4>time: " + last_gyro_time + "</h4>";
}

function temp_onrefresh(chart){
	request.open("GET", "/data/temperature", false);
	request.send(null);
	if(request.readyState != 4 || request.status != 200){
		last_temp_time = 0.0;
		return;
	}
	var data = JSON.parse(request.responseText);
	var dataset = chart.config.data.datasets;
	for(var i =0;i<Object.keys(data).length;i++){
		const client_time = Date.now();
		if(last_temp_time >= data[i].time) continue;
		last_temp_time = data[i].time;
		dataset[0].data.push({ x: client_time, y: data[i].bus });
	}
	document.getElementById("temperature").innerHTML = "<h4>time: " + last_temp_time + "</h4>";
}

function gps_onrefresh(chart){
	request.open("GET", "/data/gps", false);
	request.send(null);
	if(request.readyState != 4 || request.status != 200){
		last_gps_time = 0.0;
		return;
	}
	var data = JSON.parse(request.responseText);
	var lat = parseFloat(data.lat) / 100;
	var lng = parseFloat(data.lng) / 100;
	if(lat_min == 0.0 || lat_min > lat){
		lat_min = lat;
	}
	if(lat_max == 0.0 || lat_max < lat){
		lat_max = lat;
	}
//	var setting = chart.config.options.scale.xAxes.ticks;
//	setting.suggestedMin = lat_min;
//	setting.suggestedMax = lat_max;

	var dataset = chart.config.data.datasets;
	document.getElementById("gps_info").innerHTML =
		"<h4>internal time: " + data.time + ", UTC: " + data.utc + "</h4>" +
		"<h4>lat: " + lat + ", lng: " + lng + "</h4>";

	if(lat == 0.0 || lng == 0.0)
		return;

	dataset[0].data.push({ x: lat, y: lng });
	chart.update();
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

var temp_chart = new Chart(temp_ctx, {
	type: 'line',
	data: {
		datasets: [{
			label: "bus",
			borderColor: 'rgba(200, 0, 0)',
			fill: false,
			data: []
		}, {
			label: "mission",
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
					refresh: 100,
					delay: 0,
					onRefresh: temp_onrefresh,
				}
			}],
			yAxes: [{
				ticks: {
					min: 25,
					max: 50,
				}
			}]
		}
	},
});

var gps_chart = new Chart(gps_ctx, {
	type: 'scatter',
	data: {
		datasets: [{
			label: "GPS",
			borderColor: 'rgba(200, 0, 0)',
			pointRadius: 1,
			data: []
		}]
	},
	options: {
		scales: {
			xAxes: [{
				type: 'linear',
				position: 'bottom',
				ticks: {
					suggestedMin: 40.139,
					suggestedMax: 40.149,
					stepSize: 0.000001,
				}
			}],
			yAxes: [{
				ticks: {
					suggestedMin: 140.008,
					suggestedMax: 140.010,
					stepSize: 0.0001,
				}
			}]
		},
	},
});

setInterval("gps_onrefresh(gps_chart)", 100);
