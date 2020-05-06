
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

#define mqtt_server "192.168.1.10"
#define mqtt_port 1883

String relay1_topic_out = String("relay1/out");
String relay1_topic_in = String("relay1/in");

String relay2_topic_out = String("relay2/out");
String relay2_topic_in = String("relay2/in");

String relay3_topic_out = String("relay3/out");
String relay3_topic_in = String("relay3/in");

String relay4_topic_out = String("relay4/out");
String relay4_topic_in = String("relay4/in");

String topicOut[4];
String topicIn[4];
int relayPin[4];


String deviceName = String("node4relay");

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);


int buttonPin = A0; // A0
//970-1024,500-590,330-370,250-280

const int BUTTON1LOW = 970;
const int BUTTON1HIGH = 1025;
const int BUTTON2LOW = 500;
const int BUTTON2HIGH = 590;
const int BUTTON3LOW = 300;
const int BUTTON3HIGH = 400;
const int BUTTON4LOW = 200;
const int BUTTON4HIGH = 280;

// constants won't change. They're used here to
// set pin numbers:
const int ledPin =  13;      // the number of the LED pin for testing

const int BUTTON1 = 1;
const int BUTTON2 = 2;
const int BUTTON3 = 3;
const int BUTTON4 = 4;

const int relay1 = 5; //D1
const int relay2 = 4; //D2
const int relay3 = 14; //D5
const int relay4 = 12; //D6



void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT); // sets analog pin for input
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);

  digitalWrite(relay1, HIGH);
  digitalWrite(relay2, HIGH);
  digitalWrite(relay3, HIGH);
  digitalWrite(relay4, HIGH);

  WiFiManager wifiManager;
  wifiManager.autoConnect(deviceName.c_str());

  // Start the server
  server.begin();
  Serial.println("Server started");

  relay1_topic_out = String(deviceName + "/" + relay1_topic_out);
  relay1_topic_in = String(deviceName + "/" + relay1_topic_in);

  relay2_topic_out = String(deviceName + "/" + relay2_topic_out);
  relay2_topic_in = String(deviceName + "/" + relay2_topic_in);

  relay3_topic_out = String(deviceName + "/" + relay3_topic_out);
  relay3_topic_in = String(deviceName + "/" + relay3_topic_in);

  relay4_topic_out = String(deviceName + "/" + relay4_topic_out);
  relay4_topic_in = String(deviceName + "/" + relay4_topic_in);

  topicOut[0] = relay1_topic_out;
  topicOut[1] = relay2_topic_out;
  topicOut[2] = relay3_topic_out;
  topicOut[3] = relay4_topic_out;

  relayPin[0] = relay1;
  relayPin[1] = relay2;
  relayPin[2] = relay3;
  relayPin[3] = relay4;

  topicIn[0] = relay1_topic_in;
  topicIn[1] = relay2_topic_in;
  topicIn[2] = relay3_topic_in;
  topicIn[3] = relay4_topic_in;

  Serial.println(relay1_topic_out);
  Serial.println(relay1_topic_in);
  Serial.println(relay2_topic_out);
  Serial.println(relay2_topic_in);
  Serial.println(relay3_topic_out);
  Serial.println(relay3_topic_in);
  Serial.println(relay4_topic_out);
  Serial.println(relay4_topic_in);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  // Print the IP address
  Serial.println(WiFi.localIP());

  // Print the IP address
  Serial.print(WiFi.localIP());
  Serial.println("/app?relay=1&swich=1");
  Serial.print(WiFi.localIP());
  Serial.println("/app?relay=2&swich=1");

  server.on("/app", handleSpecificArg); //Associate the handler
}

void callback(char* topic, byte* payload, unsigned int length) {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  int relayNum = 0;
  if (String(topic).equals(relay1_topic_in)) {
    relayNum = 1;
  }
  if (String(topic).equals(relay2_topic_in)) {
    relayNum = 2;
  }
  if (String(topic).equals(relay3_topic_in)) {
    relayNum = 3;
  }
  if (String(topic).equals(relay4_topic_in)) {
    relayNum = 4;
  }

  for (int i = 0; i < length; i++) {
    char receivedChar = (char)payload[i];
    if (receivedChar == '1') {
      relayOn(relayNum);
    }
    if (receivedChar == '0') {
      relayOff(relayNum);
    }
  }

  digitalWrite(LED_BUILTIN, LOW);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    //if (client.connect(mqtt_client_name, mqtt_user, mqtt_pass)) {
    if (client.connect(String("ESP8266Client_" + deviceName).c_str())) {
      Serial.println("connected");
      client.subscribe(relay1_topic_in.c_str());
      client.subscribe(relay2_topic_in.c_str());
      client.subscribe(relay3_topic_in.c_str());
      client.subscribe(relay4_topic_in.c_str());

      //      client.subscribe(relay1_topic_out.c_str());
      //      client.subscribe(relay2_topic_out.c_str());
      //      client.subscribe(relay3_topic_out.c_str());
      //      client.subscribe(relay4_topic_out.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void handleSpecificArg() {

  String swich = "";

  if (server.arg("swich") != "") {   //Parameter not found
    swich = server.arg("swich");
    if (swich == "1") {
      Serial.print("off");
    } else if (swich == "0") {
      Serial.print("on");
    }
    Serial.println(server.arg("swich"));
  }

  server.send(200, "text/plain", swich);          //Returns the HTTP response
}


int lastButtonState = 0;
int lastButtonPressedState = 0;
void loop() {

  int buttonState = getButton();

  if (buttonState > 0 && buttonState != lastButtonState) {
    int relayNum = buttonState;
    switch (buttonState) {
      case BUTTON1:
        Serial.println("relay1 pressed");
        break;
      case BUTTON2:
        Serial.println("relay2 pressed");
        break;
      case BUTTON3:
        Serial.println("relay3 pressed");
        break;
      case BUTTON4:
        Serial.println("relay4 pressed");
        break;
    }

    if (digitalRead(relayPin[relayNum-1]) == LOW) {
      relayOff(relayNum);
    } else {
      relayOn(relayNum);
    }
  }

  lastButtonState = buttonState;

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  server.handleClient();    //Handling of incoming requests
  delay(50);
}

void relayOff(int relayNum) {
  Serial.println("off");
  digitalWrite(relayPin[relayNum-1], HIGH);
  client.publish(topicOut[relayNum - 1].c_str(), String(0).c_str(), true);
}

void relayOn(int relayNum) {
  Serial.println("on");
  digitalWrite(relayPin[relayNum-1], LOW);
  client.publish(topicOut[relayNum - 1].c_str(), String(1).c_str(), true);
}

int getButton() {
  int reading = analogRead(buttonPin);
  int tmpButton = 0;
  if (reading > BUTTON4LOW && reading < BUTTON4HIGH) {
    //Read switch 4
    tmpButton = BUTTON4;
  } else if (reading > BUTTON3LOW && reading < BUTTON3HIGH) {
    //Read switch 3
    tmpButton = BUTTON3;
  } else if (reading > BUTTON2LOW && reading < BUTTON2HIGH) {
    //Read switch 2
    tmpButton = BUTTON2;
  } else if (reading > BUTTON1LOW && reading < BUTTON1HIGH) {
    //Read switch 1
    tmpButton = BUTTON1;
  }
  return tmpButton;
}
