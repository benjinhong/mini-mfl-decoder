/*
Version 5 is the latest working version without OLED splash screen. Includes (basic) range logic. Direct pin control.
*/

#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

unsigned long lastTime = 0;
unsigned long currTime = 0;
unsigned long timeDiff = 0;

bool disableFlag = false;
bool timeoutEnable = false;
int buttonDelay = 130;

bool debug = 0;
bool listenOnly = 0;  // ALWAYS 1 for when in the car
short int range = 0;
short int lastRange = 0;
short int rangeDelta = 0;

#define PHONE_FLAG_PIN 3
#define MCP2515_INT_PIN 2
#define PLAYPAUSE_PIN 7
#define NEXT_PIN 4
#define PREVIOUS_PIN 8
#define PHONE_FLAG_PIN 9
#define BT_TX 6
#define BT_RX 5

MCP_CAN CAN0(10);                          // Set CS to pin 10

SoftwareSerial bt = SoftwareSerial(BT_RX, BT_TX);

void setup() {
  Serial.begin(115200);
  bt.begin(115200);
  if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.print("MCP2515 Init Success\r\n");
  else Serial.print("MCP2515 Init Failed\r\n");

  pinMode(MCP2515_INT_PIN, INPUT);                       // Setting pin 2 for /INT input
  pinMode(PHONE_FLAG_PIN, OUTPUT);

  CAN0.init_Mask(0, 0, 0x07FF0000);                // Init first mask
  CAN0.init_Filt(0, 0, 0x03660000);                // Init first filter: range
  CAN0.init_Filt(1, 0, 0x03F90000);                // Init second filter: gear (byte 6)
  
  CAN0.init_Mask(1, 0, 0x07FF0000);                // Init second mask
  CAN0.init_Filt(2, 0, 0x044C0000);                // Init third filter: display calls
  CAN0.init_Filt(3, 0, 0x01D60000);                // Init fourth filter: dictation and phone
  CAN0.init_Filt(4, 0, 0x01F70000);                // Init fifth filter: up/ok/down
  CAN0.init_Filt(5, 0, 0x036B0000);                // Init sixth filter: TPMS

  /*CAN0.init_Mask(0, 0, 0x07FF0000);                // Init first mask
  CAN0.init_Filt(0, 0, 0x03660000);                // Init first filter: range
  CAN0.init_Filt(1, 0, 0x03660000);                // Init second filter: gear
  
  CAN0.init_Mask(1, 0, 0xFFFFFFFF);                // Init second mask
  CAN0.init_Filt(2, 0, 0x00000000);                // Init third filter: display calls
  CAN0.init_Filt(3, 0, 0x00000000);                // Init fourth filter: dictation and phone
  CAN0.init_Filt(4, 0, 0x00000000);                // Init fifth filter: up/ok/down
  CAN0.init_Filt(5, 0, 0x00000000);                // Init sixth filter: same to disable*/

  if (listenOnly) {
    CAN0.setMode(MCP_LISTENONLY);                // LISTEN ONLY
    if (debug) Serial.println("MCP_LISTENONLY");
  } else {
    CAN0.setMode(MCP_NORMAL);      
    if (debug) Serial.println("MCP_NORMAL");
  }          

  /*
  Use MCP_NORMAL for debugging and MCP_LISTENONLY for use in car. My analyzer will spam the message until it receives an ACK from MCP2515, hence slow response.
  Do not want to transmit anything on car's bus, so use LISTENONLY for final integration.
  */
}

