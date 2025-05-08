#define TINY_GSM_MODEM_SIM800
#define MODEM_RX 26
#define MODEM_TX 27
#define MODEM_BAUD 9600
#define MODEM_PWRKEY 4
#define MODEM_POWER_ON 23
#define MODEM_RST 5
#define LED_GPIO 12
#define LED_OFF HIGH
#define SerialMon Serial

#include <TinyGsmClient.h>

HardwareSerial sim800(1);
TinyGsm modem(sim800);

void setupModem() {
#ifdef MODEM_RST
  pinMode(MODEM_RST, OUTPUT); digitalWrite(MODEM_RST, HIGH);
#endif
  pinMode(MODEM_PWRKEY, OUTPUT); pinMode(MODEM_POWER_ON, OUTPUT);
  digitalWrite(MODEM_POWER_ON, HIGH); digitalWrite(MODEM_PWRKEY, HIGH);
  delay(100); digitalWrite(MODEM_PWRKEY, LOW); delay(1000); digitalWrite(MODEM_PWRKEY, HIGH);
  pinMode(LED_GPIO, OUTPUT); digitalWrite(LED_GPIO, LED_OFF);
  modem.sendAT("E0");
}

void turnOnNetlight()  { modem.sendAT("+CNETLIGHT=1"); }
void turnOffNetlight() { modem.sendAT("+CNETLIGHT=0"); }

void sendSensorDataViaGSM() {
  sim800.begin(MODEM_BAUD, SERIAL_8N1, MODEM_RX, MODEM_TX);
  setupModem(); delay(3000);

  SerialMon.print("GSM: Start - ");
  modem.restart(); SerialMon.print("bereit OK - ");
  turnOnNetlight();

  SerialMon.print("Netz ");
  while (!modem.waitForNetwork(10000)) SerialMon.print(".");
  SerialMon.print(" OK - ");

  if (!modem.gprsConnect("iot.1nce.net", "", "")) {
    SerialMon.println("GPRS Fehler");
    return;
  }
  SerialMon.print("GPRS OK - ");

  TinyGsmClient client(modem);
  SerialMon.print("HTTP ");
  if (!client.connect("con.radocon.de", 80)) {
    SerialMon.println("Fehler");
    return;
  }
  SerialMon.print("OK - ");

  uint32_t now = getCurrentUnixTime();
  String url = "/nt/srd.php?sn=" + SN +
               "&bqm3=" + String(radon) +
               "&tmp=" + String(temperature, 1) +
               "&hum=" + String(humidity, 1) +
               "&hpa=" + String(pressure) +
               "&epoch=" + String(now);

  client.print("GET " + url + " HTTP/1.1\r\nHost: con.radocon.de\r\nConnection: close\r\n\r\n");

  String response = ""; bool capture = false;
  unsigned long timeout = millis() + 10000;
  while (millis() < timeout && (client.connected() || client.available())) {
    while (client.available()) {
      char c = client.read();
      if (c == '#') { capture = true; response = ""; }
      else if (capture) response += c;
    }
  }

  client.stop();
  turnOffNetlight();
  modem.sendAT("+CPOWD=1");
  SerialMon.print("GSM: Close - ");

  response.trim();
  uint32_t serverTime = response.toInt();
  if (serverTime > 1746363600) {
    unixTime = serverTime;
    syncMillis = millis();
    SerialMon.printf("SYNC: %lu\n", unixTime);
  } else {
    SerialMon.printf("SYNC: Ungültige Zeit: '%s'\n", response.c_str());
  }
}



