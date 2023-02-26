#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];

MCP_CAN CAN0(10);                          // Set CS to pin 10

void setup()
{
  Serial.begin(115200);
  if(CAN0.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) Serial.print("MCP2515 Init Success\r\n");
  else Serial.print("MCP2515 Init Failed\r\n");
  pinMode(2, INPUT);                       // Setting pin 2 for /INT input

  CAN0.init_Mask(0,0,0xFFFFFFFF);                // Init first mask...
  CAN0.init_Filt(0,0,0x00000000);                // Init first filter...
  CAN0.init_Filt(1,0,0x00000000);                // Init second filter...
  
  CAN0.init_Mask(1,0,0x07FF0000);                // Init second mask... 
  CAN0.init_Filt(2,0,0x044C0000);                // Init third filter...
  CAN0.init_Filt(3,0,0x01D60000);                // Init fourth filter...
  CAN0.init_Filt(4,0,0x01F70000);                // Init fifth filter...
  CAN0.init_Filt(5,0,0x01F70000);                // Init sixth filter...

  CAN0.setMode(MCP_LISTENONLY);                // LISTEN ONLY

  /*
  Use MCP_NORMAL for debugging and MCP_LISTENONLY for use in car. My analyzer will spam the message until it receives an ACK from MCP2515, hence slow response.
  Do not want to transmit anything on car's bus, so use LISTENONLY for final integration.
  */
}

void loop()
{
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
      if((rxId == 0x1D6) && (rxBuf[0] == 0xF1)) {
        Serial.println("PHONE");
      }
      if((rxId == 0x1D6) && (rxBuf[1] == 0xF1)) {
        Serial.println("DICT");
      }
      if((rxId == 0x44C) && (rxBuf[0] == 0x10)) {
        Serial.println("GO HOME");
      }
      if((rxId == 0x1F7) && (rxBuf[0] == 0x80)) {
        Serial.println("UP");
      }
      if((rxId == 0x1F7) && (rxBuf[1] == 0xFD)) {
        Serial.println("OK");
      }
      if((rxId == 0x1F7) && (rxBuf[0] == 0x7E)) {
        Serial.println("DOWN");
      }
      //Serial.println();
    }
}
