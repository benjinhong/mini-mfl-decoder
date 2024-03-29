#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <TM1637Display.h>

#define CLK 2
#define DIO 3
#define BT_TX 6
#define BT_RX 5

SoftwareSerial bt = SoftwareSerial(BT_RX, BT_TX);
TM1637Display segment(CLK, DIO);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static const unsigned char PROGMEM fuel_icon [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x09, 0x80,
0x00, 0x00, 0xF8, 0x00, 0x00, 0x03, 0xFC, 0x00, 0x00, 0x07, 0xFE, 0x00, 0x00, 0x1F, 0xFC, 0x00,
0x00, 0x3F, 0xFC, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x3F, 0xFC, 0x00, 0x00, 0x7F, 0xFE, 0x00,
0x00, 0x60, 0xFE, 0x00, 0x00, 0xC0, 0x7E, 0x00, 0x01, 0x80, 0x3C, 0x00, 0x03, 0x01, 0xF8, 0x00,
0x06, 0x00, 0x30, 0x00, 0x0C, 0x04, 0x20, 0x00, 0x1C, 0x00, 0x40, 0x00, 0x34, 0x08, 0x80, 0x00,
0x62, 0x01, 0x00, 0x00, 0x61, 0x12, 0x00, 0x00, 0x60, 0x84, 0x00, 0x00, 0xE0, 0x38, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM temp_icon [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00,
0x7E, 0xFC, 0x00, 0x00, 0x00, 0xFC, 0x0F, 0x8E, 0x0E, 0xFC, 0x19, 0xCA, 0x00, 0xFC, 0x70, 0xCE,
0x7E, 0xFC, 0x60, 0x00, 0x00, 0xFC, 0x60, 0x00, 0x0E, 0xEC, 0x60, 0x00, 0x00, 0xCC, 0x60, 0x00,
0x7E, 0xCC, 0x60, 0x00, 0x00, 0xCC, 0x70, 0xC0, 0x0E, 0xCC, 0x19, 0xC0, 0x00, 0xCC, 0x0F, 0x80,
0x7E, 0xCC, 0x00, 0x00, 0x00, 0xCC, 0x00, 0x00, 0x01, 0xCE, 0x00, 0x00, 0x03, 0xCF, 0x00, 0x00,
0x03, 0x87, 0x00, 0x00, 0x03, 0x87, 0x00, 0x00, 0x03, 0x87, 0x00, 0x00, 0x03, 0xC7, 0x00, 0x00,
0x01, 0xFE, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM MINI [] = {
  0x00, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xf0, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x3f, 0x00, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x1f, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 
	0x00, 0x03, 0xc0, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xff, 0xff, 0x80, 
	0xff, 0xff, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xff, 0xff, 0x80, 0x7f, 0xff, 0xff, 0x80, 0x00, 0x00, 
	0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x07, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0x00, 0x00, 0x00, 0x7f, 0xff, 
	0xf0, 0x00, 0x07, 0xff, 0xfe, 0x20, 0x92, 0x12, 0x3f, 0xff, 0xf0, 0x00, 0x03, 0xff, 0xfe, 0x31, 
	0x93, 0x12, 0x3f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x0e, 0x2a, 0x92, 0x92, 0x38, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x0e, 0x24, 0x92, 0x52, 0x38, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 0x20, 0x92, 0x32, 
	0x3f, 0xfe, 0x00, 0x00, 0x00, 0x3f, 0xfe, 0x20, 0x92, 0x12, 0x3f, 0xfe, 0x00, 0x00, 0x00, 0x1f, 
	0xff, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x70, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x80, 
	0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 
	0x00, 0x00, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x01, 0xe0, 0x00, 0x03, 
	0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x7c, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x00, 0x7e, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 
	0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00
};

static const unsigned char PROGMEM axle [] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x81, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xFF, 0xFF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

unsigned char segmentArray[] = {0x00, 0x00, 0x00, 0x00};
unsigned char startupAnim[] =     {0x00, 0x00, 0x00, 0x00};
const unsigned char border[] =       {0x39, 0x09, 0x09, 0x0F};
const unsigned char clear[] =     {0x00, 0x00, 0x00, 0x00};

const unsigned char recvBufSize = 40; //27
unsigned short int range; // 0 ~ 65535
unsigned char gear;       // 0 ~ 255
unsigned char oilTemp;    // 0 ~ 255
unsigned char speed;      // 0 ~ 255
unsigned char speedDelayed;
unsigned short int brightness;

unsigned long currTimeSpeed = 0;
unsigned long currTimeBrightness = 0;
const short int brightnessUpdateFreq = 1000;
const short int speedDelay = 500;

bool segmentStartup = 0;

char mode = 0;
char string[recvBufSize];
float psi[4];
bool enable = 0;
bool debug = 0;

void displayWelcome();
void displayTPMS();
void displayRangeTemp();
void displayMisc();
void parseTPMS();
void displaySegments();

void setup() {
  Serial.begin(115200);
  bt.begin(9600);

  segment.setBrightness(7);
  segment.setSegments(clear);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  displayWelcome();

  //delay(1E3);
  //delay(5E3);
  //displayTPMS(0, 0, 0, 0);
}

void loop() {
  if (bt.available() > 0) {
    delay(recvBufSize+1);
    //Serial.println(bt.available());
    //==============[ FILL BUFFER ]==============//
    for (int i = 0; i < recvBufSize; i++) {
      string[i] = bt.read();
    }
    
    //==============[ SHOW BUFFER ]==============//
    if (debug) {
      Serial.print("Received: ");
      for (int j = 0; j < recvBufSize; j++) {
        Serial.print(string[j]);
      }
      Serial.println();
    }
    
    //================[ PARSING ]================//
    
    if (string[0] == 'D' || string[1] == 'D' || string[30] == 'D') {        // the check at index 28 is there because 'D' sometimes shows up
                                                                            // (depending on when pressed) at the end of a normal response string.
      mode++;
      if (mode == 2) mode = 0;

    } else {
      char *token;                                // token pointer
      token = strtok(string, "G");
      range = atoi(token);                        // int to int
      
      token = strtok(NULL, "O");                
      gear = strtol(token, NULL, 16);             // hex to int  
      
      token = strtok(NULL, "S");
      oilTemp = strtol(token, NULL, 16) - 48;          // hex to int

      token = strtok(NULL, "I");
      speed = strtol(token, NULL, 16);          // hex to int

      token = strtok(NULL, "T");
      enable = atoi(token);

      for (int i = 0; i < 4; i++) {
        token = strtok(NULL, "P");
        //Serial.println(token);
        if (token[0] == 'F' && token[1] == 'E') psi[i] = 0;
        else psi[i] = strtol(token, NULL, 16) * 0.1 + 25.6;   // hex to int -> formula
      }

      if (debug) {
        Serial.print(F("Range (i): "));
        Serial.println(range);
        Serial.print(F("Gear (i): "));
        Serial.println(gear);
        Serial.print(F("Oil (C): "));
        Serial.println(oilTemp);
        Serial.print(F("Speed (MPH): "));
        Serial.println(speed);
      }

      /*for (int i = 0; i < 4; i++) {
        Serial.println(psi[i]);
      }*/
      //displayTPMS(psi[0], psi[1], psi[2], psi[3]);
      //displayRangeTemp(range, oilTemp);
      //displayMisc(gear);
    }
  }
  //=============[ MAIN LOOP ]=============//
  if (millis() >= currTimeBrightness + brightnessUpdateFreq) {
      currTimeBrightness += brightnessUpdateFreq;
      brightness = map(analogRead(A0), 50, 512, 0, 6);             // used to be 0, 512, 0, 6. raised floor to 50 because it oscillates between light levels when its still bright outside.
      segment.setBrightness(brightness);
      //Serial.println(analogRead(A0));
    }
  if (enable) {
    if (segmentStartup != 1) {
      for (int i = 0; i < 4; i++) {
        startupAnim[i] = 0x40;
      if (i > 0) startupAnim[i-1] = 0x00;
        segment.setSegments(startupAnim);
        delay(70);
      }
      segment.setSegments(border);
      delay(100);
      segmentStartup = 1;
    }

    switch (mode) {
      case 0:
        displayRangeTemp(range, oilTemp);
        break;
      case 1:
        //parseTPMS(string, psi);
        displayTPMS(psi[0], psi[1], psi[2], psi[3]);
        break;
      case 2:
        displayMisc(gear);
        break;
    }

    if (millis() >= currTimeSpeed + speedDelay) {
      currTimeSpeed += speedDelay;
      speedDelayed = speed;         // copy speed into speedDelayed every (speedDelay) ms.
    }
    
    displaySegments(gear, speedDelayed);  // constantly update 7 segment
        
  } else {
    // do when ignition is 0
    //segment.setSegments(clear);
  }

}

void displayWelcome() {
  randomSeed(analogRead(1));
  char slogan = random(4);
  display.clearDisplay();
  display.drawBitmap(27, 8, MINI, 80, 32, 1);
  display.display();
  delay(500);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  switch (slogan) {
    case 0: 
      display.setCursor(32, 50);
      display.println(F("SMALL WINS."));
      break;
    case 1:
      display.setCursor(23, 50);
      display.println(F("FOR THE DRIVE."));
      break;
    case 2:
      display.setCursor(26, 50);
      display.println(F("KEEP IT WILD."));
      break;
    case 3:
      display.setCursor(29, 50);
      display.println(F("LET'S MOTOR."));
      break;
    default:
      break;
  }
  display.display();
}

void displayTPMS(float FL, float FR, float RL, float RR) {
  display.clearDisplay();
  display.drawBitmap(0, 0, axle, 128, 64, 1);
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);
  if (FL) display.println(FL, 1);
  else display.println("----"); 
  display.setCursor(80, 0);
  if (FR) display.println(FR, 1);
  else display.println("----"); 
  display.setCursor(0, 50);
  if (RL) display.println(RL, 1);
  else display.println("----"); 
  display.setCursor(80, 50);
  if (RR) display.println(RR, 1);
  else display.println("----"); 
  display.display();
}

void displayRangeTemp(unsigned short int range, unsigned char temp) {
  display.clearDisplay();
  // Range
  display.drawBitmap(0, 0, fuel_icon, 32, 32, 1);
  display.setTextSize(2);
  display.setCursor(43, 10);
  display.print(range);
  display.println(F("mi"));   //memory issues with oled, using F() helps?
  // Oil temperature
  display.drawBitmap(0, 35, temp_icon, 32, 32, 1);
  display.setCursor(43, 40);
  display.print(temp, DEC);
  display.println("C");
  // Delta since last range update
  //display.setTextSize(1);
  //display.setCursor(103, 15);
  //display.print("-9");
  
  display.display();
}

void displayMisc(unsigned char gear) {
  display.clearDisplay();
  display.setCursor(45, 10);
  display.setTextSize(4);
  if (gear >= 5 && gear <= 10) {  // M gears
      //display.print("G");
      display.println(gear-4);
    } else
    if (gear == 1) display.println(F("N")); // neutral
    else
    if (gear == 2) display.println(F("R")); // reverse
    else
    if (gear == 3) display.println(F("P")); // ignition off in park
    else
    if (gear == 35) display.println(F("ON")); // ignition on in park
    else
    if (gear == 37 || gear == 0) display.println(F("-")); // engine stop from start/stop
    else display.println(F("?"));
    display.display();
}

void displaySegments(unsigned char gear, unsigned char speed) {
  
  if (speed < 10) {
    segmentArray[0] = segment.encodeDigit(speed);
    segmentArray[1] = 0x00;
  } else {
    segmentArray[0] = segment.encodeDigit(speed / 10);
    segmentArray[1] = segment.encodeDigit(speed % 10);
  }
  
  if (gear >= 5 && gear <= 10) {  // M gears
      segmentArray[3] = segment.encodeDigit(gear-4);
    } else
    if (gear == 1 || gear == 33) segmentArray[3] = 0x54; // neutral
    else
    if (gear == 2 || gear == 34) segmentArray[3] = 0x50; // reverse
    else
    if (gear == 35 || gear == 3 || gear == 99) segmentArray[3] = 0x73; // park
    else
    if (gear == 37 || gear == 0) segmentArray[3] = 0x40; // engine stop from start/stop

  segment.setSegments(segmentArray);
}


