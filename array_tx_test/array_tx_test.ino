#include <SoftwareSerial.h>


SoftwareSerial bt = SoftwareSerial(5, 6);


char range[5];
char gear[4];
char oil[5];
char TPMS[13];

void setup() {
  Serial.begin(115200);
  bt.begin(9600);
  sprintf(range, "328");
  sprintf(gear, "G25");
  sprintf(oil, "O123");
  sprintf(TPMS, "T72P72P72P72");
}

void loop() {
  bt.print(range);
  bt.print(gear);
  bt.print(oil);
  bt.print(TPMS);
  bt.print("\n");
  delay(1000);
}
