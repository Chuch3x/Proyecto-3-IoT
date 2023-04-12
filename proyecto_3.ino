#include <WiFi.h>
#include <PubSubClient.h>
int sensorPin = 32;
int ledPin=33;

// Variable para almacenar la lectura del sensor
int sensorValue = 0;
const char* WIFI_SSID = "COMTECO-AP";
const char* WIFI_PASS = "wificf93";

const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_BROKER_PORT = 1883;

const char* MQTT_CLIENT_ID = "jose.sanchez.s@ucb.edu.bo";
const char* SUBSCRIBE_TOPIC_1 = "ucb/2756a/control";        // ESP32 SUBSCRIBE_TOPIC
const char* PUBLISH_TOPIC = "ucb/890e4/publish"; 
const char* SUBSCRIBE_TOPIC_2 = "ucb/2756a/on_off"; 

bool MANUAL_CONTROL=false;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void callback(const char* topic, byte* payload, unsigned int lenght) {
  String message;
  for (int i = 0; i < lenght; i++) {
    message += String((char) payload[i]);
  }
  if (String(topic) == SUBSCRIBE_TOPIC_2) {
    Serial.println("Message from topic " + String(topic) + ":" + message);
    if (message == "LED_ON") {
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(ledPin, LOW);
    }
  }
  if (String(topic) == SUBSCRIBE_TOPIC_1) {
    Serial.println("Message from topic " + String(topic) + ":" + message);
    if (message == "1") {
      MANUAL_CONTROL=true;
    } else {
      MANUAL_CONTROL=false;
    }
  }
  
}

void subscriptionToMQTT() {
    mqttClient.subscribe(SUBSCRIBE_TOPIC_1);
    mqttClient.subscribe(SUBSCRIBE_TOPIC_2);
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPIC_1));
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPIC_2));
}

boolean mqttClientConnect() {
  Serial.println("Connecting to " + String(MQTT_BROKER));
  if (mqttClient.connect(MQTT_CLIENT_ID)) {
    Serial.println("Connected to " + String(MQTT_BROKER));
    subscriptionToMQTT();
  } else {
    Serial.println("Cant't connecto to " + String(MQTT_BROKER));
  }
  return mqttClient.connected();
}

void controlLed(int valor){
  if(valor>=2000){
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}

void setWifiConnection() {
  Serial.println("Connecting to " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Can't connect to " + String(WIFI_SSID));
    delay(200); 
  }
  Serial.println("Connected to " + String(WIFI_SSID));
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
  setWifiConnection();
  //defining the parameters for the mqtt client
  mqttClient.setServer(MQTT_BROKER, MQTT_BROKER_PORT);
  mqttClient.setCallback(callback);
}

unsigned char counter = 0;
unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

void loop() {
  sensorValue = analogRead(sensorPin);
  Serial.print("LDR sensor reading: ");
  Serial.println(sensorValue);
  unsigned long now = millis();
  if (!mqttClient.connected()) {
    if (now - previousConnectMillis >= 2000) {
      previousConnectMillis = now;
      if (mqttClientConnect()) previousConnectMillis = 0;
      else delay(1000);
    }
  }else { // Connected to the MQTT Broker
    mqttClient.loop();
    delay(20);
    
    if (now - previousPublishMillis >= 10000) {
      previousPublishMillis = now;

      String message = "Hello from ESP32! " + String(sensorValue);
      
      mqttClient.publish(PUBLISH_TOPIC, message.c_str());
      Serial.print("Manual Control Value: ");
      Serial.println(MANUAL_CONTROL);
      if(MANUAL_CONTROL==false){
        controlLed(sensorValue);
      }
    }
  }
  delay(2000);
}