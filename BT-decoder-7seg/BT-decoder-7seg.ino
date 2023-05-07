#include <SoftwareSerial.h>
#include <TM1637Display.h>

#define CLK 2
#define DIO 3
#define BT_TX 6
#define BT_RX 5

SoftwareSerial bt = SoftwareSerial(BT_RX, BT_TX);
TM1637Display display(CLK, DIO);

const unsigned char recvBufSize = 5;
unsigned short int range; // 0 ~ 65535
unsigned char gear;       // 0 ~ 255
bool displayFlag = 1;     // what to display at startup
bool debug = 1;           // enable/disable debug messages
char buffer[recvBufSize]; // second serial buffer

const unsigned char nt[] =        {0x40, 0x54, 0x78, 0x40};
const unsigned char rr[] =        {0x40, 0x50, 0x50, 0x40};
const unsigned char stop[] =      {0x40, 0x40, 0x40, 0x40};
const unsigned char on[] =        {0x40, 0x5C, 0x54, 0x40};
const unsigned char p[] =         {0x40, 0x00, 0x73, 0x40};
const unsigned char unk[] =       {0x40, 0x46, 0x70, 0x40};
const unsigned char clear[] =     {0x00, 0x00, 0x00, 0x00};
const unsigned char all[] =       {0xFF, 0xFF, 0xFF, 0xFF};
unsigned char startupAnim[] =     {0x00, 0x00, 0x00, 0x00};
unsigned char dispGear[] =        {0x40, 0x00, 0x70, 0x40};

void setup() {
  Serial.begin(9600);
  bt.begin(9600);
  display.setBrightness(7);

  for (int i = 0; i < 4; i++) {
    startupAnim[i] = 0x40;
    display.setSegments(startupAnim);
    delay(50);
  }
  display.setSegments(all);
  delay(100);

  while(bt.available() == 0) {
    display.setSegments(stop);
    delay(1E3);
    display.setSegments(clear);
    delay(1E3);
  }
}

void loop() {
  if (bt.available() > 0) {
    //==============[ FILL BUFFER ]==============//
    for (int i = 0; i < recvBufSize; i++) {
      buffer[i] = bt.read();
      delay(1);
    }
    //==============[ SHOW BUFFER ]==============//
    if (debug) {
      Serial.print("Received: ");
      for (int j = 0; j < recvBufSize; j++) {
        Serial.write(buffer[j]);
      }
      Serial.println();
    }
    //================[ PARSING ]================//
    //Serial.println(strtol(buffer, NULL, 16)); // for hexadecimal
    if (buffer[0] == 'R') {
      range = atoi(&buffer[1]); //ignore first character and parse the rest into (int) range
      if (debug) {Serial.print("range: "); Serial.println(range); }
    }
    if (buffer[0] == 'G') {
      gear = strtol(&buffer[1], NULL, 16);
      if (debug) {Serial.print("gear: "); Serial.println(gear); }
    }
    if (buffer[0] == 'D') {
      displayFlag = !displayFlag;
      if (debug) {Serial.print("displayFlag: "); Serial.println(displayFlag); }
    }
  }
  //================[ MAIN LOOP ]================//
  if(displayFlag) {
    // show range
    display.showNumberDec(range, false);
  } else {
    // show gear
    if (gear >= 5 && gear <= 10) {  // M gears
      dispGear[2] = display.encodeDigit(gear-4);
      display.setSegments(dispGear);
    } else
    if (gear == 1) display.setSegments(nt); // neutral
    else
    if (gear == 2) display.setSegments(rr); // reverse
    else
    if (gear == 3) display.setSegments(p); // ignition off in park
    else
    if (gear == 35) display.setSegments(on); // ignition on in park
    else
    if (gear == 37 || gear == 0) display.setSegments(stop); // engine stop from start/stop
    else display.setSegments(unk);
    
  }
}
  
