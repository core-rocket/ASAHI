var request = new XMLHttpRequest();
var acc_ctx = document.getElementById('acc-chart').getContext('2d');

function acc_onrefresh(chart){
	request.open("GET", "/data/acc", false);
	request.send(null);
	if(request.readyState != 4 || request.status != 200)
		return;
	var data = JSON.parse(request.responseText);

	var datasets = chart.config.data.datasets;
	datasets[0].data.push({
		x: Date.now(),
		y: data.x
	});
	datasets[1].data.push({
		x: Date.now(),
		y: data.y
	});
	datasets[2].data.push({
		x: Date.now(),
		y: data.z
	});
}

var acc_chart = new Chart(acc_ctx, {
	type: 'line',
	data: {
		datasets: [{
			label: "x",
			borderColor: 'rgba(200, 0, 0)',
			data: []
		}, {
			label: "y",
			borderColor: 'rgba(0, 200, 0)',
			data: []
		}, {
			label: "z",
			borderColor: 'rgba(0, 0, 200)',
			data: []
		}]
	},
	options: {
		title: {
			display: true,
			text: "acc"
		},
		scales: {
			xAxes: [{
				type: 'realtime',
				realtime: {
					duration: 20000,
					refresh: 100,
					delay: 0,
					onRefresh: acc_onrefresh
				}
			}]
		},
	}
});
