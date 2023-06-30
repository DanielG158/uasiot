#include <Wire.h>
#include <DHT.h>
#include <BH1750.h>
#include <WiFi.h>
#include <ThingsBoard.h>
#include <Adafruit_Sensor.h>
// Pin definitions
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
    ESP.restart();
    while (1);
  }
}

void loop() {
  static unsigned long lastTempTime = 0;
  static unsigned long lastHumidityTime = 0;
  static unsigned long lastLightTime = 0;
  static bool isConnected = false; // Track ThingsBoard connection status
  static unsigned long lastConnectionAttempt = 0; // Track last connection attempt

  // Check if ThingsBoard is connected
  if (!isConnected) {
    if (millis() - lastConnectionAttempt >= 2000) { // Retry every 2 seconds
      // Attempt to reconnect to ThingsBoard
      if (tb.connect(thingsboardServer, token)) {
        Serial.println("Connected to ThingsBoard");
        isConnected = true;
      } else {
        Serial.println("Failed to connect to ThingsBoard. Retrying...");
        lastConnectionAttempt = millis();
      }
    }
  }

  // Read temperature every 5 seconds
  if (millis() - lastTempTime >= 5000) {
    float temperature = dht.readTemperature();
    if (!isnan(temperature)) {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");
      tb.sendTelemetryFloat("temperature", temperature);
    } else {
      Serial.println("Error reading temperature!");
    }
    lastTempTime = millis();
  }

  // Read humidity every 5 seconds
  if (millis() - lastHumidityTime >= 5000) {
    float humidity = dht.readHumidity();
    if (!isnan(humidity)) {
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
      tb.sendTelemetryFloat("humidity", humidity);
    } else {
      Serial.println("Error reading humidity!");
    }
    lastHumidityTime = millis();
  }

  // Read light intensity every 5 seconds
  if (millis() - lastLightTime >= 5000) {
    uint16_t lux = lightMeter.readLightLevel();
    if (!isnan(lux)) {
      Serial.print("Light: ");
      Serial.print(lux);
      Serial.println(" lux");
      tb.sendTelemetryFloat("light", lux);
    } else {
      Serial.println("Error reading light level!");
    }
    lastLightTime = millis();
  }

  // Process ThingsBoard events
  tb.loop();

  // Check if disconnected from ThingsBoard
  if (isConnected && !tb.connected()) {
    Serial.println("Disconnected from ThingsBoard. Reconnecting...");
    isConnected = false;
    lastConnectionAttempt = millis();
  }
}
