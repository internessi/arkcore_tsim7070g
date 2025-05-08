#include <Arduino.h>
#include <NimBLEDevice.h>

#define LED_PIN 13

// BLE-Variablen
static const NimBLEAdvertisedDevice* advDevice;
static bool doConnect = false;
static const uint32_t scanTimeMs = 5000;
static uint32_t lastReadTime = 0;
static const uint32_t readInterval = 10 * 60 * 1000;

String SN;
float temperature;
float pressure;
float humidity;
uint16_t radon;
const char* statusScanAranetRn = "";

uint32_t unixTime = 1746345600;  // Platzhalter-Zeit bis erste Synchronisierung
uint32_t syncMillis = 0;  // Zeitpunkt der letzten Synchronisierung (millis)

#include <aranet_rn.h>
#include <radocon_alpha.h>
#include <gsm.h>

#include <WiFi.h>

const char* ssid = "flow";
const char* password = "Wolf1212";

void sendSensorDataViaWIFI() {
  Serial.print("WiFi: Verbinde mit "); Serial.println(ssid);
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(100);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Verbindung fehlgeschlagen");
    return;
  }

  Serial.println("Verbunden");

  WiFiClient client;
  if (!client.connect("con.radocon.de", 80)) {
    Serial.println("Verbindung zu Server fehlgeschlagen");
    return;
  }

  uint32_t now = getCurrentUnixTime();
  String url = "/nt/srd.php?sn=" + SN +
               "&bqm3=" + String(radon) +
               "&tmp=" + String(temperature, 1) +
               "&hum=" + String(humidity, 1) +
               "&hpa=" + String(pressure) +
               "&epoch=" + String(now);

  client.print("GET " + url + " HTTP/1.1\r\nHost: con.radocon.de\r\nConnection: close\r\n\r\n");

  while (client.connected() || client.available()) {
    if (client.available()) client.read(); // einfache Pufferleerung
  }

  client.stop();
  WiFi.disconnect(true);
  Serial.println("WiFi: Senden abgeschlossen");
}

void setup() {
  // Sensor-Teil
  pinMode(LED_PIN, OUTPUT); delay(1000);
  Serial.begin(115200); delay(1000);
  SerialMon.begin(115200); delay(1000);

  SN = generateSerialFromChipId();
  Serial.println("Seriennummer: " + SN); delay(1000);
  
  scanSensor(); delay(1000);
  readSensor(); delay(1000);

  // sendSensorDataViaGSM(); delay(1000);
  sendSensorDataViaWIFI(); delay(1000);
}

void loop() {
  if (timeToReadSensor()) {

    readSensor();
    // sendSensorDataViaGSM();
    sendSensorDataViaWIFI();
  }
}


 // Serial.println("LightSleep: 9 Minuten"); delay(1000);
//  esp_sleep_enable_timer_wakeup(9ULL * 60 * 1000000);
 // esp_light_sleep_start();
 // Serial.println("Ende Setup: 9 Minuten"); delay(1000);
