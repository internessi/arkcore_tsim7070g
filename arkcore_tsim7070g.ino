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
float temperature = 20;
float pressure = 900;
float humidity = 40;
uint16_t radon = 10;
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
      saveSensorDataToNVS();
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

bool testAndShutdownModem() {
  sim7070.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(100);  // kurze Stabilisierung

  bool ok = modem.testAT(1000);  // Antwort auf AT?

  if (ok) {
    SerialMon.println("Modem antwortet auf AT");
    modem.sendAT("+CPOWD=1");    // reguläres Abschalten
    delay(2000);                 // Zeit zum Herunterfahren
  } else {
    SerialMon.println("Keine Antwort vom Modem");
  }

  sim7070.end();                 // UART freigeben
  serialBegun = false;          // Status zurücksetzen
  return ok;
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


  Serial.begin(115200);
  delay(100);

  switchResetReason();
  if (!isSleepWakeup) {
    delay(6000);
    blinkBlueLed3x();
    testAndShutdownModem();
    blinkBlueLed3x();
    delay(3000);
  }

  Serial.println("");
  if (isSleepWakeup) {  
    Serial.println("Reset-Grund: Aufwachen aus Deep Sleep");
  } else {
    Serial.println("Reset-Grund: Anderer Grund");
  }
  
  SN = generateSerialFromChipId();
  //SN = "RGZ9R"; // nur für mein Testmodell !!!!
  Serial.println("Seriennummer: " + SN);

  if (!isSleepWakeup || (nvsSensorCounter == 9)) {
    scanSensor();
    delay(100);
    readSensor();
    if (humidity == 0) {
      loadSensorDataFromNVS();
    }
    //NimBLEDevice::deinit(true);
    saveSensorDataToNVS();
    nvsSensorCounter = 0;
  }

  ePaperInit();
  loadSensorDataFromNVS();
  drawScreen();

  measureBattery();
  batDisplay();

  // Senden nur, wenn BAT > 3.3V — und entweder kein Sleep-Wakeup oder Zähler auf 9
  if (batteryVolts100 > 330 && (!isSleepWakeup || nvsGsmCounter == 9)) {
    loadSensorDataFromNVS();
    pressure = batteryVolts100 + 400; // temporär batterie senden
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
  
  //delay(59000); // Alternativer sleep mit viel verbrauch
  //esp_sleep_enable_timer_wakeup(1000000ULL); // 1 Sekunde Deep Sleep
  
  esp_sleep_enable_timer_wakeup(1 * sleepMinute);
  esp_deep_sleep_start();

  //lastSendTime = millis(); remember the variable you will need it
}

void loop() {}