//http://playground.arduino.cc/Code/WebClient

#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xDE, 0xED }; // MAC address of the controller
IPAddress ip(192, 168, 24, 38); // Static IP address to use if the DHCP fails to assign
EthernetClient client; // Initialize the Ethernet client library. Client that will connect to the server

// Data to send to the server
int counter = 0;
int sensor_id = 1;
int user_id = 5;

int connected_to_server = 0; // 1 if client connected to the server successfully, else 0

char server_address[100]; // Address of the server to connect
int server_port; // Port of the server to connect

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Starting Ethernet connection...");
  
  // Start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // try to congifure using IP address instead of DHCP:
    Serial.println("Trying to connect with static IP...");
    Ethernet.begin(mac, ip);
  } else {
    Serial.println(" connected");
  }
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
  char url[1024];
  sprintf(url, "/?sensorid=%d&userid=%d&data=%d", sensor_id, user_id, counter); // URL parameters
  
  // Make a HTTP POST request:
  client.print("POST ");
  client.print(url);
  client.println(" HTTP/1.1");
  client.print("Host: ");
  client.println(server_address);
  client.println("User-Agent: Arduino/1.0");
  client.println("Connection: close");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.println("Content-Length: 0");
  client.println();
}

// If there are incoming bytes available from the server, read them and print them
void receive_from_server() {
  while (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
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
    counter++;
    delay(1000);
  } else { // If the server's not connected, wait for serial input for server address and port
    if (Serial.available()> 0) {
      read_server_info();
    }
  }
}

