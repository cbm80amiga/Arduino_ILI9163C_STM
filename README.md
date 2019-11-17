# Arduino_ILI9163C_STM
Fast STM32 SPI/DMA library for the ILI9163C 1.44" 128x128 LCD

YouTube video (AVR Arduino):
https://youtu.be/V1KBm99Qagw

Significantly optimized for STM32 boards. Supports 36MHz SPI and DMA channel

## Configuration

Use "#define COMPATIBILITY_MODE" - then the library should work on all Arduino compatible boards
Remove above for the best performance on 16MHz AVR

Use "#define CS_ALWAYS_LOW" for LCD boards where CS pin is internally connected to the ground, it gives better performance

"#define __144_BLACK_PCB__"  for "black" PCB version

"#define __144_RED_PCB__" for "red" PCB version

## Extra Features
- invertDisplay()
- sleepDisplay()
- enableDisplay()
- idleDisplay() - saves power by limiting colors to 3 bit mode (8 colors)
- resetDisplay() - software reset
- partialDisplay() and setPartArea() - limiting display area for power saving
- setScrollArea() and setScroll() - smooth vertical scrolling
- fast drawImage() from RAM
- fast drawImage() from flash (PROGMEM)

## Connections (header on top):

|LCD pin|LCD pin name|Arduino|
|--|--|--|
 |#01| LED| 3.3V|
 |#02| SCK |PA5/SCK|
 |#03| SCA |PA7/MOSI|
 |#04| A0/DC|PA1 or any digital
 |#05| RESET|PA0 or any digital|
 |#06| CS|PA2 or any digital|
 |#07| GND | GND|
 |#08| VCC | 3.3V|
 
