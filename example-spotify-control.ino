// tag::imports[]

//Check out the documentation section on how to manage credentials like this
#include <wifiCredentials.h>
#include <offbeatIotCredentials.h>
//connect to Offbeat-IoT using websockets
#include <WebSocketsClient.h>
//using a timer to synchronize with spotify
#include <SimpleTimer.h>
// end::imports[]

WebSocketsClient webSocketClient;
//#define USE_SERIAL TelnetStream
#define USE_SERIAL Serial

WiFiClient http;

const byte BUILTIN_LED1 = 1;     //GPIO0
const int ledPin = LED_BUILTIN;  // the number of the LED pin


//https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
//https://www.esp8266.com/viewtopic.php?p=92213
#define DHTPIN 0       //(rød gpio0, pin D3) (3,3 volt)
#define dhtPowerPin 0  //not used
#define echoPin 3      //rx = gpio3
#define trigPin 2      //(orange gpio2)

//for prod
char host[] = "www.offbeat-iot.com";
//char user[] = "qky5cvhccl";
//char offbeatIotUser[] = "NbAXTcSi";
char path[] = "/ws?device=C33I0PVk";
char deviceId[] = "C33I0PVk";

SimpleTimer timer;

void turnOnInternalLed() {
  digitalWrite(ledPin, LOW);
}
void turnOffInternalLed() {
  digitalWrite(ledPin, HIGH);
}

// a function to be executed periodically
void repeatMe() {
  USE_SERIAL.print("Uptime (s): ");
  USE_SERIAL.println(millis() / 1000);
}

float percent = 0;
long duration;        // variable for the duration of sound wave travel
float distance = -1;  // variable for the distance measurement
int measurementsNumber = 5;

boolean mailWasNotified = false;

float measureDistance() {
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  if (duration == 0) {
    USE_SERIAL.println("No pulse received from ultrasonic sensor");
    USE_SERIAL.flush();
  }
  // Calculating the distance
  return duration * 0.034 / 2.0;  // Speed of sound wave divided by 2 (go and back)
}

void sendCommand(int){
    // tag::determineDistance[]
    //TODO: this is the fun part
    // close::determineDistance[]
}

void calculateDistance() {

  float averageSum = 0;
  for (int i = 0; i < measurementsNumber; i++) {
    averageSum += measureDistance();
  }


  distance = averageSum / measurementsNumber;

  USE_SERIAL.print("Distance: ");
  USE_SERIAL.print(distance);
  USE_SERIAL.println(" cm");
  // tag::determineDistance[]
  //create weirdest remote control ever
  if (distance >= 0.0 || distance < 9.99) {
    sendCommand("next");
  } else if (distance >= 10.0 && distance < 19.99) {
    sendCommand("play");
  } else if (distance >= 20.0 && distance < 29.99) {
    sendCommand("volumeUp");
  } else if (distance >= 30.0 && distance < 39.99) {
    sendCommand("shuffle");
  } else if (distance >= 40.0 && distance < 49.99) {
    sendCommand("prev");
  } else if (distance >= 50.0 && distance < 59.99) {
    sendCommand("pause");
  } else if (distance >= 60.0 && distance < 69.99) {
    sendCommand("volumeDown");
  } 
  // end::determineDistance[]
}
else {
  //mailWasNotified = false;
}
}

char ipaddressString[256];

void reportIPAddress() {
  // send message to server when Connected
  IPAddress ipAddress = WiFi.localIP();

  sprintf(ipaddressString, "%d.%d.%d.%d", ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3]);
  USE_SERIAL.println(F("This is the ip: "));
  USE_SERIAL.println(ipaddressString);
  //webSocketClient.sendTXT(ipaddressString);
}

void updateStatus(char* deviceId, char* state, char* newState) {
  if (webSocketClient.isConnected()) {
    char powerstate[4];
    strcpy(powerstate, newState);
    char s[75];
    sprintf(s, "%s=%s,deviceId=%s", state, newState, deviceId);
    USE_SERIAL.print(F("Sending text: "));
    USE_SERIAL.println(s);
    webSocketClient.sendTXT(s);
  } else {
    USE_SERIAL.println(F("Not connected when updating status"));
  }
}

