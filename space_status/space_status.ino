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

void connectWiFi() {
  delay(10);

  Serial.println("");

  WiFi.begin(wifi_ssid, wifi_password);

  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //ESP.wdtFeed();
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
    if (mqttClient.connect("vspace.one.state.open")) {
      Serial.println("connected");
      mqttClient.subscribe(mqtt_topic);
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

uint64_t time_to_update = 0;  //set to pow(2,64)-1 to prevent setting state on boot

IRAM_ATTR void interruptStateSR() {
  uint64_t time_to_update = millis() + 100;  //only after there have been no interrupts for 100ms will the state update
}

void update_status() {
  static bool last = false;

  stateOpen = digitalRead(interruptStatePin) == LOW;
  state.setLocalSpaceState(stateOpen ? SpaceState::SOPEN : SpaceState::SCLOSED);
  state.setRemoteSpaceState(stateOpen ? SpaceState::SOPEN : SpaceState::SCLOSED);

  if (last != stateOpen) {
    StaticJsonDocument<128> doc;

    doc["status"] = "ok";

    doc["open"] = stateOpen ? true : false;

    uint16_t len = measureJson(doc) + 1;
    char message[len];
    serializeJson(doc, message, len);

    mqttClient.publish(mqtt_topic, message);
    last = stateOpen;
  }

  time_to_update = 0xFFFFFFFFFFFFFFFF;  //just set it to the max value of an uint64_t so it won't run again
}

void mqttCallback(char* topic, byte* pl, uint16_t len) {
  if (strcmp(topic, mqtt_topic) == 0) {
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, pl, len);
    if (err == DeserializationError::Ok) {
      //if (doc["open"].is<bool>()) {
      state.setLocalSpaceState(doc["open"].as<bool>() ? SpaceState::SOPEN : SpaceState::SCLOSED);
      //}
    }
  }
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
  while (!Serial) {
    //ESP.wdtFeed();
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

  if (millis() > time_to_update) {
    update_status();
  }

  interface.loop();
  //ESP.wdtFeed();
}
