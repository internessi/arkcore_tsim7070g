

#include <epaper.h>

void setup()
{
  Serial.begin(115200);
  SPI.begin(18, -1, 23, 21); // SCK, MISO, MOSI, SS
  delay(4000);
  Serial.println(" ");
  delay(1000);
  Serial.println("Hello, world!");

  ePaperInit();

  //ePaperShowAll();


}
  
  


void loop() {
  delay(2000);
  Serial.print(".");
}