void reportProperties() {
  updateStatus(deviceId, "SSID", (char*)mySSID);
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  char* deviceIdFromMessage;
  char* command;
  char* commandValue;
  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED:
      {
        USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);
        timer.setTimeout(5000, reportIPAddress);
        reportProperties();
      }
      break;
      // tag::websocketEvent[]
    case WStype_TEXT:

      USE_SERIAL.printf("[WSc] get text: %s\n", payload);
      break;
      // end::websocketEvent[]

    case WStype_PING:
      // pong will be send automatically
      USE_SERIAL.println("[WSc] get ping");
      break;
    case WStype_PONG:
      // answer to a ping we send
      USE_SERIAL.println("[WSc] get pong");
      break;
    default:
      USE_SERIAL.print("[WSc] default:");
      //      USE_SERIAL.printf(type);
      USE_SERIAL.println(type);
      break;
  }
}


void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
    case ENC_TYPE_WEP:
      USE_SERIAL.println("WEP");
      break;
    case ENC_TYPE_TKIP:
      USE_SERIAL.println("WPA");
      break;
    case ENC_TYPE_CCMP:
      USE_SERIAL.println("WPA2");
      break;
    case ENC_TYPE_NONE:
      USE_SERIAL.println("None");
      break;
    case ENC_TYPE_AUTO:
      USE_SERIAL.println("Auto");
      break;
  }
}


void listNetworks() {
  // scan for nearby networks:
  USE_SERIAL.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    USE_SERIAL.println("Couldn't get a wifi connection");
    Serial.println("Couldn't get a wifi connection");
    Serial.flush();
  }

  // print the list of networks seen:
  USE_SERIAL.print("number of available networks:");
  USE_SERIAL.println(numSsid);

  // print the network number and name for each network found:
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    USE_SERIAL.print(thisNet);
    USE_SERIAL.print(") ");
    USE_SERIAL.print(WiFi.SSID(thisNet));
    USE_SERIAL.print("\tSignal: ");
    USE_SERIAL.print(WiFi.RSSI(thisNet));
    USE_SERIAL.print(" dBm");
    USE_SERIAL.print("\tEncryption: ");
    printEncryptionType(WiFi.encryptionType(thisNet));
  }
}

void restartIfNotConnected() {
  if (!WiFi.isConnected()) {
    ESP.restart();
  } else {
    USE_SERIAL.println("Still connected to wifi");
  }
}

void setup() {

  //https://www.instructables.com/How-to-use-the-ESP8266-01-pins/
  //This allows you to use RX as normal I/O, while still writing debug messages to Serial.
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);

  Serial.println(F("Setup"));
  Serial.flush();
  //setup hostname so you can see it in your internet router
  WiFi.hostname("example-spotify-control");

  Serial.flush();

  reportIPAddress();

  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
                             //GP2= dht22
                             //TXS=LED
                             //GP0=LDR1
                             //RXD=LDR2

  Serial.println("Connecting to websocket");
  Serial.flush();
  //turnOnInternalLed();
  // tag::connection[]
  webSocketClient.begin(host, 443, path);
  //Tell Offbeat-IoT that you'd like to receive data in json format
  webSocketClient.setExtraHeaders("Accept=application/json");
  webSocketClient.setAuthorization(offbeatIotUser, offbeatIotPassword);
  // end::connection[]

  webSocketClient.onEvent(webSocketEvent);
  //reconnect after 5 seconds if disconnected
  webSocketClient.setReconnectInterval(5000);
  //wait 15 seconds for pong message
  webSocketClient.enableHeartbeat(15000, 15000, 2);
  Serial.println("past connection");
  Serial.flush();

  timer.setInterval(60 * 1000, repeatMe);
  //calculate the distance every second
  timer.setInterval(1 * 1000, calculateDistance);
  Serial.println("setup done");
  Serial.flush();
}


void loop() {
  //do websocket stuff
  webSocketClient.loop();
  //do timer stuff
  timer.run();
}
