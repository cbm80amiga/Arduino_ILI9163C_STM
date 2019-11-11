// Fast ILI9163C 128x128 LCD SPI display library
// (c) 2019 by Pawel A. Hernik

#include "Arduino_ILI9163C_STM.h"
#include <limits.h>
#include "pins_arduino.h"
#include "wiring_private.h"
#include <SPI.h>

// Initialization commands for ILI9163C
// taken from https://github.com/sumotoy/TFT_ILI9163C
static const uint8_t PROGMEM init_ILI9163C[] = {
  18,
  ILI9163C_SWRESET,  0+ST_CMD_DELAY, 100,
  ILI9163C_SLPOUT,   0+ST_CMD_DELAY, 5,
  ILI9163C_PIXFMT,   1+ST_CMD_DELAY, 0x05, 5,
  ILI9163C_GAMMASET, 1+ST_CMD_DELAY, 0x04, 1,
  ILI9163C_GAMRSEL,  1+ST_CMD_DELAY, 0x01, 1,
  ILI9163C_NORML,    0,
  ILI9163C_DFUNCTR,  2,0b11111111,0b00000110,
  
  ILI9163C_FRMCTR1,  2+ST_CMD_DELAY, 0x08,0x02, 1,
  ILI9163C_DINVCTR,  1+ST_CMD_DELAY, 0x07, 1,
  ILI9163C_PWCTR1,   2+ST_CMD_DELAY, 0x0A,0x02, 1,
  ILI9163C_PWCTR2,   1+ST_CMD_DELAY, 0x02, 1,
  ILI9163C_VCOMCTR1, 2+ST_CMD_DELAY, 0x50,99, 1,
  ILI9163C_VCOMOFFS, 1+ST_CMD_DELAY, 0, 1,
  ILI9163C_CLMADRS,  4, 0,0, (_GRAMWIDTH>>8),(_GRAMWIDTH),
  ILI9163C_PGEADRS,  4, 0,0, (_GRAMHEIGH>>8),(_GRAMHEIGH),
  ILI9163C_VSCLLDEF, 6, (__OFFSET>>8),(__OFFSET), (_GRAMHEIGH - __OFFSET)>>8,(_GRAMHEIGH - __OFFSET), 0,0,

  ILI9163C_DISPON,   0+ST_CMD_DELAY, 1,
  ILI9163C_RAMWR,    0+ST_CMD_DELAY, 1
};

// macros for fast DC and CS state changes
#ifdef COMPATIBILITY_MODE
#define DC_DATA     digitalWrite(dcPin, HIGH)
#define DC_COMMAND  digitalWrite(dcPin, LOW)
#define CS_IDLE     digitalWrite(csPin, HIGH)
#define CS_ACTIVE   digitalWrite(csPin, LOW)
#else
#define DC_DATA     digitalWrite(dcPin, HIGH)
#define DC_COMMAND  digitalWrite(dcPin, LOW)
#define CS_IDLE     digitalWrite(csPin, HIGH)
#define CS_ACTIVE   digitalWrite(csPin, LOW)
#endif

// if CS always connected to the ground then don't do anything for better performance
#ifdef CS_ALWAYS_LOW
#define CS_IDLE
#define CS_ACTIVE
#endif

// ----------------------------------------------------------
Arduino_ILI9163C::Arduino_ILI9163C(int8_t dc, int8_t rst, int8_t cs) : Adafruit_GFX(_TFTWIDTH,_TFTHEIGHT) 
{
  csPin = cs;
  dcPin = dc;
  rstPin = rst;
}

