#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Adafruit_Sensor.h>
// Pin definitions
const int DHT_PIN = 4;        // DHT22 sensor
const int LIGHT_SENSOR_ADDRESS = 0x23;  // I2C address of light sensor

// Sensor type definitions
const int DHT_TYPE = DHT22;   // DHT11 sensor
const int LIGHT_SENSOR_TYPE = BH1750::ONE_TIME_HIGH_RES_MODE;  // 1 lx resolution

// Sensor objects
DHT dht(DHT_PIN, DHT_TYPE);    // DHT11 sensor
BH1750 lightMeter;             // I2C address 0x23

// WiFi and ThingsBoard credentials
const char* ssid = "B29 2.4GHz";
const char* password = "68205713";
const char* thingsboardServer = "demo.thingsboard.io";
const char* token = "7uuM8csCE6K0nVlC4mWv";

WiFiClient wifiClient;
ThingsBoard tb(wifiClient);

void setup() {
  Serial.begin(9600);                 // Initialize serial communication
  Wire.begin(21, 22);                    // SDA, SCL
  dht.begin();                           // Initialize temperature/humidity sensor
  lightMeter.begin();                    // Initialize light sensor
  delay(100);                            // Wait for sensor to initialize


  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Connect to ThingsBoard
  if (!tb.connect(thingsboardServer, token)) {
    Serial.println("Failed to connect to ThingsBoard");
    while (1);
  }
}

void loop() {
  static unsigned long lastTempTime = 0;       
  static unsigned long lastHumidityTime = 0;   
  static unsigned long lastLightTime = 0;      


  if (millis() - lastTempTime >= 1000) {
    float temperature = dht.readTemperature();  // Make the DHT11 read the temperature
    if (!isnan(temperature)) {                   // If the temperature is not a number, print the temperature
      Serial.print("Temperature: ");             // Print the temperature
      Serial.print(temperature);
      Serial.println(" C");
      
      // Send temperature data to ThingsBoard
      tb.sendTelemetryFloat("temperature", temperature);
    } else {
      Serial.println("Error reading temperature!");  // If the temperature is a number, print an error message
    }
    lastTempTime = millis();  // This is to make the temperature read every 7 seconds
  }


  if (millis() - lastHumidityTime >= 1000) {  
    float humidity = dht.readHumidity();      // Make the DHT11 read the humidity
    if (!isnan(humidity)) {                    // If the humidity is not a number, print the humidity
      Serial.print("Humidity: ");              // Print the humidity
      Serial.print(humidity);
      Serial.println(" %");
      
      // Send humidity data to ThingsBoard
      tb.sendTelemetryFloat("humidity", humidity);
    } else {
      Serial.println("Error reading humidity!");  // If the humidity is a number, print an error message
    }
    lastHumidityTime = millis();
  }


  if (millis() - lastLightTime >= 1000) {
    uint16_t lux = lightMeter.readLightLevel();  // To make the BH1750 read the light level
    if (!isnan(lux)) {                           // If the light level is not a number, print the light level
      Serial.print("Light: ");
      Serial.print(lux);
      Serial.println(" lux");
      // Send light level data to ThingsBoard
      tb.sendTelemetryFloat("light", lux);
    } else {
      Serial.println("Error reading light level!");  // If the light level is a number, print an error message
    }
    lastLightTime = millis();  
  }

  // Process ThingsBoard events
  tb.loop();
}