var app = angular.module('dachshund', []);

app.controller('MainController', function($scope) {
	var socket = io();
	
	socket.on('device data', function(device) {
		console.log(device);
		console.log(device.ID);
		$scope.name = device.ID;
		$scope.data = device.data;
	});
});

