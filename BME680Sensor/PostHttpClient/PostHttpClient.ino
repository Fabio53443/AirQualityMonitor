#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#define SERVER_IP "192.168.200.47:8000"

#ifndef STASSID
#define STASSID "Op"
#define STAPSK "12345678"
#endif

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C





void setup() {

  Serial.begin(115200);

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

  


  Serial.println();
  Serial.println();
  Serial.println();

  WiFi.begin(STASSID, STAPSK);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
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



  delay(2000);

  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    WiFiClient client;
    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    http.begin(client, "http://" SERVER_IP "/");  // HTTP
    http.addHeader("Content-Type", "application/json");

    Serial.print("[HTTP] POST...\n");
    // start connection and send HTTP header and body
    int httpCode = http.POST(payload);

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        const String& payload = http.getString();
        Serial.println("received payload:\n<<");
        Serial.println(payload);
        Serial.println(">>");
      }
    } else {
      Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
  delay(2000);
  delay(10000);
}
double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity * 0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}