// ----------------------------------------------------------
void Arduino_ILI9163C::init() 
{
  pinMode(dcPin, OUTPUT);
#ifndef CS_ALWAYS_LOW
	pinMode(csPin, OUTPUT);
#endif

  CS_ACTIVE;
  if(rstPin != -1) {
    pinMode(rstPin, OUTPUT);
    digitalWrite(rstPin, HIGH);
    delay(50);
    digitalWrite(rstPin, LOW);
    delay(50);
    digitalWrite(rstPin, HIGH);
    delay(50);
  }

  spiStart8b();

  _width  = 128;
  _height = 128;
  displayInit(init_ILI9163C);

  int i;
	writeCmd(ILI9163C_PGAMMAC); //Positive Gamma Correction Setting
	for(i=0;i<15;i++) writeData(pGammaSet[i]);
	writeCmd(ILI9163C_NGAMMAC); //Negative Gamma Correction Setting
	for(i=0;i<15;i++) writeData(nGammaSet[i]);

  setRotation(0);

  spiStart16b();
}

// ----------------------------------------------------------
void Arduino_ILI9163C::spiStart8b()
{
  SPI.beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE0, DATA_SIZE_8BIT));
}
// ----------------------------------------------------------
void Arduino_ILI9163C::spiStart16b()
{
  SPI.beginTransaction(SPISettings(SPI_FREQ, MSBFIRST, SPI_MODE0, DATA_SIZE_16BIT));
}
// ----------------------------------------------------------
void Arduino_ILI9163C::writeCmd(uint8_t c) 
{
  DC_COMMAND;
  CS_ACTIVE;

  SPI.write(c);

  CS_IDLE;
}

// ----------------------------------------------------------
void Arduino_ILI9163C::writeData(uint8_t d) 
{
  DC_DATA;
  CS_ACTIVE;
    
  SPI.write(d);

  CS_IDLE;
}
// ----------------------------------------------------------
void Arduino_ILI9163C::displayInit(const uint8_t *addr) 
{
  uint8_t numCommands, numArgs;
  uint16_t ms;
  numCommands = pgm_read_byte(addr++);
  while(numCommands--) {
    writeCmd(pgm_read_byte(addr++));
    numArgs  = pgm_read_byte(addr++);
    ms       = numArgs & ST_CMD_DELAY;
    numArgs &= ~ST_CMD_DELAY;
    while(numArgs--) writeData(pgm_read_byte(addr++));

    if(ms) {
      ms = pgm_read_byte(addr++);
      if(ms == 255) ms = 500;
      delay(ms);
    }
  }
}

// ----------------------------------------------------------
void Arduino_ILI9163C::setRotation(uint8_t m) 
{
  uint8_t madctl;
  rotation = m & 3;
	switch (rotation) {
	case 0:
		madctl =  __COLORSPC;
		_width  = _TFTWIDTH;
		_height = _TFTHEIGHT;
		break;
	case 1:
		madctl = ILI9163C_MADCTL_MX | ILI9163C_MADCTL_MV | __COLORSPC;
		_width  = _TFTHEIGHT;
		_height = _TFTWIDTH;
		break;
	case 2:
		madctl = ILI9163C_MADCTL_MY | ILI9163C_MADCTL_MX | __COLORSPC;
		_width  = _TFTWIDTH;
		_height = _TFTHEIGHT;
		break;
	case 3:
		madctl = ILI9163C_MADCTL_MY | ILI9163C_MADCTL_MV | __COLORSPC;
		_width  = _TFTWIDTH;
		_height = _TFTHEIGHT;
		break;
	}
  spiStart8b();
	writeCmd(ILI9163C_MADCTL);
	writeData(madctl);
  spiStart16b();
}

// ----------------------------------------------------------
// optimized
void Arduino_ILI9163C::setAddrWindow(uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye) 
{
  CS_ACTIVE;
  DC_COMMAND; SPI.write(ILI9163C_CLMADRS);
  DC_DATA; SPI.write(xs); SPI.write(xe);
  DC_COMMAND; SPI.write(ILI9163C_PGEADRS);
  DC_DATA; SPI.write(ys); SPI.write(ye);
  DC_COMMAND; SPI.write(ILI9163C_RAMWR);
  CS_IDLE;
  DC_DATA;
}

// ----------------------------------------------------------
void Arduino_ILI9163C::pushColor(uint16_t color) 
{
  DC_DATA;
  CS_ACTIVE;

  SPI.write(color);

  CS_IDLE;
}

