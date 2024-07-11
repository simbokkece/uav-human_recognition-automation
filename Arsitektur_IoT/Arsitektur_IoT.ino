#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>
#include <Wire.h>
#include <SPI.h>

//#define WIFI_SSID "TELLO-98BC1C"
#define WIFI_SSID "IEELab_002"
//#define WIFI_SSID "CALVIN-Student"
//#define WIFI_PASSWORD "CITStudentsOnly"
#define WIFI_PASSWORD NULL
#define SOUND_VELOCITY 0.034

#define MQTT_HOST IPAddress(192, 168, 10, 10)
//#define MQTT_HOST IPAddress(10, 252, 240, 74)
#define MQTT_PORT 1883
#define MQTT_PUB_ULTRA "/esp/ultrasonic"
#define MQTT_PUB_LIGHT "/esp/light"

float ultrasonic;
float light;

const int trigPin = 12;
const int echoPin = 14;
long duration;
float distanceCm;
float distanceInch;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0;   // Stores last time temperature was published
const long interval = 10000;        // Interval at which to publish sensor readings

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(9600); // Starts the serial communication
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin, INPUT); // Sets the echoPin as an Input

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  //mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  // If your broker requires authentication (username and password), set them below
  //mqttClient.setCredentials("REPlACE_WITH_YOUR_USER", "REPLACE_WITH_YOUR_PASSWORD");
  connectToWifi();
}

void loop() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
  }
  float lightread = analogRead(A0);
  light = lightread * (5.0 / 1023.0);
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_VELOCITY/2;
  if (distanceCm <= 30){
    ultrasonic = 1;
  } else {
    ultrasonic = 0;
  }
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  Serial.println(light);
  
  // Publish an MQTT message on topic esp/ultrasonic
  uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_ULTRA, 1, true, String(ultrasonic).c_str());
  Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_ULTRA, packetIdPub1);
  Serial.printf("Message: %.2f \n", ultrasonic);
  delay(2000);
  
  // Publish an MQTT message on topic esp/light
  uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_LIGHT, 1, true, String(light).c_str());
  Serial.printf("Publishing on topic %s at QoS 1, packetId %i: ", MQTT_PUB_LIGHT, packetIdPub2);
  Serial.printf("Message: %.2f \n", light);
  delay(2000);
}
