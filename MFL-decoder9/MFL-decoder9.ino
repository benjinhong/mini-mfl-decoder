/*
Version 9 is a new prototype with a buffer system that fires data over bluetooth every 500ms.
*/

#include <mcp_can.h>
#include <SPI.h>
#include <SoftwareSerial.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

char rangeTx[5] = "R000";
char gearTx[4] = "G00";
char oilTx[4] = "O00";
char TPMSTx[14] = "T00P00P00P00P";
char ignTx[3] = "I0";


unsigned long lastTime_phone = 0;
unsigned long timeDiff_phone = 0;

unsigned long lastTime_dict = 0;
unsigned long timeDiff_dict = 0;

unsigned long currTimeTx = 0;
const short int txInterval = 250;

bool disableFlag = false;
bool timeoutEnable = false;
int buttonDelay = 130;

bool debug = 1;
bool listenOnly = 1;  // ALWAYS 1 for when in the car
short int range = 0;
short int lastRange = 0;
short int rangeDelta = 0;
bool ignitionFlag = 0;
char lastGear = 0;
short int lastOilTemp = 0;

#define PHONE_FLAG_PIN 3
#define MCP2515_INT_PIN 2
#define PLAYPAUSE_PIN 7
#define NEXT_PIN 4
#define PREVIOUS_PIN 8
#define PHONE_FLAG_PIN 9
#define BT_TX 6
#define BT_RX 5

MCP_CAN CAN0(10);  // Set CS to pin 10

SoftwareSerial bt = SoftwareSerial(BT_RX, BT_TX);

void setup() {
  Serial.begin(115200);
  bt.begin(9600);
  if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.print("MCP2515 Init Success\r\n");
  else Serial.print("MCP2515 Init Failed\r\n");
  

  pinMode(MCP2515_INT_PIN, INPUT);  // Setting pin 2 for /INT input
  pinMode(PHONE_FLAG_PIN, OUTPUT);

  CAN0.init_Mask(0, 0, 0x07FF0000);  // Init first mask. Only listen to these from 1xx
  CAN0.init_Filt(0, 0, 0x01F70000);  // Init first filter: MFL up/ok/down
  CAN0.init_Filt(1, 0, 0x01D60000);  // Init second filter: MFL dict/phone

  CAN0.init_Mask(1, 0, 0x07000000);  // Init second mask. Listen to all IDs that start with the following numbers:
  CAN0.init_Filt(2, 0, 0x03000000);  // Init third filter: 3xx
  CAN0.init_Filt(3, 0, 0x04000000);  // Init fourth filter: 4xx
  CAN0.init_Filt(4, 0, 0x04000000);  // Init fifth filter: 4xx
  CAN0.init_Filt(5, 0, 0x04000000);  // Init sixth filter: 4xx

  if (listenOnly) {
    CAN0.setMode(MCP_LISTENONLY);  // LISTEN ONLY
    if (debug) Serial.println("MCP_LISTENONLY");
  } else {
    CAN0.setMode(MCP_NORMAL);
    if (debug) Serial.println("MCP_NORMAL");
  }

  /*
  Use MCP_NORMAL for debugging and MCP_LISTENONLY for use in car. My analyzer will spam the message until it receives an ACK from MCP2515, hence slow response.
  Do not want to transmit anything on car's bus, so use LISTENONLY for final integration.
  */
  //sprintf(ignTx, "I0");
}