// ----------------------------------------------------------
// optimized
void Arduino_ILI9163C::drawPixel(int16_t x, int16_t y, uint16_t color) 
{
  if(x<0 ||x>=_width || y<0 || y>=_height) return;
  CS_ACTIVE;
  DC_COMMAND; SPI.write(ILI9163C_CLMADRS);
  DC_DATA; SPI.write(x); SPI.write(x+1);
  DC_COMMAND; SPI.write(ILI9163C_PGEADRS);
  DC_DATA; SPI.write(y); SPI.write(y+1);
  DC_COMMAND; SPI.write(ILI9163C_RAMWR);
  DC_DATA; SPI.write(color);
  CS_IDLE;
}

// ----------------------------------------------------------
void Arduino_ILI9163C::drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) 
{
  if(x>=_width || y>=_height || h<=0) return;
  if(y+h-1>=_height) h=_height-y;
  if (h<2) { drawPixel(x, y, color); return; } 
  setAddrWindow(x, y, x, y+h-1);

  CS_ACTIVE;

#ifndef COMPATIBILITY_MODE
  if(h>DMA_MIN) {
    dmaBuf[0] = color;
    SPI.dmaSend(dmaBuf, h, 0);
  } else 
#endif
  SPI.write(color, h);

  CS_IDLE;
}

// ----------------------------------------------------------
void Arduino_ILI9163C::drawFastHLine(int16_t x, int16_t y, int16_t w,  uint16_t color) 
{
  if(x>=_width || y>=_height || w<=0) return;
  if(x+w-1>=_width)  w=_width-x;
  if(w<2) { drawPixel(x, y, color); return; } 
  setAddrWindow(x, y, x+w-1, y);

  CS_ACTIVE;

#ifndef COMPATIBILITY_MODE
  if(w>DMA_MIN) {
    dmaBuf[0] = color;
    SPI.dmaSend(dmaBuf, w, 0);
  } else
#endif
  SPI.write(color, w);
 
  CS_IDLE;
}

// ----------------------------------------------------------
void Arduino_ILI9163C::fillScreen(uint16_t color) 
{
  fillRect(0, 0, _width, _height, color);
}

// ----------------------------------------------------------
void Arduino_ILI9163C::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) 
{
  if(x>=_width || y>=_height || w<=0 || h<=0) return;
  if(x+w-1>=_width)  w=_width -x;
  if(y+h-1>=_height) h=_height-y;
  setAddrWindow(x, y, x+w-1, y+h-1);

  dmaBuf[0] = color;

  CS_ACTIVE;
  uint32_t num = w * h;

#ifndef COMPATIBILITY_MODE
  if(num>DMA_MIN) {
    while(num>DMA_MAX ) {
      num -= DMA_MAX;
      SPI.dmaSend(dmaBuf, DMA_MAX, 0);
    }
    SPI.dmaSend(dmaBuf, num, 0);
  } else 
#endif
  SPI.write(color, num);

  CS_IDLE;
}

// ----------------------------------------------------------
// draws image from RAM
void Arduino_ILI9163C::drawImage(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *img16) 
{
  if(x>=_width || y>=_height || w<=0 || h<=0) return;
  setAddrWindow(x, y, x+w-1, y+h-1);

  CS_ACTIVE;
  uint32_t num = (uint32_t)w*h;
  SPI.write(img16, num);
  CS_IDLE;
}

// ----------------------------------------------------------
// draws image from flash (PROGMEM)
void Arduino_ILI9163C::drawImageF(int16_t x, int16_t y, int16_t w, int16_t h, const uint16_t *img16) 
{
  if(x>=_width || y>=_height || w<=0 || h<=0) return;
  setAddrWindow(x, y, x+w-1, y+h-1);

  CS_ACTIVE;
  uint32_t num = (uint32_t)w*h;
  SPI.write(img16, num);
  CS_IDLE;
}

// ----------------------------------------------------------
// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t Arduino_ILI9163C::Color565(uint8_t r, uint8_t g, uint8_t b) 
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// ----------------------------------------------------------
void Arduino_ILI9163C::invertDisplay(boolean mode) 
{
  spiStart8b();
  writeCmd(mode ? ILI9163C_DINVON : ILI9163C_DINVOF);
  spiStart16b();
}

