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
