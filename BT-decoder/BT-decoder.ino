#include <SoftwareSerial.h>
#include <TM1637Display.h>

#define CLK 2
#define DIO 3
#define BT_TX 6
#define BT_RX 5

SoftwareSerial bt = SoftwareSerial(BT_RX, BT_TX);

const unsigned char recvBufSize = 5;
unsigned short int range; // 0 ~ 65535
unsigned char gear;       // 0 ~ 255
bool displayFlag = false;

char string[recvBufSize];

void setup() {
  Serial.begin(9600);
  bt.begin(9600);
}

void loop() {
  if (bt.available() > 0) {
    //==============[ FILL BUFFER ]==============//
    for (int i = 0; i < recvBufSize; i++) {
      string[i] = bt.read();
      delay(1);
    }
    //==============[ SHOW BUFFER ]==============//
    Serial.print("Received: ");
    for (int j = 0; j < recvBufSize; j++) {
      Serial.print(string[j]);
    }
    Serial.println();
    //================[ PARSING ]================//
    //Serial.println(strtol(string, NULL, 16)); // for hexadecimal
    if (string[0] == 'R') {
      range = atoi(&string[1]); //ignore first character and parse the rest into (int) range
      Serial.print("range: ");
      Serial.println(range);
    }
    if (string[0] == 'G') {
      gear = strtol(&string[1], NULL, 16);
      Serial.print("gear: ");
      Serial.println(gear);
    }
    if (string[0] == 'D') {
      displayFlag = !displayFlag;
      Serial.print("displayFlag: ");
      Serial.println(displayFlag);
    }
  }
  //================[ MAIN LOOP ]================//
  

}
  
