var app = angular.module('dachshund', []);

app.controller('MainController', function($scope, socket) {
	$scope.devices = [];
	$scope.sendTime = sendTime;

	socket.on('device data', function(device) {
		console.log(device);
		/*$scope.name = device.ID;
		$scope.data = JSON.stringify(device.data);*/
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
			if ($scope.devices[i].ID === receivedData.ID) {
				$scope.devices[i].data = receivedData.data;
				found = true;
			}
		}

		if (!found) {
			receivedData.timeToWait = 1000;
			$scope.devices.push(receivedData);
		}
	}
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
