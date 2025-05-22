/**
 * arkcore_tsim7070g
 * Allgemeine Unterprogramme
 */

uint32_t getCurrentUnixTime() {
  return unixTime + (millis() / 1000 - syncMillis);
}

String generateSerialFromChipId() {
  const char* baseChars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"; // 32 Zeichen, verwechslungsfrei
  const int base = 32;
  String result = "";

  uint32_t chipId = (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF);  // untere 4 Bytes
  chipId += 0x5F3759DF; // optional: Streuung erhöhen

  while (chipId > 0) {
    result = String(baseChars[chipId % base]) + result;
    chipId /= base;
  }

  while (result.length() < 5) result = "A" + result;

  return "R" + result.substring(result.length() - 4); // z. B. R9X7D
}

void scanSensor() {
    statusScanAranetRn = scanAranetRn();
    Serial.printf("Scan: %s - %s\n", statusScanAranetRn, advDevice ? advDevice->getName().c_str() : "kein Gerät");
}

void readSensor() {
    const char* statusRead = readAranetRn();
    Serial.printf("Read: %s - %u Bq/m³ - %.1f °C - %.1f %%RH\n", statusRead, radon, temperature, humidity);
}

bool timeToReadSensor() {
    if (millis() - lastReadTime >= readInterval) {
        lastReadTime = millis();
        return true;
    }
    return false;
}

// Batteriemessung: nur GPIO35 intern (Faktor 2.0)
void measureBattery() {
  int raw = analogRead(35);
  float voltage = (raw / 4095.0) * 3.3 * 2.0;
  batteryVolts100 = voltage * 100 + 20; // Kalibrierwert anpassen

  if (batteryVolts100 == 20) {
    batteryVolts100 = 500;
  }

  Serial.print("BAT intern (35): ");
  Serial.print(batteryVolts100);
  Serial.println(" (x100 V)");
}

void loadCountersFromNVS() {
  Preferences prefs;
  prefs.begin("config", true);
  nvsSensorCounter = prefs.getUChar("sensorCount", 0);
  nvsGsmCounter    = prefs.getUChar("gsmCount", 0);
  prefs.end();
}

void saveCountersToNVS() {
  Preferences prefs;
  prefs.begin("config", false);
  prefs.putUChar("sensorCount", nvsSensorCounter);
  prefs.putUChar("gsmCount", nvsGsmCounter);
  prefs.end();
}

void saveSensorDataToNVS() {
  Preferences prefs;
  prefs.begin("sensor", false);
  prefs.putFloat("temp", temperature);
  prefs.putFloat("press", pressure);
  prefs.putFloat("hum", humidity);
  prefs.putUShort("radon", radon);
  prefs.end();
}

void loadSensorDataFromNVS() {
  Preferences prefs;
  prefs.begin("sensor", true);
  temperature = prefs.getFloat("temp", 0.0);
  pressure    = prefs.getFloat("press", 0.0);
  humidity    = prefs.getFloat("hum", 0.0);
  radon       = prefs.getUShort("radon", 0);
  prefs.end();
}
