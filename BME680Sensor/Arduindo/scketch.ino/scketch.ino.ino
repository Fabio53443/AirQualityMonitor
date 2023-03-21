#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <ESP8266WiFi.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

const char* ssid = "FASTWEB-B80DEF";
const char* password = "C77H1JCWCR";
const char* serverAddress = "http://192.168.1.171:8000"; // Replace with your server address
WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, serverAddress);

void connectToWiFi() {
  //Connect to WiFi Network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to WiFi");
  Serial.println("...");
  WiFi.begin(ssid, password);
  int retries = 0;
  while ((WiFi.status() != WL_CONNECTED) && (retries < 15)) {
    retries++;
    delay(500);
    Serial.print(".");
  }
  if (retries > 14) {
    Serial.println(F("WiFi connection FAILED"));
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("WiFi connected!"));
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
  Serial.println(F("Setup ready"));
}

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  connectToWiFi();
}

void loop() {
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  float temperature = bme.temperature;
  float pressure = bme.pressure / 100.0;
  float altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  float humidity = bme.humidity;
  float gas = bme.gas_resistance / 1000.0;
  double dewPoint = dewPointFast(temperature, humidity);

  // Create a JSON object with the sensor data
  DynamicJsonDocument doc(1024);
  doc["timestamp"] = millis(); // Add a timestamp
  doc["temperature"] = temperature;
  doc["pressure"] = pressure;
  doc["altitude"] = altitude;
  doc["humidity"] = humidity;
  doc["gas"] = gas;
  doc["dew_point"] = dewPoint;

  // Serialize the JSON object into a string
  String payload;
  serializeJson(doc, payload);
  Serial.println(payload);

  // Send a POST request to the server with the JSON payload
  httpClient.beginRequest();
  httpClient.post("/");
  httpClient.sendHeader("Content-Type", "application/json");
  httpClient.sendHeader("Content-Length", String(payload.length()));
  httpClient.sendHeader("Connection", "close");
  httpClient.println();
  httpClient.print(payload);
  httpClient.endRequest();

  delay(2000);
}
 
double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity * 0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}
