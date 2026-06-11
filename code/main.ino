/*******
  Blynk + Sensors + Relays (Pump and Fan Control)
 *******/

#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL6bkRlE2sV"
#define BLYNK_TEMPLATE_NAME "Green House Automated System"
#define BLYNK_AUTH_TOKEN "B26PNpyGJ1Hfi8GlnHtemwAkPN4sadSe"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// WiFi credentials
char ssid[] = "Redmi Note 9";
char pass[] = "20030506suga";

// Sensor and LCD setup
#define SOIL_MOISTURE_PIN A0
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// I2C LCD address and dimensions (16x2)
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Relay pins
#define PUMP_RELAY_PIN 14  // 5 GPIO14 - Relay 1 (Pump Control)
#define FAN_RELAY_PIN 12   // 6 GPIO12 - Relay 2 (Fan Control)

// Threshold values
#define SOIL_MOISTURE_THRESHOLD 300  // Value above which pump turns on
#define TEMPERATURE_THRESHOLD 25.0   // Temperature above which fan turns on

BlynkTimer timer;

// Variables to track manual control for pump and fan
bool manualPumpControl = false;
bool manualFanControl = false;

void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Initialize Blynk with WiFi credentials
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize LCD
  lcd.init();
  lcd.backlight();
  
  // Set the soil moisture pin as input
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  
  // Set the relay pins as output
  pinMode(PUMP_RELAY_PIN, OUTPUT);
  pinMode(FAN_RELAY_PIN, OUTPUT);

  // Set relays to OFF initially (assuming active LOW relays)
  digitalWrite(PUMP_RELAY_PIN, HIGH);
  digitalWrite(FAN_RELAY_PIN, HIGH);

  // Timer to send sensor data to Blynk and LCD every 2 seconds
  timer.setInterval(2000L, sendSensorData);
}


// Blynk function to control pump manually (V4 button)
BLYNK_WRITE(V4) {
  manualPumpControl = param.asInt(); // Read button state (0 or 1)
  if (manualPumpControl) {
    digitalWrite(PUMP_RELAY_PIN, LOW);  // Turn on pump (relay active LOW)
    Serial.println("Pump ON (Manual)");
  } else {
    digitalWrite(PUMP_RELAY_PIN, HIGH); // Turn off pump
    Serial.println("Pump OFF (Manual)");
  }
}

// Blynk function to control fan manually (V5 button)
BLYNK_WRITE(V5) {
  manualFanControl = param.asInt(); // Read button state (0 or 1)
  if (manualFanControl) {
    digitalWrite(FAN_RELAY_PIN, LOW);  // Turn on fan (relay active LOW)
    Serial.println("Fan ON (Manual)");
  } else {
    digitalWrite(FAN_RELAY_PIN, HIGH); // Turn off fan
    Serial.println("Fan OFF (Manual)");
  }
}

void sendSensorData() {
  // Read the temperature, humidity, and soil moisture
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int soilMoisture = analogRead(SOIL_MOISTURE_PIN);

  // Send data to Blynk
  Blynk.virtualWrite(V1, t);           // Virtual pin for temperature
  Blynk.virtualWrite(V2, h);           // Virtual pin for humidity
  Blynk.virtualWrite(V3, soilMoisture);  // Virtual pin for soil moisture

  // Print the data to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" °C, Humidity: ");
  Serial.print(h);
  Serial.print(" %, Soil: ");
  Serial.println(soilMoisture);

  // Display data on the LCD
  lcd.clear();
  lcd.setCursor(0, 0);   // First line
  lcd.print("Temp: ");
  lcd.print(t, 1);       // Print temperature with 1 decimal
  lcd.print("C");

  lcd.setCursor(10, 0);
  lcd.print("H: ");
  lcd.print(h, 0);       // Print humidity with no decimal
  lcd.print("%");

  lcd.setCursor(0, 1);   // Second line
  lcd.print("Soil: ");
  lcd.print(soilMoisture);  // Print soil moisture

  // Control pump relay based on soil moisture threshold
  if (soilMoisture > SOIL_MOISTURE_THRESHOLD) {
    digitalWrite(PUMP_RELAY_PIN, LOW);  // Turn on pump (relay active LOW)
    Serial.println("Pump ON");
  } else {
    digitalWrite(PUMP_RELAY_PIN, HIGH); // Turn off pump
    Serial.println("Pump OFF");
  }

  // Control fan relay based on temperature threshold
  if (t > TEMPERATURE_THRESHOLD) {
    digitalWrite(FAN_RELAY_PIN, LOW);   // Turn on fan (relay active LOW)
    Serial.println("Fan ON");
  } else {
    digitalWrite(FAN_RELAY_PIN, HIGH);  // Turn off fan
    Serial.println("Fan OFF");
  }
}

void loop() {
  Blynk.run();  // Run the Blynk service
  timer.run();  // Run the timer
}
