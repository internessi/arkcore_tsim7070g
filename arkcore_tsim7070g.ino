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

uint32_t unixTime = 1746345600;  // Platzhalter-Zeit bis erste Synchronisierung
uint32_t syncMillis = 0;  // Zeitpunkt der letzten Synchronisierung (millis)

#include <aranet_rn.h>
#include <arkcore_tsim7070g.h>
#include <epaper.h>

void setup()
{
  Serial.begin(115200);
  SPI.begin(18, -1, 23, 21); // SCK, MISO, MOSI, SS
  delay(1000);
  delay(1000);
  Serial.println(" ");
  delay(1000);
  Serial.println("- - - - - - - - - - -");
  delay(1000);
  SN = generateSerialFromChipId();
  Serial.println("Seriennummer: " + SN); delay(1000);

  scanSensor(); delay(1000);
  readSensor(); delay(1000);

  ePaperInit();
  drawScreen();
  //ePaperShowAll();


}
  

void loop() {
  delay(10000);
  Serial.print(".");
}