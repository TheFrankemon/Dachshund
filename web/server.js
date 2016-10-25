var express = require("express");
var app = express();
app.use(express.static(__dirname + '/'));

app.post("/", function(req, res) {
	//Curl cmd should be: curl -X POST "localhost:8080/?sensorid=2&userid=4&data=allo"
	var sensoridURL = req.query.sensorid;
	var useridURL = req.query.userid;
	var dataURL = req.query.data;
	console.log(sensoridURL);
	console.log(useridURL);
	console.log(dataURL);
	res.send("Recieved your POST :)");
});

//Starts the server, it listens on port 8080
var port = 8080;
app.listen(port);
console.log("Server running on:" + port);