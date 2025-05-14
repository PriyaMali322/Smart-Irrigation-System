#define BLYNK_TEMPLATE_ID "TMPL31uokKOR1"
#define BLYNK_TEMPLATE_NAME "Smart Irrigation"
#define BLYNK_PRINT Serial

#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>

// Blynk credentials
char auth[] = "zcXdcyxdPQWT4yjN2RUYKC_kddo9zaHs";
char ssid[] = "Priyanka";
char pass[] = "priya322";

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin definitions
#define SOIL_PIN A0
#define PIR_PIN D5
#define RELAY_PIN D3
#define DHT_PIN D4
#define DHTTYPE DHT11
DHT dht(DHT_PIN, DHTTYPE);

// Blynk Virtual Pins
#define VPIN_TEMP V0
#define VPIN_HUM V1
#define VPIN_SOIL V3
#define VPIN_MOTION_LED V5
#define VPIN_RELAY_BUTTON V12
#define VPIN_PIR_TOGGLE V6

BlynkTimer timer;
WidgetLED motionLED(VPIN_MOTION_LED);

int relayState = LOW;
int pirToggle = 0;
int soilMoisture = 100;
bool motionDetected = false;

void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();

  pinMode(PIR_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, relayState);

  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  dht.begin();

  lcd.setCursor(0, 0);
  lcd.print("SMART IRRIGATION");

  timer.setInterval(1000L, readDHT);
  timer.setInterval(1000L, readSoilMoisture);
  timer.setInterval(1000L, readPIR);
}

void readDHT() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("DHT read failed");
    return;
  }

  Blynk.virtualWrite(VPIN_TEMP, t);
  Blynk.virtualWrite(VPIN_HUM, h);
}

void readSoilMoisture() {
  int value = analogRead(SOIL_PIN);
  soilMoisture = map(value, 0, 1024, 100, 0); // 100 = wet, 0 = dry
  Blynk.virtualWrite(VPIN_SOIL, soilMoisture);

  // Auto control
  if (soilMoisture < 30) {
    relayState = LOW; // Motor ON
  } else {
    relayState = HIGH; // Motor OFF
  }

  digitalWrite(RELAY_PIN, relayState);
  Blynk.virtualWrite(VPIN_RELAY_BUTTON, relayState);
  updateLCD();
}

void readPIR() {
  if (pirToggle == 1) {
    motionDetected = digitalRead(PIR_PIN);
    if (motionDetected) {
      Blynk.logEvent("motion_detected", "WARNING! Motion Detected!");
      motionLED.on();
    } else {
      motionLED.off();
    }
  } else {
    motionDetected = false;
    motionLED.off();
  }
  updateLCD();
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("SMART IRRIGATION");

  lcd.setCursor(0, 1);
  lcd.print("S:");
  if (soilMoisture < 0) lcd.print(" "); // extra space for alignment
  lcd.print(soilMoisture);
  lcd.print("% ");

  lcd.print(relayState == LOW ? "W:ON " : "W:OFF");

  lcd.print(motionDetected ? "M:ON " : "M:ON");
}

BLYNK_WRITE(VPIN_RELAY_BUTTON) {
  relayState = param.asInt();
  digitalWrite(RELAY_PIN, relayState);
  updateLCD();
}

BLYNK_WRITE(VPIN_PIR_TOGGLE) {
  pirToggle = param.asInt();
}

BLYNK_CONNECTED() {
  Blynk.syncVirtual(VPIN_RELAY_BUTTON);
  Blynk.syncVirtual(VPIN_PIR_TOGGLE);
}

void loop() {
  Blynk.run();
  timer.run();
}
