#include <SoftwareSerial.h>
#include <TM1637Display.h>

#define CLK 2
#define DIO 3
#define BT_TX 6
#define BT_RX 5

SoftwareSerial bt = SoftwareSerial(BT_RX, BT_TX);

void setup() {
  Serial.begin(115200);
  bt.begin(115200);
}

void loop() {
}
  
