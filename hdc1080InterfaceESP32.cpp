#include <ArduinoJson.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "ClosedCube_HDC1080.h" // Include the HDC1080 library

#define wifi_ssid “" 
#define wifi_password “"
#define mqtt_server “" 

#define mqtt_topic "sensor/HDC1080"

WiFiClient espClient;
PubSubClient client(espClient);
ClosedCube_HDC1080 hdc1080; // Initialize HDC1080 sensor object

void setup() {
  Serial.begin(9600);
  Wire.begin(); // Initialize I2C communication
  hdc1080.begin(0x40); // HDC1080 address
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    
    if (client.connect("ESP32Client")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  float temperature = hdc1080.readTemperature();
  float humidity = hdc1080.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from HDC1080 RTH sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  //Create a JSON object
  DynamicJsonDocument jsonDoc(256);
  
  // Add temperature and humidity data to JSON object
  jsonDoc["temperature"] = temperature;
  jsonDoc["humidity"] = humidity;

  // Serialize JSON object to a string
  char jsonString[100];
  serializeJson(jsonDoc, jsonString);

  // Publish JSON-formatted sensor data
  client.publish(mqtt_topic, jsonString, true);

  delay(2000); 
}
