#include<SoftwareSerial.h>
SoftwareSerial communicationSerial(3, 1);

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


String relayDeviceName = String("node4relay");
String deviceName = String("esp4relay");

ESP8266WebServer server(80);
WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  communicationSerial.begin(115200);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  WiFiManager wifiManager;
  wifiManager.autoConnect(deviceName.c_str());

  // Start the server
  server.begin();
  Serial.println("Server started");

  relay1_topic_out = String(relayDeviceName + "/" + relay1_topic_out);
  relay1_topic_in = String(relayDeviceName + "/" + relay1_topic_in);

  relay2_topic_out = String(relayDeviceName + "/" + relay2_topic_out);
  relay2_topic_in = String(relayDeviceName + "/" + relay2_topic_in);

  relay3_topic_out = String(relayDeviceName + "/" + relay3_topic_out);
  relay3_topic_in = String(relayDeviceName + "/" + relay3_topic_in);

  relay4_topic_out = String(relayDeviceName + "/" + relay4_topic_out);
  relay4_topic_in = String(relayDeviceName + "/" + relay4_topic_in);

  topicOut[0] = relay1_topic_out;
  topicOut[1] = relay2_topic_out;
  topicOut[2] = relay3_topic_out;
  topicOut[3] = relay4_topic_out;

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

void callback(char* topic, byte* payload, unsigned int length) {
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  boolean sendStatus = false;
  if (String(topic).indexOf("/in") > 0) {
    sendStatus = true;
  }

  int relayNum = 0;
  if (String(topic).equals(relay1_topic_in) || String(topic).equals(relay1_topic_out)) {
    relayNum = 1;
  }
  if (String(topic).equals(relay2_topic_in) || String(topic).equals(relay2_topic_out)) {
    relayNum = 2;
  }
  if (String(topic).equals(relay3_topic_in) || String(topic).equals(relay3_topic_out)) {
    relayNum = 3;
  }
  if (String(topic).equals(relay4_topic_in) || String(topic).equals(relay4_topic_out)) {
    relayNum = 4;
  }

  for (int i = 0; i < length; i++) {
    char receivedChar = (char)payload[i];
    String mqttCommand = String("c" + String(relayNum) + receivedChar);

    communicationSerial.println(mqttCommand.c_str());
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

      client.subscribe(relay1_topic_out.c_str());
      client.subscribe(relay2_topic_out.c_str());
      client.subscribe(relay3_topic_out.c_str());
      client.subscribe(relay4_topic_out.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  int relayNum = -1;
  int relayState = -1;

  String comMessage = getCommunicationSerialMessage();

  if (comMessage != "" && comMessage.charAt(0) == 'c') {
    relayNum = comMessage.charAt(1) - '0';
    relayState = comMessage.charAt(2) - '0';
  }

  if (relayNum > 0) {
    if (relayState > 0) {
      relayOn(relayNum);
    } else {
      relayOff(relayNum);
    }
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  server.handleClient();    //Handling of incoming requests
  delay(50);
}

String getCommunicationSerialMessage() {
  String content = "";
  char character;

  while (communicationSerial.available()) {
    character = communicationSerial.read();
    content.concat(character);
  }
  content.trim();
  return content;
}

void relayOff(int relayNum) {
  //client.publish(topicOut[relayNum - 1].c_str(), String(0).c_str(), true);
  client.publish(topicIn[relayNum - 1].c_str(), String(0).c_str(), true);
}

void relayOn(int relayNum) {
  //client.publish(topicOut[relayNum - 1].c_str(), String(1).c_str(), true);
  client.publish(topicIn[relayNum - 1].c_str(), String(1).c_str(), true);
}