// ----------------------------------------------------------
void Arduino_ILI9163C::partialDisplay(boolean mode) 
{
  spiStart8b();
  writeCmd(mode ? ILI9163C_PTLON : ILI9163C_NORML);
  spiStart16b();
}

// ----------------------------------------------------------
void Arduino_ILI9163C::sleepDisplay(boolean mode) 
{
  spiStart8b();
  writeCmd(mode ? ILI9163C_SLPIN : ILI9163C_SLPOUT);
  spiStart16b();
  delay(5);
}

// ----------------------------------------------------------
void Arduino_ILI9163C::enableDisplay(boolean mode) 
{
  spiStart8b();
  writeCmd(mode ? ILI9163C_DISPON : ILI9163C_DISPOFF);
  spiStart16b();
}

// ----------------------------------------------------------
void Arduino_ILI9163C::idleDisplay(boolean mode) 
{
  spiStart8b();
  writeCmd(mode ? ILI9163C_IDLEON : ILI9163C_IDLEOF);
  spiStart16b();
}

// ----------------------------------------------------------
void Arduino_ILI9163C::resetDisplay() 
{
  spiStart8b();
  writeCmd(ILI9163C_SWRESET);
  spiStart16b();
  delay(500);
}

// ----------------------------------------------------------
void Arduino_ILI9163C::setScrollArea(uint16_t tfa, uint16_t bfa) 
{
  tfa += __OFFSET;
  uint16_t vsa = _GRAMHEIGH-tfa-bfa;
  spiStart8b();
  writeCmd(ILI9163C_VSCLLDEF);
  writeData(tfa >> 8); writeData(tfa);
  writeData(vsa >> 8); writeData(vsa);
  writeData(bfa >> 8); writeData(bfa);
  spiStart16b();
}

// ----------------------------------------------------------
void Arduino_ILI9163C::setScroll(uint16_t vsp) 
{
  vsp+=__OFFSET;
  spiStart8b();
  writeCmd(ILI9163C_VSSTADRS);
  writeData(vsp >> 8); writeData(vsp);
  spiStart16b();
}

// ----------------------------------------------------------
void Arduino_ILI9163C::setPartArea(uint16_t sr, uint16_t er) 
{
  spiStart8b();
  writeCmd(ILI9163C_PARTAREA);  // SETPARTAREA = 0x30
  writeData(sr >> 8); writeData(sr);
  writeData(er >> 8); writeData(er);
  spiStart16b();
}

// ------------------------------------------------
// Input a value 0 to 511 (85*6) to get a color value.
// The colours are a transition R - Y - G - C - B - M - R.
void Arduino_ILI9163C::rgbWheel(int idx, uint8_t *_r, uint8_t *_g, uint8_t *_b)
{
  idx &= 0x1ff;
  if(idx < 85) { // R->Y  
    *_r = 255; *_g = idx * 3; *_b = 0;
    return;
  } else if(idx < 85*2) { // Y->G
    idx -= 85*1;
    *_r = 255 - idx * 3; *_g = 255; *_b = 0;
    return;
  } else if(idx < 85*3) { // G->C
    idx -= 85*2;
    *_r = 0; *_g = 255; *_b = idx * 3;
    return;  
  } else if(idx < 85*4) { // C->B
    idx -= 85*3;
    *_r = 0; *_g = 255 - idx * 3; *_b = 255;
    return;    
  } else if(idx < 85*5) { // B->M
    idx -= 85*4;
    *_r = idx * 3; *_g = 0; *_b = 255;
    return;    
  } else { // M->R
    idx -= 85*5;
    *_r = 255; *_g = 0; *_b = 255 - idx * 3;
   return;
  }
} 

uint16_t Arduino_ILI9163C::rgbWheel(int idx)
{
  uint8_t r,g,b;
  rgbWheel(idx, &r,&g,&b);
  return RGBto565(r,g,b);
}

// ------------------------------------------------