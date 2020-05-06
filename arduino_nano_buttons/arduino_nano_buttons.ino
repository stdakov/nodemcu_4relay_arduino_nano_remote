#include<SoftwareSerial.h>
SoftwareSerial communicationSerial(0, 1);

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

const int BUTTON1 = 1;
const int BUTTON2 = 2;
const int BUTTON3 = 3;
const int BUTTON4 = 4;

const int relay1 = 4; //D4
const int relay2 = 5; //D5
const int relay3 = 6; //D6
const int relay4 = 7; //D7

// Variables will change:
int ledState = HIGH;         // the current state of the output pin



void setup() {
  Serial.begin(115200);
  communicationSerial.begin(115200);
  pinMode(buttonPin, INPUT); // sets analog pin for input

  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
}

int buttonState;             // the current reading from the input pin
int lastButtonState = 0;
int lastButtonPressedState = 0;

void loop() {
  int relayNum = -1;
  int relayPin = -1;
  int relayState = -1;

  String comMessage = getCommunicationSerialMessage();

  if (comMessage != "") {
    if (comMessage.charAt(0) == 'c') {
      relayNum = comMessage.charAt(1) - '0';
      relayState = comMessage.charAt(2) - '0';
      switch (relayNum) {
        case BUTTON1:
          relayPin = relay1;
          break;
        case BUTTON2:
          relayPin = relay2;
          break;
        case BUTTON3:
          relayPin = relay3;
          break;
        case BUTTON4:
          relayPin = relay4;
          break;
      }

    }
  }

  int buttonState = getButton();

  if (buttonState > 0 && buttonState != lastButtonState) {
    switch (buttonState) {
      case BUTTON1:
        relayPin = relay1;
        break;
      case BUTTON2:
        relayPin = relay2;
        break;
      case BUTTON3:
        relayPin = relay3;
        break;
      case BUTTON4:
        relayPin = relay4;
        break;
    }
    relayNum = buttonState;
    int currentButtonState = 0;
    if (digitalRead(relayPin) == LOW) {
      currentButtonState = 1;
    }
    String espCommand =  String("c" + String(relayNum) + String(currentButtonState));
    Serial.println(espCommand.c_str());
  }

  if (relayPin > 0) {
    if (relayState > 0) {
      digitalWrite(relayPin, HIGH);
    } else {
      digitalWrite(relayPin, LOW);
    }
  }

  lastButtonState = buttonState;

  delay(50);
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

String getCommunicationSerialMessage() {
  String content = "";
  char character;

  while (communicationSerial.available()) {
    character = communicationSerial.read();
    content.concat(character);
  }
  content.trim();
  if (content != "") {
    return content;
  }
}
