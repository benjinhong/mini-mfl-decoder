#include <TM1637Display.h>

#define CLK 2
#define DIO 3

TM1637Display display(CLK, DIO);

const unsigned char all[] =   {0xFF, 0xFF, 0xFF, 0xFF};
const unsigned char clear[] = {0x00, 0x00, 0x00, 0x00};

const unsigned char nt[] =       {0x40, 0x54, 0x78, 0x40};
const unsigned char rr[] =       {0x40, 0x50, 0x50, 0x40};
const unsigned char stop[] =     {0x40, 0x40, 0x40, 0x40};
const unsigned char on[] =       {0x40, 0x5C, 0x54, 0x40};
unsigned char dispGear[] = {0x40, 0x00, 0xFF, 0x40};


void setup() {
  display.setBrightness(7);
}

void loop() {
  dispGear[2] = display.encodeDigit(3);
  display.setSegments(nt);
  //display.showNumberDec(3, false);
}
  
