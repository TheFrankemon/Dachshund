var app = angular.module('dachshund', ["chart.js"]);

app.controller('MainController', function($scope, socket) {
	$scope.devices = [];
	$scope.sendTime = sendTime;

	$scope.chartOnClick = chartOnClick;
  	$scope.chartOptions = {
  		animation: false,
  		responsive: true,
  		scales: {
            xAxes: [{
                display: true,
                scaleLabel: {
                    display: true,
                    labelString: 'Datetime'
                }
            }],
            yAxes: [{
                display: true,
                scaleLabel: {
                    display: true,
                    labelString: 'Value'
                }
            }]
        }
  	}

  	var defaultDashedDatasetOverride = {
        type: 'line',
        fill: false
        //borderDash: [5, 5]
  	};

	socket.on('device data', function(device) {
		console.log(device);
		updateDeviceInfo(device);
	});

	function sendTime(id, time) {
		socket.emit('update time', {
			'ID': id,
			'time': time
		});
	}

	function updateDeviceInfo(receivedData) {
		var found = false;
		for (var i = 0; i < $scope.devices.length && !found; i++) {
			var device = $scope.devices[i];
			if (device.ID === receivedData.ID) {
				if (device.labels.length > 10) {
					device.labels.shift();
					console.log(device.labels);
					device.data.every(numbers => numbers.shift());
				}

				var fill_elements = [];
				for (var j = 0; j < device.labels.length; j++) {
					fill_elements.push(0);
				}

				device.labels.push(receivedData.datetime);
				for (var key in receivedData.data) {
					if (device.series.indexOf(key) == -1) {
						device.series.push(key);
						device.datasetOverride.push(defaultDashedDatasetOverride);
						device.data.push(fill_elements);
					}
				}

				for (var j = 0; j < device.series.length; j++) {
					if (receivedData.data[device.series[j]]) {
						device.data[j].push(receivedData.data[device.series[j]]);
					} else {
						device.data[j].push(0);
					}
				}

				found = true;
			}
		}

		if (!found) {
			var newDevice = {
				ID: receivedData.ID,
				timeToWait: 1000,
				labels: [receivedData.datetime],
				series: [],
				data: [],
				datasetOverride: []
			}
			for (var key in receivedData.data) {
				newDevice.series.push(key);
				newDevice.datasetOverride.push(defaultDashedDatasetOverride);
				newDevice.data.push([receivedData.data[key]]);
			}
			$scope.devices.push(newDevice);
			//console.log(new_device);
		}
	}

	function chartOnClick(points, evt) {
	    console.log(points, evt);
	};
});

app.factory('socket', ['$rootScope', function ($rootScope) {
  var socket = io();

  return {
    on: function (eventName, callback) {
      function wrapper() {
        var args = arguments;
        $rootScope.$apply(function () {
          callback.apply(socket, args);
        });
      }

      socket.on(eventName, wrapper);

      return function () {
        socket.removeListener(eventName, wrapper);
      };
    },

    emit: function (eventName, data, callback) {
      socket.emit(eventName, data, function () {
        var args = arguments;
        $rootScope.$apply(function () {
          if(callback) {
            callback.apply(socket, args);
          }
        });
      });
    }
  };
}]);