void loop() {
    if (timeoutEnable)
        timeDiff = abs(millis() - lastTime);    // only calculate diff when phone gets pressed.
    if ((timeDiff > 8200) && (timeDiff < 8210) && (lastTime != 0) && timeoutEnable) {
        disableFlag = false;
        timeoutEnable = false;                // timeout will only work if phone button is pressed. this is to avoid timeout resets when phone was never pressed. 
        lastTime = 0;
        digitalWrite(PHONE_FLAG_PIN, disableFlag);

        if (debug) {
            Serial.print("timeDiff = ");
            Serial.println(timeDiff);
        }
    }

    if(!digitalRead(2))                    // If pin 2 is low, read receive buffer
    {
      CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
      //printData(rxId, len, rxBuf);
      if (disableFlag) {
        if ((rxId == 0x44C) && (rxBuf[0] == 0x10)) {
            disableFlag = false;
            digitalWrite(PHONE_FLAG_PIN, disableFlag);

            if (debug) Serial.println("Home display called, clearing disable flag");
        } else {
            matchAndSet(rxId, rxBuf, disableFlag, lastTime); // take time but don't do anything

            if (debug) Serial.println("ignoring commands but still noting time");
        }
      } else {
        if ((rxId == 0x1D6) && (rxBuf[0] == 0xF1)) {
            disableFlag = true;
            timeoutEnable = true;
            lastTime = millis();
            digitalWrite(PHONE_FLAG_PIN, disableFlag);

            if (debug) Serial.println("Phone button called, setting disable flag");
        } else {
            matchAndSet(rxId, rxBuf, disableFlag, lastTime);  // normal command parsing
        }
      }

      if (rxId == 0x366) {  // bypasses all checks. if it gets range data (0x366), always process it. used to be inside matchAndSet
        range = (rxBuf[2] << 4) | (rxBuf[1] >> 4);
        if (debug) { Serial.print(range, HEX); Serial.print(" = "); Serial.print(range); Serial.println("mi"); }
        // do stuff with range here

        rangeDelta = range - lastRange;
        lastRange = range;
        bt.print("R");
        bt.println(range);
      }

      if (rxId == 0x3F9) {  // gear
        if (debug) { Serial.print("Gear: "); Serial.println(rxBuf[6], HEX); }
        bt.print("G");
        bt.println(rxBuf[6], HEX);
      }

      if (rxId == 0x36B) {  // TMPS
        /*bt.print("TPMS: ");
        bt.print(rxBuf[0], HEX);
        bt.print(" ");
        bt.print(rxBuf[2], HEX);
        bt.print(" ");
        bt.print(rxBuf[4], HEX);
        bt.print(" ");
        bt.print(rxBuf[6], HEX);
        bt.println(" ");*/
      }

      if ((rxId == 0x1D6) && (rxBuf[1] == 0xF1)) {
        if (debug) Serial.println("Dictation button pressed");
        bt.println("DICT");
      }

    }
}

void matchAndSet(long unsigned int id, unsigned char buf[], bool flag, unsigned long &time) {
    if ((id == 0x1F7) && (buf[0] == 0x80)) {
        time = millis();                          // always update time regardless of mode
        if (!flag) {
          if (debug) Serial.println("UP");
          pinMode(NEXT_PIN, OUTPUT);
          digitalWrite(NEXT_PIN, LOW);
          delay(buttonDelay);
          pinMode(NEXT_PIN, INPUT);
        }
    } 
    if ((id == 0x1F7) && (buf[1] == 0xFD)) {
        time = millis();                          // always update time regardless of mode
        if (!flag) {
          if (debug) Serial.println("OK");
          pinMode(PLAYPAUSE_PIN, OUTPUT);
          digitalWrite(PLAYPAUSE_PIN, LOW);
          delay(buttonDelay);
          pinMode(PLAYPAUSE_PIN, INPUT);
        }
    } 
    if ((id == 0x1F7) && (buf[0] == 0x7E)) {
        time = millis();                          // always update time regardless of mode
        if (!flag) {
          if (debug) Serial.println("DOWN");
          pinMode(PREVIOUS_PIN, OUTPUT);
          digitalWrite(PREVIOUS_PIN, LOW);
          delay(buttonDelay);
          pinMode(PREVIOUS_PIN, INPUT);
        }
    }
}

void printData(long unsigned int id, unsigned char length, unsigned char buf[]) {
    Serial.print("ID: ");
    Serial.print(rxId, HEX);
    Serial.print(" Data: ");
    for(int i = 0; i<length; i++)           // Print each byte of the data
    {
      if(rxBuf[i] < 0x10)                // If data byte is less than 0x10, add a leading zero
      {
        Serial.print("0");
      }
    Serial.print(rxBuf[i], HEX);
    Serial.print(" ");
    }
    Serial.println();
}