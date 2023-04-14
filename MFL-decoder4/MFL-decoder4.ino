#include <mcp_can.h>
#include <SPI.h>

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

unsigned long lastTime = 0;
unsigned long currTime = 0;
unsigned long timeDiff = 0;

bool disableFlag = false;
bool timeoutEnable = false;

bool debug = 1;
bool listenOnly = 0;  // ALWAYS 1 for when in the car
int buttonDelay = 85;
short int range = 0;
short int lastRange = 0;
short int rangeDelta = 0;

#define PLAYPAUSE_PIN 7
#define NEXT_PIN 4
#define PREVIOUS_PIN 8
#define PHONE_FLAG_PIN 3

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

MCP_CAN CAN0(10);                          // Set CS to pin 10

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

void setup() {
  Serial.begin(115200);
  if (CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.print("MCP2515 Init Success\r\n");
  else Serial.print("MCP2515 Init Failed\r\n");

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  displayWaiting();

  pinMode(2, INPUT);                       // Setting pin 2 for /INT input
  pinMode(PHONE_FLAG_PIN, OUTPUT);

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
        
        if (rangeDelta != 0) {             // delta persists until it changes
          display.clearDisplay();
          display.drawBitmap(0, 0, fuel_icon, 32, 32, 1);
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(50, 0); //can manually place "mi" depending on size of range. 1, 2, or 3 digits.
          display.print(range);
          display.println("mi");
          display.display();      //display range

          if (rangeDelta != range) {                  //if delta and range arent the same (shows at boot) then begin showing range
            if (rangeDelta >= 0)  display.print("+"); //display "+" if positive, else "-" if negative
            else                  display.print("-");

            display.setTextSize(2);
            display.setCursor(50, 18);
            display.print(rangeDelta);
            display.println("mi");
            display.display();
          }
        }
      }

      if (rxId == 0x0F3) {  // gear testing area
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

void displayWaiting() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Waiting on");
  display.setCursor(0, 16);
  display.println("CAN bus...");
  display.display();
}



