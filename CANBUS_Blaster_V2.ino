/************************************************************************
** CAN BLASTER - V2.1
**  500Kbps Bus Rate
**  Need at least one device on bus that is not in listen only mode
**    oshw --- Feb 19 2022 (V1)
**     >> updated Jan 2023 (V2.0) && Feb 2023 (V2.1)
**        - added four unique messages with DLC 8,7,2,1
**        - improved text debug messages, option prints
**        - added LED on pin 9 (if desired, toggles at low frame rates)
**        
**        
************************************************************************/


#include <SPI.h>
/* https://github.com/autowp/arduino-mcp2515 */ 
#include <mcp2515.h>

#define READY     1
#define NOT_READY 0

const uint8_t CHIP_SELECT_PIN   = 10;
const uint8_t LED_PIN           = 9;
struct can_frame canMsg1;
struct can_frame canMsg2;
struct can_frame canMsg3;
struct can_frame canMsg4;
MCP2515 mcp2515(CHIP_SELECT_PIN);

uint32_t prevTicks;
uint32_t currentTicks;
/* Debug loop (i.e. print statements) - interval in milliseconds */
uint32_t intervalMS                 = 1000;
/* Default delay between CAN frames */
uint16_t delayUS                    = 16000;
/* serial command - temp holder for delay */
uint16_t delayUSPrevious            = 16000;
/* Minimum frame spacing for debug loop to run (otherwise it can adversely affect the frame rate) */
uint16_t minimumDelayForDebugLoop   = 625;
uint8_t readyToAcceptSerialConfig   = NOT_READY;

char rxd  = '0';
void setup() {
  pinMode(LED_PIN, OUTPUT);

  canMsg1.can_id  = 0x353;
  canMsg1.can_dlc = 8;
  canMsg1.data[0] = 0x11;
  canMsg1.data[1] = 0x22;
  canMsg1.data[2] = 0x33;
  canMsg1.data[3] = 0x44;
  canMsg1.data[4] = 0xDE;
  canMsg1.data[5] = 0xAD;
  canMsg1.data[6] = 0xBE;
  canMsg1.data[7] = 0xEF;

  canMsg2.can_id  = 0x2DA;
  canMsg2.can_dlc = 7;
  canMsg2.data[0] = 0x25;
  canMsg2.data[1] = 0x26;
  canMsg2.data[2] = 0x27;
  canMsg2.data[3] = 0x28;
  canMsg2.data[4] = 0x29;
  canMsg2.data[5] = 0x30;
  canMsg2.data[6] = 0x31;
  canMsg2.data[7] = 0x32;

  canMsg3.can_id  = 0x369;
  canMsg3.can_dlc = 2;
  canMsg3.data[0] = 0xAA;
  canMsg3.data[1] = 0xBB;
  canMsg3.data[2] = 0xCC;
  canMsg3.data[3] = 0xDD;

  canMsg4.can_id  = 0x005;
  canMsg4.can_dlc = 1;
  canMsg4.data[0] = 0xA5;
  canMsg4.data[1] = 0xA5;
  canMsg4.data[2] = 0xA5;
  canMsg4.data[3] = 0xA5;

  while (!Serial);
  Serial.begin(115200);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  delayUS = 16000;
  Serial.println(F("\n\nCANBUS Message Blaster file: CAN_write_PH_ex2 - Test Mode - V2.1"));
  Serial.println(F("  - Pins to MCP2515 Module ->>   D13=SCLK,  D12=SO,  D11=SI,  D10=CS"));
  Serial.println(F("    send x (serial cmd) at any time to change the message rate..."));
  Serial.println(F("    send i (serial cmd) at any time to get the current message rate..."));
  Serial.println(F("    MCP2515 CANBUS Controller IC, with an Arduino Nano Host."));
  Serial.print(F("\n\n default delay:   "));
  Serial.print(delayUS * 2);
  Serial.println(F("us."));
  Serial.print(F(" msg Rate (approx):   "));
  Serial.print(1.0 / float(delayUS * 2) * 1000000.0);
  Serial.println(F(" CANBUS frames per second..."));
  Serial.println(F("\n\nMessages to be sent at regular intervals: "));
  Serial.print(F("ID (DEC): "));
  Serial.print(canMsg1.can_id);
  Serial.print(F("\tID (HEX): "));
  Serial.print(canMsg1.can_id, HEX);
  Serial.print(F(" \tDLC: "));
  Serial.print(canMsg1.can_dlc);
  Serial.print(F(" \tDATA (HEX): "));
  Serial.print(canMsg1.data[0], HEX);
  Serial.print(canMsg1.data[1], HEX);
  Serial.print(canMsg1.data[2], HEX);
  Serial.print(canMsg1.data[3], HEX);
  Serial.print(canMsg1.data[4], HEX);
  Serial.print(canMsg1.data[5], HEX);
  Serial.print(canMsg1.data[6], HEX);
  Serial.print(canMsg1.data[7], HEX);
  Serial.println(" ");
  Serial.print(F("ID (DEC): "));
  Serial.print(canMsg2.can_id);
  Serial.print(F("\tID (HEX): "));
  Serial.print(canMsg2.can_id, HEX);
  Serial.print(F(" \tDLC: "));
  Serial.print(canMsg2.can_dlc);
  Serial.print(F(" \tDATA (HEX): "));
  Serial.print(canMsg2.data[0], HEX);
  Serial.print(canMsg2.data[1], HEX);
  Serial.print(canMsg2.data[2], HEX);
  Serial.print(canMsg2.data[3], HEX);
  Serial.print(canMsg2.data[4], HEX);
  Serial.print(canMsg2.data[5], HEX);
  Serial.print(canMsg2.data[6], HEX);
  //Serial.print(canMsg2.data[7], HEX);
  Serial.println(" ");
  Serial.print(F("ID (DEC): "));
  Serial.print(canMsg3.can_id);
  Serial.print(F("\tID (HEX): "));
  Serial.print(canMsg3.can_id, HEX);
  Serial.print(F(" \tDLC: "));
  Serial.print(canMsg3.can_dlc);
  Serial.print(F(" \tDATA (HEX): "));
  Serial.print(canMsg3.data[0], HEX);
  Serial.print(canMsg3.data[1], HEX);
  //Serial.print(canMsg3.data[2], HEX);
  //Serial.print(canMsg3.data[3], HEX);
  Serial.println(" ");
  Serial.print(F("ID (DEC): "));
  Serial.print(canMsg4.can_id);
  Serial.print(F("\tID (HEX): "));
  Serial.print(canMsg4.can_id, HEX);
  Serial.print(F(" \tDLC: "));
  Serial.print(canMsg4.can_dlc);
  Serial.print(F(" \tDATA (HEX): "));
  Serial.print(canMsg4.data[0], HEX);
  //Serial.print(canMsg4.data[1], HEX);
  //Serial.print(canMsg4.data[2], HEX);
  //Serial.print(canMsg4.data[3], HEX);
  Serial.println(F("\n\n ...Transmitting frames now..."));
}

