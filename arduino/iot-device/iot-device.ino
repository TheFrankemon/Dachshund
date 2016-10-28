//http://playground.arduino.cc/Code/WebClient

#include <SPI.h>
#include <Ethernet.h>
#include <Time.h>
#include <TimeLib.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xED }; // MAC address of the Arduino
IPAddress ip(192, 168, 24, 38); // Static IP address to use if the DHCP fails to assign an IP
EthernetClient client; // Client that connects to the server

char server_address[100]; // Input Address of the server to connect
int server_port; // Input Port of the server to connect

// Data to send to the server as hypothetic sensor values.
int sensor0;
int sensor1;
int sensor2;

int connected_to_server = 0; // 1 if the client connected successfully to the server, otherwise 0
int TTS = 1000; //Time To Send. Time interval between data sent to the server
String res_param; //Body recieved from the server as response, optionaly it carries a value that modifies the TTS

void setup() {
  // Open serial communications and wait for port to open
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.println("Starting Ethernet connection...");
  
  // Start the Ethernet connection with DHCP.  If it fails, connect with an static IP address.
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Serial.println("Trying to connect with static IP..."); 
    Ethernet.begin(mac, ip);
  } else {
    Serial.println(" connected");
  }
  delay(1000); // give the Ethernet shield a second to initialize.
}

// Try to connect to a server with a given address and port
int connect_to(char address[], int port) {
  Serial.println("connecting...");
  if (client.connect(address, port)) {
    Serial.println("connected");
    connected_to_server = 1;
  } else {
    Serial.println("connection failed");
    connected_to_server = 0;
  }
  return connected_to_server;
}

// Stop the current Ethernet client
void disconnect_from_server() {
  Serial.println();
  Serial.println("disconnecting.");
  connected_to_server = 0;
  client.stop();
}

// Send and print a HTTP POST request with the sensor values
void post_to_server() {
  String json = build_json();
  
  // Make a HTTP POST request:
  client.print("POST ");
  client.print("/iot-device");
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server_address);
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.println("Content-Type: application/json");
  client.print("Content-Length: ");
  client.println(json.length());
  client.println();  
  client.println(json);

  Serial.print("POST ");
  Serial.print("/iot-device");
  Serial.println(" HTTP/1.1");
  Serial.print("Host: ");
  Serial.println(server_address);
  Serial.println("User-Agent: Arduino/1.0");
  Serial.println("Connection: close");
  Serial.println("Content-Type: application/json");
  Serial.print("Content-Length: ");
  Serial.println(json.length());
  Serial.println();  
  Serial.println(json);
}

// Build an JSON string to be sent in the body of the HTTP POST request,
// Based on a format given: { “id” : “IOT_NAME“, “datetime” : “YYY-MM-DD HH:MM:SS”, “data” : {“sensor0” = XX , “sensor1” = XX, “sensor1” = XX}}
String build_json() {
  String js;
  time_t T = now();
  sensor0 = random(-20,21) * 5;
  sensor1 = random(-20,21) * 5;
  sensor2 = random(-20,21) * 5;
  // JSON construction
  js = "{ \"id\" : \"My IoT\", \"datetime\" : \"";
  js.concat(year(T));
  js.concat("-");
  js.concat(month(T));
  js.concat("-");
  js.concat(day(T));
  js.concat(" ");
  js.concat(hour(T));
  js.concat(":");
  js.concat(minute(T));
  js.concat(":");
  js.concat(second(T));
  js.concat("\", \"data\" : { \"sensor0\" : ");
  js.concat(sensor0);
  js.concat(", \"sensor1\" : ");
  js.concat(sensor1);
  js.concat(", \"sensor2\" : ");
  js.concat(sensor2);
  js.concat(" }}");
  return js;
}

// Read data sent from the server. Obtain the body from it and parse it.
void receive_from_server() {
  char temp;
  bool flag = true;
  while ((client.available()) && flag) {
    char c1 = client.read();
    if ((temp == '\n') && (c1 == '\r')) {
      parse_body();
      !flag;
    } else {
      temp = c1;
    }
  }
}

//Parses the body from server's response to update the TTS
void parse_body() {
  char c;
  res_param = "";
  while(client.available()) {
    c = client.read();
    if (!(c == '\n')) {
      Serial.print(c);
      res_param.concat(c);
    }
  }
  Serial.println();
  Serial.println(res_param);
  TTS = res_param.toInt();
}

// Read serial input for server address and port
void read_server_info() {
  String incoming_string;

  // Read address
  incoming_string = Serial.readString();
  incoming_string.toCharArray(server_address, incoming_string.length() + 1);

  // Print the address
  Serial.print("Server: ");
  Serial.println(server_address);

  while (Serial.available() <= 0); // Wait for another serial input
  server_port = Serial.parseInt(); // Read port number

  //Print the port
  Serial.print("Port: ");
  Serial.println(server_port);

  connect_to(server_address, server_port); // Try to make a connection with the given address and port
}

void loop() {
  // If the server is connected, send sensor data in POST requests and handle incoming data
  if (connected_to_server) {
    receive_from_server();

    // If the server's disconnected, stop the client
    if (!client.connected()) {
      disconnect_from_server();
      connect_to(server_address, server_port);
    }

    // send a POST request and handle incoming data
    post_to_server();
    receive_from_server();
    
    // Modify the TTS if requested, otherwise 1000 ms
    delay(TTS);
  } else {
    // If the server's not connected, wait for serial input for server address and port
    if (Serial.available()> 0) {
      read_server_info();
    }
  }
}

