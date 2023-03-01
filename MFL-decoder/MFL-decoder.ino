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

MCP_CAN CAN0(10);                          // Set CS to pin 10

void setup()
{
  Serial.begin(115200);
  if(CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.print("MCP2515 Init Success\r\n");
  else Serial.print("MCP2515 Init Failed\r\n");
  pinMode(2, INPUT);                       // Setting pin 2 for /INT input
  pinMode(3, OUTPUT);

  CAN0.init_Mask(0,0,0xFFFFFFFF);                // Init first mask...
  CAN0.init_Filt(0,0,0x00000000);                // Init first filter...
  CAN0.init_Filt(1,0,0x00000000);                // Init second filter...
  
  CAN0.init_Mask(1,0,0x07FF0000);                // Init second mask... 
  CAN0.init_Filt(2,0,0x044C0000);                // Init third filter...
  CAN0.init_Filt(3,0,0x01D60000);                // Init fourth filter...
  CAN0.init_Filt(4,0,0x01F70000);                // Init fifth filter...
  CAN0.init_Filt(5,0,0x01F70000);                // Init sixth filter...

  //CAN0.setMode(MCP_LISTENONLY);                // LISTEN ONLY
  CAN0.setMode(MCP_NORMAL);                

  /*
  Use MCP_NORMAL for debugging and MCP_LISTENONLY for use in car. My analyzer will spam the message until it receives an ACK from MCP2515, hence slow response.
  Do not want to transmit anything on car's bus, so use LISTENONLY for final integration.
  */
}

void loop()
{
    if (timeoutEnable)
      timeDiff = abs(millis() - lastTime);    // only calculate diff when phone gets pressed.
    if ( (timeDiff > 8200) && (timeDiff < 8210) && (lastTime != 0) && timeoutEnable) {
      disableFlag = false;
      lastTime = 0;
      Serial.print("timeDiff = ");
      Serial.println(timeDiff);
      digitalWrite(3, disableFlag);
      timeoutEnable = false;                // timeout will only work if phone button is pressed. this is to avoid timeout resets when phone was never pressed. 
    }


    if(!digitalRead(2))                    // If pin 2 is low, read receive buffer
    {
      CAN0.readMsgBuf(&rxId, &len, rxBuf); // Read data: len = data length, buf = data byte(s)
      /*Serial.print("ID: ");
      Serial.print(rxId, HEX);
      Serial.print(" Data: ");
      for(int i = 0; i<len; i++)           // Print each byte of the data
      {
        if(rxBuf[i] < 0x10)                // If data byte is less than 0x10, add a leading zero
        {
          Serial.print("0");
        }
        Serial.print(rxBuf[i], HEX);
        Serial.print(" ");
      }*/

      if (disableFlag) {
        if ( (rxId == 0x44C) && (rxBuf[0] == 0x10) ) {
          Serial.println("Home display called, clearing disable flag");
          disableFlag = false;
          digitalWrite(3, disableFlag);
        } else {
          //Serial.println("Home display has not been called, still disabled. Ignoring commands");

          if ((rxId == 0x1F7) && (rxBuf[0] == 0x80)) {
            lastTime = millis(); //save current time
            //Serial.println(lastTime);
            //Serial.println("UP - ignored");
          }
          if ((rxId == 0x1F7) && (rxBuf[1] == 0xFD)) {
            lastTime = millis(); //save current time
            //Serial.println(lastTime);
            //Serial.println("OK - ignored");
          }
          if ((rxId == 0x1F7) && (rxBuf[0] == 0x7E)) {
            lastTime = millis(); //save current time
            //Serial.println(lastTime);
            //Serial.println("DOWN - ignored");
          }


        }
      } else {
        if ( (rxId == 0x1D6) && (rxBuf[0] == 0xF1) ) {
          Serial.println("Phone button called, setting disable flag");
          disableFlag = true;
          timeoutEnable = true;
          lastTime = millis(); //save current time
          //Serial.println(lastTime);
          digitalWrite(3, disableFlag);
        } else {
          //Serial.println("Something else was called, matching");
          if ((rxId == 0x1F7) && (rxBuf[0] == 0x80)) {
            lastTime = millis(); //save current time
            //Serial.println(lastTime);
            Serial.println("UP");
          }
          if ((rxId == 0x1F7) && (rxBuf[1] == 0xFD)) {
            lastTime = millis(); //save current time
            //Serial.println(lastTime);
            Serial.println("OK");
          }
          if ((rxId == 0x1F7) && (rxBuf[0] == 0x7E)) {
            lastTime = millis(); //save current time
            //Serial.println(lastTime);
            Serial.println("DOWN");
          }
        }
      }
    }
}