void loop() {
  mcp2515.sendMessage(&canMsg1);
  delayMicroseconds(delayUS);
  delayMicroseconds(delayUS);
  mcp2515.sendMessage(&canMsg2);
  delayMicroseconds(delayUS);
  delayMicroseconds(delayUS);
  mcp2515.sendMessage(&canMsg3);
  delayMicroseconds(delayUS);
  delayMicroseconds(delayUS);
  mcp2515.sendMessage(&canMsg4);
  delayMicroseconds(delayUS);
  delayMicroseconds(delayUS);
  currentTicks = millis();
  if (delayUS >= minimumDelayForDebugLoop)
  {
    if ((currentTicks - prevTicks) >= intervalMS)
    {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      prevTicks = currentTicks;
      Serial.print(F("Up(sec): "));
      Serial.println(currentTicks/1000);
    }
  }
  rxd = Serial.read();
  if (rxd == 'i') {
    Serial.print(F("Current Rate Info: "));
    Serial.print(delayUS * 2);
    Serial.println(F("us,  message frame spacing"));
  }
  if (rxd == 'x')
  {
    Serial.println(F("New rate? -- 0 to 9, slow to fast...  'q' to quit"));
    Serial.println(F(" 0 = ~31   frames per sec"));
    Serial.println(F(" 9 = ~4000 frames per sec"));
    Serial.println(F("speed selection will take place as soon as the next character (0~9) is received...."));
    Serial.println(F("   until a selection has been made - no CAN frames are transmitted... printing options now."));
    readyToAcceptSerialConfig = NOT_READY;
    if (Serial.available()) {
      delayUSPrevious = delayUS;
      while (1)
      {
        if (readyToAcceptSerialConfig == READY)
        {
          rxd = Serial.read();
          delayUS = delayUSPrevious;
        }
        
        if (rxd == '0' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 16000;
          Serial.print(F("data rate = 0  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == '1' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 10000;

          Serial.print(F("data rate = 1  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == '2' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 5000;
          Serial.print(F("data rate = 2  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == '3' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 2500;
          Serial.print(F("data rate = 3  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == '4' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 1250;
          Serial.print(F("data rate = 4  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == '5' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 625;
          Serial.print(F("data rate = 5  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == '6' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 313;
          Serial.print(F("data rate = 6  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == '7' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 156;
          Serial.print(F("data rate = 7  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == '8' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 78;
          Serial.print(F("data rate = 8  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == '9' || readyToAcceptSerialConfig == NOT_READY) {
          delayUS = 64;
          Serial.print(F("data rate = 9  ---- delay in us:  "));
          Serial.println(delayUS * 2);
          if (readyToAcceptSerialConfig == READY)
          {
            break;
          }
        }
        if (rxd == 'q'){
          break;
        }
        readyToAcceptSerialConfig = READY;
      }
      Serial.println(F(" \n$$ New Frame Rate Set $$"));
      Serial.print(delayUS * 2);
      Serial.println(F("us,   new delay between CAN frames... \n"));
    }
  }
}
