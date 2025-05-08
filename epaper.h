/**
 * epaper.h
 * alles für das display
 */


#define ENABLE_GxEPD2_GFX 0
#include <GxEPD2_BW.h>
#include <GxEPD2_3C.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold18pt7b.h>  
#include <Fonts/FreeSansBold18pt7b.h>
#include <FreeSansBold12pt7b.h>
#include <bahnschrift22pt7b.h>


// 1.54'' EPD Module
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(GxEPD2_154_D67(/*CS=*/21, /*DC=*/22, /*RES=*/5, /*BUSY=*/39)); // GDEH0154D67 200x200, SSD1681

const char HelloWorld[] = "Hello World!";
const char HelloWeACtStudio[] = "WAWA";

void helloWorld()
{
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  int16_t tbx, tby; uint16_t tbw, tbh;
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  // center the bounding box by transposition of the origin:
  uint16_t x = ((display.width() - tbw) / 2) - tbx;
  uint16_t y = ((display.height() - tbh) / 2) - tby;
  display.setFullWindow();
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(x, y-tbh);
    display.print(HelloWorld);
    display.setTextColor(display.epd2.hasColor ? GxEPD_RED : GxEPD_BLACK);
    display.getTextBounds(HelloWeACtStudio, 0, 0, &tbx, &tby, &tbw, &tbh);
    x = ((display.width() - tbw) / 2) - tbx;
    display.setCursor(x, y+tbh);
    display.print(HelloWeACtStudio);
  }
  while (display.nextPage());
}

void helloFullScreenPartialMode()
{
  //Serial.println("helloFullScreenPartialMode");
  const char fullscreen[] = "full screen update";
  const char fpm[] = "fast partial mode";
  const char spm[] = "slow partial mode";
  const char npm[] = "no partial mode";
  display.setPartialWindow(0, 0, display.width(), display.height());
  display.setRotation(1);
  display.setFont(&FreeMonoBold9pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  const char* updatemode;
  if (display.epd2.hasFastPartialUpdate)
  {
    updatemode = fpm;
  }
  else if (display.epd2.hasPartialUpdate)
  {
    updatemode = spm;
  }
  else
  {
    updatemode = npm;
  }
  // do this outside of the loop
  int16_t tbx, tby; uint16_t tbw, tbh;
  // center update text
  display.getTextBounds(fullscreen, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t utx = ((display.width() - tbw) / 2) - tbx;
  uint16_t uty = ((display.height() / 4) - tbh / 2) - tby;
  // center update mode
  display.getTextBounds(updatemode, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t umx = ((display.width() - tbw) / 2) - tbx;
  uint16_t umy = ((display.height() * 3 / 4) - tbh / 2) - tby;
  // center HelloWorld
  display.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
  uint16_t hwx = ((display.width() - tbw) / 2) - tbx;
  uint16_t hwy = ((display.height() - tbh) / 2) - tby;
  display.firstPage();
  do
  {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(hwx, hwy);
    display.print(HelloWorld);
    display.setCursor(utx, uty);
    display.print(fullscreen);
    display.setCursor(umx, umy);
    display.print(updatemode);
  }
  while (display.nextPage());
  //Serial.println("helloFullScreenPartialMode done");
}

void showPartialUpdate()
{
  // some useful background
  helloWorld();
  // use asymmetric values for test
  uint16_t box_x = 10;
  uint16_t box_y = 15;
  uint16_t box_w = 70;
  uint16_t box_h = 20;
  uint16_t cursor_y = box_y + box_h - 6;
  if (display.epd2.WIDTH < 104) cursor_y = box_y + 6;
  float value = 13.95;
  uint16_t incr = display.epd2.hasFastPartialUpdate ? 1 : 3;
  display.setFont(&FreeMonoBold9pt7b);
  if (display.epd2.WIDTH < 104) display.setFont(0);
  display.setTextColor(GxEPD_BLACK);
  // show where the update box is
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.firstPage();
    do
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_BLACK);
      //display.fillScreen(GxEPD_BLACK);
    }
    while (display.nextPage());
    delay(2000);
    display.firstPage();
    do
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    }
    while (display.nextPage());
    delay(1000);
  }
  //return;
  // show updates in the update box
  for (uint16_t r = 0; r < 4; r++)
  {
    display.setRotation(r);
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    for (uint16_t i = 1; i <= 10; i += incr)
    {
      display.firstPage();
      do
      {
        display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
        display.setCursor(box_x, cursor_y);
        display.print(value * i, 2);
      }
      while (display.nextPage());
      delay(500);
    }
    delay(1000);
    display.firstPage();
    do
    {
      display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    }
    while (display.nextPage());
    delay(1000);
  }
  Serial.println("loop:");
}

void ePaperInit()
{
  display.init(115200, true, 50, false);
  Serial.println("Display init done");
}


void ePaperShowAll()
{
  helloWorld();
  helloFullScreenPartialMode();
  delay(1000);
  if (display.epd2.hasFastPartialUpdate)
  {
    showPartialUpdate();
    delay(1000);
  }
  display.hibernate();
}

void drawScreen()
{
  char tempStr[10];
  snprintf(tempStr, sizeof(tempStr), "%.1f", temperature);  // z. B. "22.7"

  display.setRotation(1);
  display.setFullWindow();
  display.setTextColor(GxEPD_BLACK);

  // zuerst Temperatur

  int16_t x = 2;
  int16_t y = 31;

  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);

    // Temperaturwert
    display.setFont(&bahnschrift22pt7b);
    display.setCursor(x, y);
    display.print(tempStr);

    // Breite der Ziffern berechnen
    int16_t tbx, tby;
    uint16_t tbw, tbh;
    display.getTextBounds(tempStr, x, y, &tbx, &tby, &tbw, &tbh);

    // Gradzeichen als kleines o
    display.setFont(&FreeSansBold12pt7b);
    display.setCursor(x + tbw + 4, y - 19);
    display.print("o");

    // C wieder in Bahnschrift
    display.setFont(&bahnschrift22pt7b);
    display.setCursor(x + tbw + 15, y);
    display.print("C");
  
  // humidity anzeigen
    char humStr[5];
    snprintf(humStr, sizeof(humStr), "%.0f", humidity);  // ohne Nachkomma, z. B. "43"

    display.setFont(&bahnschrift22pt7b);
    display.setTextColor(GxEPD_BLACK);
    display.setCursor(130, 32);  // links vom Tropfen
    display.print(humStr);

    int16_t hx = 188;       // horizontal zentriert
    int16_t hy = 22;        // vertikale Mitte

    // Tropfenkörper (Kreis unten)
    display.fillCircle(hx, hy, 11, GxEPD_BLACK);  // Radius 11 → Durchmesser 22

    // Tropfenspitze (nach oben, 1 px kleiner)
    display.fillTriangle(hx - 11, hy, hx + 11, hy, hx, hy - 21, GxEPD_BLACK);

    // % innen (weiß, zentriert)
    display.setFont(&FreeMonoBold9pt7b);
    display.setTextColor(GxEPD_WHITE);
    display.setCursor(hx - 6, hy + 4);
    display.print("%");

    display.setTextColor(GxEPD_BLACK);  // Farbe zurücksetzen

  } while (display.nextPage());
}
