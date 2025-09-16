#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// ----------- WiFi & MQTT ------------
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "broker.hivemq.com";  // free public MQTT broker

WiFiClient espClient;
PubSubClient client(espClient);

// ----------- Sensors & Relays ----------
#define DHTPIN 4       // DHT11 connected to GPIO4
#define DHTTYPE DHT11
#define RELAY1  5      // Light relay on GPIO5
#define RELAY2  18     // Fan relay on GPIO18
#define LDR_PIN 34     // LDR connected to GPIO34 (ADC)

DHT dht(DHTPIN, DHTTYPE);

unsigned long lastMsg = 0;
char msg[50];

void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* message, unsigned int length) {
  String msg = "";
  for (int i = 0; i < length; i++) {
    msg += (char)message[i];
  }

  if (String(topic) == "home/light") {
    if (msg == "ON") digitalWrite(RELAY1, LOW);
    else digitalWrite(RELAY1, HIGH);
  }
  if (String(topic) == "home/fan") {
    if (msg == "ON") digitalWrite(RELAY2, LOW);
    else digitalWrite(RELAY2, HIGH);
  }
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client")) {
      client.subscribe("home/light");
      client.subscribe("home/fan");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    int ldr = analogRead(LDR_PIN);

    String payload = "{\"temp\":" + String(t) + ",\"hum\":" + String(h) + ",\"light\":" + String(ldr) + "}";
    client.publish("home/sensors", payload.c_str());
  }
}
