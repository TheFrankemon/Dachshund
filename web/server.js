var express = require("express");
var app = express();
var http = require('http').Server(app);
var io = require("socket.io")(http);
var bodyParser = require('body-parser');

app.use(express.static(__dirname + '/www'));

app.use(bodyParser.json()); // support json encoded bodies
app.use(bodyParser.urlencoded({ extended: true })); // support encoded bodies

app.post("/", function(req, res) {
	//Curl cmd should be: curl -X POST "localhost:8080/?sensorid=2&userid=4&data=allo"
	var sensoridURL = req.query.sensorid;
	var useridURL = req.query.userid;
	var dataURL = req.query.data;
	console.log(sensoridURL);
	console.log(useridURL);
	console.log(dataURL);
	res.send("Received your POST :)");
});

app.post("/iot-device", function(req, res) {
	console.log(req.body);
	var iot_id = req.body.id;
	var data = [];
	i = 0;
	for (var key in req.body.data) {
		data[i] = {
			x: [req.body.datetime],
			y: [req.body.data[key]],
			name: key
		}
		i++;
	}

	console.log({
		"ID": iot_id,
		"data": data
	});

	io.emit("device data", {
		"ID": iot_id,
		"data": data
	});

	res.send("Received your POST :)");
})

io.on('connection', function(socket){
  console.log('a user connected');
});

//Starts the server, it listens on port 8080
var port = 8080;
http.listen(port);
console.log("Server running on:" + port);