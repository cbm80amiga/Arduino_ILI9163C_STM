// ILI9163C library example
// (c) 2019 Pawel A. Hernik

/*
Pinout (header on the top, from left):
  LED   -> 3.3V
  SCK   -> PA5
  SDA   -> PA7/MOSI
  A0/DC -> PA1 or any digital
  RESET -> PA0 or any digital
  CS    -> PA2 or any digital
  GND   -> GND
  VCC   -> 3.3V
*/


#define TFT_CS  PA2
#define TFT_DC  PA1
#define TFT_RST PA0
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ILI9163C_Fast.h>
Arduino_ILI9163C lcd = Arduino_ILI9163C(TFT_DC, TFT_RST, TFT_CS);

void setup(void) 
{
  Serial.begin(9600);
  lcd.init();
  lcd.fillScreen(RED);
  lcd.setCursor(0, 0);
  lcd.setTextColor(WHITE,BLUE);
  lcd.setTextSize(1);
  lcd.println("HELLO WORLD");
}

void loop()
{
}