void loop() {
  if (timeoutEnable)
    timeDiff_phone = abs(millis() - lastTime_phone);  // only calculate diff when phone gets pressed.
  if ((timeDiff_phone > 8200) && (timeDiff_phone < 8210) && (lastTime_phone != 0) && timeoutEnable) {
    disableFlag = false;
    timeoutEnable = false;  // timeout will only work if phone button is pressed. this is to avoid timeout resets when phone was never pressed.
    lastTime_phone = 0;
    digitalWrite(PHONE_FLAG_PIN, disableFlag);

    if (debug) {
      Serial.print("timeDiff_phone = ");
      Serial.println(timeDiff_phone);
    }
  }

  if (!digitalRead(2))  // If pin 2 is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);  // Read data: len = data length, buf = data byte(s)
    //printData(rxId, len, rxBuf);
    //==============[ SET/CLEAR IGNORE FLAGS OR MATCH-AND-SET (MUSIC ONLY) ]==============//
    if (disableFlag) {
      if ((rxId == 0x44C) && (rxBuf[0] == 0x10)) {
        disableFlag = false;
        digitalWrite(PHONE_FLAG_PIN, disableFlag);

        if (debug) Serial.println("Home display called, clearing disable flag");
      } else {
        matchAndSet(rxId, rxBuf, disableFlag, lastTime_phone);  // take time but don't do anything

        if (debug) Serial.println("ignoring commands but still noting time");
      }
    } else {
      if ((rxId == 0x1D6) && (rxBuf[0] == 0xF1)) {
        disableFlag = true;
        timeoutEnable = true;
        lastTime_phone = millis();
        digitalWrite(PHONE_FLAG_PIN, disableFlag);

        if (debug) Serial.println("Phone button called, setting disable flag");
      } else {
        matchAndSet(rxId, rxBuf, disableFlag, lastTime_phone);  // normal command parsing
      }
    }
    //==============[ ALL OTHER DATA TO PROCESS ]==============//

    //==============[ RANGE ]==============//
    if (rxId == 0x366) {  // bypasses all checks. if it gets range data (0x366), always process it. used to be inside matchAndSet
      range = (rxBuf[2] << 4) | (rxBuf[1] >> 4);
      if (debug) {
        Serial.print("Range: ");
        Serial.print(range, HEX);
        Serial.print(" = ");
        Serial.print(range);
        Serial.println("mi");
      }
      rangeDelta = range - lastRange;

      sprintf(rangeTx, "%d", range);
      
      lastRange = range;
    }
    //==============[ IGNITION STATUS ]==============//
    if (rxId == 0x438) {
        if ((rxBuf[3] == 0x30) || (rxBuf[3] == 0x00)) {
          if (debug) Serial.println("IGNITION ON SENT");
          sprintf(ignTx, "I1");
        }
        if (rxBuf[3] == 0x3D) {
          if (debug) Serial.println("IGNITION OFF SENT");
          sprintf(ignTx, "I0");
        }
    }
    //==============[ GEAR ]==============//
    if (rxId == 0x3F9) {
      if (debug) {
        Serial.print("Gear: ");
        Serial.println(rxBuf[6], HEX);
      }
      if (rxBuf[6] != lastGear) {

        sprintf(gearTx, "G%02X", rxBuf[6]);
        lastGear = rxBuf[6];
      }
    }

    //==============[ OIL TEMP ]==============//
    if (rxId == 0x3F9) {
      if (debug) {
        Serial.print("Oil Temp: ");
        Serial.println(rxBuf[5], HEX);
      }
      if (rxBuf[5] != lastOilTemp) {

        sprintf(oilTx, "O%02X", rxBuf[5]);
        lastOilTemp = rxBuf[5];
      }
    }

    //==============[ TPMS ]==============//
    if (rxId == 0x36B) {  // TPMS
      if (debug) {
        Serial.print("T");
        Serial.print(rxBuf[0], HEX);
        Serial.print("P");
        Serial.print(rxBuf[2], HEX);
        Serial.print("P");
        Serial.print(rxBuf[4], HEX);
        Serial.print("P");
        Serial.println(rxBuf[6], HEX);
      }
      sprintf(TPMSTx, "T%02XP%02XP%02XP%02XP", rxBuf[0], rxBuf[2], rxBuf[4], rxBuf[6]);
    }

    //==============[ DICTATION BUTTON ]==============//
    if ((rxId == 0x1D6) && (rxBuf[1] == 0xF1)) {      // dictation button
      timeDiff_dict = abs(millis() - lastTime_dict);  //debouncing logic
      if (timeDiff_dict > 250) {                      //must be at least 250ms since last press
        if (debug) {
          Serial.print("Dictation button pressed. ms since last press: ");
          Serial.println(timeDiff_dict);
        }
        bt.print("DDDD");
        lastTime_dict = millis();
      }
    }
  }
  //=============[ MAIN LOOP ]=============// 

  if (millis() >= currTimeTx + txInterval) {
    currTimeTx += txInterval;
    bt.print(rangeTx);
    bt.print(gearTx);
    bt.print(oilTx);
    bt.print(ignTx);
    bt.print(TPMSTx);
    bt.print("\n");
  }

}

void matchAndSet(long unsigned int id, unsigned char buf[], bool flag, unsigned long &time) {
  if ((id == 0x1F7) && (buf[0] == 0x80)) {
    time = millis();  // always update time regardless of mode
    if (!flag) {
      if (debug) Serial.println("UP");
      pinMode(NEXT_PIN, OUTPUT);
      digitalWrite(NEXT_PIN, LOW);
      delay(buttonDelay);
      pinMode(NEXT_PIN, INPUT);
    }
  }
  if ((id == 0x1F7) && (buf[1] == 0xFD)) {
    time = millis();  // always update time regardless of mode
    if (!flag) {
      if (debug) Serial.println("OK");
      pinMode(PLAYPAUSE_PIN, OUTPUT);
      digitalWrite(PLAYPAUSE_PIN, LOW);
      delay(buttonDelay);
      pinMode(PLAYPAUSE_PIN, INPUT);
    }
  }
  if ((id == 0x1F7) && (buf[0] == 0x7E)) {
    time = millis();  // always update time regardless of mode
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
  for (int i = 0; i < length; i++)  // Print each byte of the data
  {
    if (rxBuf[i] < 0x10)  // If data byte is less than 0x10, add a leading zero
    {
      Serial.print("0");
    }
    Serial.print(rxBuf[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}