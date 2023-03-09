#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

unsigned long lastTime = 0;
unsigned long currTime = 0;
unsigned long timeDiff = 0;

bool disableFlag = false;
bool timeoutEnable = false;

bool debug = 1;
int buttonDelay = 70;

#define PLAYPAUSE_PIN 7
#define NEXT_PIN 4
#define PREVIOUS_PIN 8
#define PHONE_FLAG_PIN 9

MCP_CAN CAN0(10);                          // Set CS to pin 10

void setup() {
  Serial.begin(115200);
  if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.print("MCP2515 Init Success\r\n");
  else Serial.print("MCP2515 Init Failed\r\n");
  pinMode(2, INPUT);                       // Setting pin 2 for /INT input
  pinMode(PHONE_FLAG_PIN, OUTPUT);
  pinMode(NEXT_PIN, OUTPUT);
  pinMode(PLAYPAUSE_PIN, OUTPUT);
  pinMode(PREVIOUS_PIN, OUTPUT);

  CAN0.init_Mask(0, 0, 0x07FF0000);                // Init first mask
  CAN0.init_Filt(0, 0, 0x03660000);                // Init first filter: range
  CAN0.init_Filt(1, 0, 0x00F30000);                // Init second filter: gear
  
  CAN0.init_Mask(1, 0, 0x07FF0000);                // Init second mask
  CAN0.init_Filt(2, 0, 0x044C0000);                // Init third filter: display calls
  CAN0.init_Filt(3, 0, 0x01D60000);                // Init fourth filter: dictation and phone
  CAN0.init_Filt(4, 0, 0x01F70000);                // Init fifth filter: up/ok/down
  CAN0.init_Filt(5, 0, 0x01F70000);                // Init sixth filter: same to disable

  /*CAN0.init_Mask(0, 0, 0x07FF0000);                // Init first mask
  CAN0.init_Filt(0, 0, 0x03660000);                // Init first filter: range
  CAN0.init_Filt(1, 0, 0x03660000);                // Init second filter: gear
  
  CAN0.init_Mask(1, 0, 0xFFFFFFFF);                // Init second mask
  CAN0.init_Filt(2, 0, 0x00000000);                // Init third filter: display calls
  CAN0.init_Filt(3, 0, 0x00000000);                // Init fourth filter: dictation and phone
  CAN0.init_Filt(4, 0, 0x00000000);                // Init fifth filter: up/ok/down
  CAN0.init_Filt(5, 0, 0x00000000);                // Init sixth filter: same to disable*/

  CAN0.setMode(MCP_LISTENONLY);                // LISTEN ONLY
  //CAN0.setMode(MCP_NORMAL);                

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
    }
}

void matchAndSet(long unsigned int id, unsigned char buf[], bool flag, unsigned long &time) {
    if ((id == 0x1F7) && (buf[0] == 0x80)) {
        time = millis();                          // always update time regardless of mode
        if (!flag) {
          if (debug) Serial.println("UP");
          digitalWrite(NEXT_PIN, 1);
          delay(buttonDelay);
          digitalWrite(NEXT_PIN, 0);
        }
    } 
    if ((id == 0x1F7) && (buf[1] == 0xFD)) {
        time = millis();                          // always update time regardless of mode
        if (!flag) {
          if (debug) Serial.println("OK");
          digitalWrite(PLAYPAUSE_PIN, 1);
          delay(buttonDelay);
          digitalWrite(PLAYPAUSE_PIN, 0);
        }
    } 
    if ((id == 0x1F7) && (buf[0] == 0x7E)) {
        time = millis();                          // always update time regardless of mode
        if (!flag) {
          if (debug) Serial.println("DOWN");
          digitalWrite(PREVIOUS_PIN, 1);
          delay(buttonDelay);
          digitalWrite(PREVIOUS_PIN, 0);
        }
    }

    if (id == 0x366) {
        time = millis();                          // always update time regardless of mode
        if (!flag) {
          if (debug) Serial.print("RANGE: ");

          short int x = (buf[2] << 4) | (buf[1] >> 4);
          Serial.print(x, HEX);
          Serial.print(" = ");
          Serial.print(x);
          Serial.println("mi");
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



