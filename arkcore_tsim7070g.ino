#include <Arduino.h>
#include <NimBLEDevice.h>

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

uint16_t batteryVolts100 = 0; // interne Teilung, in 0.01 V (z. B. 331 = 3.31 V)

uint32_t unixTime = 1746345600;  // Platzhalter-Zeit bis erste Synchronisierung
uint32_t syncMillis = 0;  // Zeitpunkt der letzten Synchronisierung (millis)
uint32_t lastSendTime = 0;  // wird nach setup gesetzt
uint8_t minutesLeft = 10;
unsigned long lastMinuteTick = 0;

#include <aranet_rn.h>
#include <arkcore_tsim7070g.h>
#include <epaper.h>
#include <gsm_tsim7070g.h>

// Batteriemessung: nur GPIO35 intern (Faktor 2.0)
void measureBattery() {
  int raw = analogRead(35);
  float voltage = (raw / 4095.0) * 3.3 * 2.0;
  batteryVolts100 = voltage * 100;  // z. B. 3.31 V → 331

  if (batteryVolts100 == 0) {
    batteryVolts100 = 500;
  }

  Serial.print("BAT intern (35): ");
  Serial.print(batteryVolts100);
  Serial.println(" (x100 V)");

  // Test: Spannung auch in pressure speichern
  pressure = batteryVolts100;
}

void setup() {
  delay(100);
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);

  pinMode(35, INPUT);
  delay(3000);
  Serial.begin(115200);
  SPI.begin(18, -1, 23, 21); // SCK, MISO, MOSI, SS
  delay(3000);

  Serial.println("\n- - - - - - - - - - -");
  SN = generateSerialFromChipId();
  Serial.println("Seriennummer: " + SN);

  scanSensor();
  delay(1000);
  readSensor();
  delay(1000);
  measureBattery();

  ePaperInit();
  drawScreen();
  batDisplay();
  sendSensorDataViaGSM();

  lastSendTime = millis();
  countdownDisplay(10);

  digitalWrite(15, LOW);
}

void loop() {
  unsigned long now = millis();

  if (now - lastSendTime >= 600000UL || lastSendTime == 0) {
    readSensor();
    measureBattery();
    drawScreen();
    batDisplay();
    sendSensorDataViaGSM();
    lastSendTime = now;
    minutesLeft = 10;
    countdownDisplay(minutesLeft);
    lastMinuteTick = now;
  }

  if (minutesLeft > 1 && now - lastMinuteTick >= 60000UL) {
    minutesLeft--;
    countdownDisplay(minutesLeft);
    lastMinuteTick = now;
  }
}