#include <FS.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "config.h"
#include "neopixel.h"
#include "state.h"

#define interruptStatePin 12

WiFiClient espClient;
PubSubClient mqttClient(espClient);

State state;
NeopixelInterface interface(&state);

bool stateOpen = false;

volatile byte interruptStateCounter = 1;

void connectWiFi() {
  delay(10);

  Serial.println("");

  WiFi.begin(wifi_ssid, wifi_password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    ESP.wdtFeed();
    Serial.print(".");
    interface.loop();
  }
  
  Serial.println("");
  Serial.print("WiFi connected ");
  Serial.println(WiFi.localIP());
  Serial.print("MQTT configured to ");
  Serial.print(mqtt_server);
  Serial.print(" with user ");
  Serial.println(mqtt_user);
}

void connectMQTT() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("vspace.one.state.open", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


// ##############################
// Interrupt handling
// ##############################

void interruptStateSR(){
  interruptStateCounter++;
}

void interruptStateHandler(){
  static bool last = false;

  stateOpen = digitalRead(interruptStatePin) == LOW;
  state.setLocalSpaceState(stateOpen ? SpaceState::SOPEN : SpaceState::SCLOSED);
  state.setRemoteSpaceState(stateOpen ? SpaceState::SOPEN : SpaceState::SCLOSED); //TODO query real remote state

  if (last != stateOpen){
    StaticJsonBuffer<200> jsonBuffer;

    JsonObject& root = jsonBuffer.createObject();
    root["status"] = "ok";

    JsonObject& data = root.createNestedObject("data");
    data["open"] = stateOpen?true:false;

    char message[200];
    root.printTo((char*)message, root.measureLength() + 1);

    mqttClient.publish("vspace/one/state/open", message);
    last = stateOpen;
  }

  interruptStateCounter = 0;
}

// ##############################
// Arduino "main" functions
// ##############################

void setup() {
  // State switch
  pinMode(interruptStatePin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(interruptStatePin), interruptStateSR, CHANGE);

  state.setConnectionState(ConnectionState::PRE_SERIAL);
  Serial.begin(115200);
  while (!Serial){
    ESP.wdtFeed();
    interface.loop();
  };

  state.setConnectionState(ConnectionState::PRE_WIFI);
  connectWiFi();

  mqttClient.setServer(mqtt_server, 1883);  
}

void loop() {
  if (!mqttClient.connected()) {
    state.setConnectionState(ConnectionState::NO_MQTT);
    connectMQTT();
  } else if (state.getConnectionState() != ConnectionState::CONN_OK) {
    state.setConnectionState(ConnectionState::CONN_OK);
  }
  mqttClient.loop();

  if (interruptStateCounter > 0){
    interruptStateHandler();
  }

  interface.loop();
  ESP.wdtFeed();
}
