//http://playground.arduino.cc/Code/WebClient

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xED }; // MAC address of the controller
IPAddress ip(192, 168, 24, 38); // Static IP address to use if the DHCP fails to assign
EthernetClient client; // Initialize the Ethernet client library. Client that will connect to the server

char server_address[100]; // Address of the server to connect
int server_port; // Port of the server to connect

// Data to send to the server
int sensor0;
int sensor1;
int sensor2;

int connected_to_server = 0; // 1 if client connected to the server successfully, else 0
int TTS = 1000;

String res_param;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Starting Ethernet connection...");
  
  // Start the Ethernet connection:
//  if (Ethernet.begin(mac) == 0) {
//    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Serial.println("Trying to connect with static IP...");
    Ethernet.begin(mac, ip);
//  } else {
    Serial.println(" connected");
//  }
  delay(1000); // give the Ethernet shield a second to initialize:
}

// Try to connect to a server with a given address and port
int connect_to(char address[], int port) {
  Serial.println("connecting...");
  // if you get a connection, report back via serial:
  if (client.connect(address, port)) {
    Serial.println("connected");
    connected_to_server = 1;
  } else {
    Serial.println("connection failed");
    connected_to_server = 0;
  }
  return connected_to_server;
}

// Stop the current ethernet client and change connected_to_server to 0
void disconnect_from_server() {
  Serial.println();
  Serial.println("disconnecting.");
  client.stop();
}

// Make and send a HTTP POST request with the sensor_id, user_id and counter variables as parameters
void post_to_server() {
  sensor0 = random(-20,21) * 5;
  sensor1 = random(-20,21) * 5;
  sensor2 = random(-20,21) * 5;
  // JSON construction
  String json = "{ \"id\" : \"My IoT\", \"datetime\" : \"2016-10-25 21:47:01\", \"data\" : { \"sensor0\" : ";
  json.concat(sensor0);
  json.concat(", \"sensor1\" : ");
  json.concat(sensor1);
  json.concat(", \"sensor2\" : ");
  json.concat(sensor2);
  json.concat(" }}");
  
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

// If there are incoming bytes available from the server, read them and print them
void receive_from_server() {
  char temp;
  bool flag = true;
  while ((client.available()) && flag) {
    char c1 = client.read();
    //Serial.print(c1);
    if ((temp == '\n') && (c1 == '\r')) {
      parse_body();
      !flag;
    } else {
      temp = c1;
    }
  }
}

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
  if (connected_to_server) { // If the server's connected, handle incoming bytes, send a POST request and increase the counter
    receive_from_server();
  
    if (!client.connected()) { // If the server's disconnected, stop the client
      disconnect_from_server();
      connect_to(server_address, server_port);
    }

    // send a POST request and handle incoming bytes
    post_to_server();
    receive_from_server();
    
    // Increase the counter and wait a second
    delay(TTS);
  } else { // If the server's not connected, wait for serial input for server address and port
    if (Serial.available()> 0) {
      read_server_info();
    }
  }
}

