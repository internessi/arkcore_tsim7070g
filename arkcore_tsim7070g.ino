#include <Arduino.h>
#include <NimBLEDevice.h>
#include <Preferences.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

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
const uint64_t sleepMinute  = 60ULL * 1000000ULL;

bool isSleepWakeup = false;
byte nvsSensorCounter = 0;
byte nvsGsmCounter = 0;


uint32_t unixTime = 1746345600;  // Platzhalter-Zeit bis erste Synchronisierung
uint32_t syncMillis = 0;  // Zeitpunkt der letzten Synchronisierung (millis)
uint32_t lastSendTime = 0;  // wird nach setup gesetzt
uint8_t minutesLeft = 10;
unsigned long lastMinuteTick = 0;

#include <aranet_rn.h>
#include <arkcore_tsim7070g.h>
#include <epaper.h>
#include <gsm_tsim7070g.h>

void switchResetReason() {
  esp_reset_reason_t reason = esp_reset_reason();

  switch (reason) {
    case ESP_RST_POWERON:
      // Serial.println("Reset-Grund: Power-On oder Flash-Vorgang");
      break;

    case ESP_RST_DEEPSLEEP:
      isSleepWakeup = true;
      loadCountersFromNVS();
      nvsSensorCounter++;
      nvsGsmCounter++; 
      // Serial.println("Reset-Grund: Aufwachen aus Deep Sleep");
      break;

    case ESP_RST_SW:
    case ESP_RST_WDT:
    case ESP_RST_PANIC:
    case ESP_RST_BROWNOUT:
      // Serial.println("Reset-Grund: Watchdog, Software-Reset, Panic oder Brownout");
      break;

    default:
      // Serial.println("Reset-Grund: Unbekannt oder sonstiger Grund");
      break;
  }
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);  // Brownout-Detector deaktivieren
  delay(100);
  pinMode(0, INPUT);
  pinMode(2, INPUT);
  pinMode(12, INPUT); // blaue LED
  pinMode(13, INPUT);
  pinMode(15, INPUT);
  pinMode(35, INPUT); // Spannungsteiler
  pinMode(MODEM_STATUS, INPUT); //  nur lesen
  pinMode(MODEM_PWRKEY, INPUT);
  //pinMode(MODEM_PWRKEY, OUTPUT); // Modem PWRKEY
  //digitalWrite(MODEM_PWRKEY, HIGH);  // PWRKEY = HIGH = AUS !
  //pinMode(MODEM_PWRKEY, OUTPUT);
  //digitalWrite(MODEM_PWRKEY, LOW);



  switchResetReason();
  if (!isSleepWakeup) { 
    delay(6000); // nur für test
    shutdownModemIfActive();
    delay(1000); // nur für test
  }

  Serial.begin(115200);
  delay(100);

  Serial.println("");
  if (isSleepWakeup) {  
    Serial.println("Reset-Grund: Aufwachen aus Deep Sleep");
  } else {
    Serial.println("Reset-Grund: Anderer Grund");
  }
  
  SN = generateSerialFromChipId();
  Serial.println("Seriennummer: " + SN);

  if (!isSleepWakeup || (nvsSensorCounter == 5)) {
    scanSensor();
    delay(1000);
    readSensor();
    if (humidity == 0 && advDevice != nullptr) {
      Serial.println("Humidity = 0 → retry...");
      delay(2000);
      readSensor();
    }
    NimBLEDevice::deinit(true);
    saveSensorDataToNVS();
    nvsSensorCounter = 0;
  }

  ePaperInit();
  loadSensorDataFromNVS();
  drawScreen();

  measureBattery();
  batDisplay();

  if (!isSleepWakeup || (nvsGsmCounter == 10)) {
    loadSensorDataFromNVS();
    pressure = batteryVolts100; // temporär batterie senden
    sendSensorDataViaGSM();
    nvsGsmCounter = 0;
  }

  countdownDisplay(nvsSensorCounter);
  SPI.end();

  Serial.println("Deep Sleep...");
  saveCountersToNVS();

  //digitalWrite(MODEM_PWRKEY, HIGH); // PWRKEY = HIGH = aus
  
  //pinMode(MODEM_PWRKEY, INPUT);        // entkoppeln
  //pinMode(MODEM_PWRKEY, OUTPUT);   // bewusst definieren
  //digitalWrite(MODEM_PWRKEY, HIGH); // halten
  delay(100);                      // stabilisieren

  esp_sleep_enable_timer_wakeup(1 * sleepMinute);
  esp_deep_sleep_start();

  //lastSendTime = millis(); remember the variable you will need it
}

void loop() {}