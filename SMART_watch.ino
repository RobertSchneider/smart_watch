#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_BLE_UART.h"

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16 
#define LOGO16_GLCD_WIDTH  16 

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define ADAFRUITBLE_REQ 9
#define ADAFRUITBLE_RDY 1
#define ADAFRUITBLE_RST 10

unsigned long seconds = 0;
bool displayMsg = false;
int secondsMsg = 0;
int cursorX, cursorY = 64/2-4;

void displayNotification(char*c, int len, int offset);

struct Packet
{
  char iD;
  char len;
};

Adafruit_BLE_UART BTLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

void setup()   {                
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  
  Serial.println(F("Adafruit Bluefruit Low Energy nRF8001 Print echo demo"));
  BTLEserial.setDeviceName("WATCH");
  BTLEserial.begin();
  display.setRotation(0);
  display.display();
  delay(200);
  display.clearDisplay();
  display.display();
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

//PACKETS
void processPacket(struct Packet p) 
{
  BTLEserial.pollACI();
  while(!BTLEserial.available());
  char buff[p.len];
  for(int i = 0; i < p.len; i++) {
    while(!BTLEserial.available()) {
      BTLEserial.pollACI();
    }
    buff[i] = BTLEserial.read();
  }
  
  if(p.iD == 2) {
    int32_t x1 = 0; x1 |= (int)buff[0];
    int32_t x2 = 0; x2 |= (int)buff[1];
    int32_t x3 = 0; x3 |= (int)buff[2];
    seconds = x1*60*60+x2*60+x3;
  }else if(p.iD == 1) {
    display.display();
  }else if(p.iD == 3) {
    displayNotification(buff, p.len, 0);
  }
}
void readPacket()
{
  struct Packet packet;
  packet.iD = (char)BTLEserial.read();
  packet.len = (char)BTLEserial.read();
  Serial.print("piD: "); Serial.println((int)packet.iD);
  Serial.print("pLe: "); Serial.println((int)packet.len);
  processPacket(packet);
}

aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;
unsigned long lastmillis = 0;
void loop() {
  unsigned long m = millis();
  if(m-lastmillis >= 1000) {
    clock();
    lastmillis = m;
  }
  
  if(!BTLEserial.available()) BTLEserial.pollACI();
  if (BTLEserial.getState() == ACI_EVT_CONNECTED) {
    if(BTLEserial.available() > 0) readPacket();
  }
}

void drawInt(int i, int x, int y, int fs)
{
  if(i == 0) {
    display.drawChar(x+fs*8, y, '0', WHITE, BLACK, fs);
    display.drawChar(x, y, '0', WHITE, BLACK, fs);
    return;
  }
  int sCopy = i;
  int len = 0; while(sCopy > 0) {sCopy/=10; len++;} sCopy = i;
  int len2 = len;
  len = 2;
  while(sCopy > 0) {
    char c = '0'+(sCopy%10);
    len--;
    display.drawChar(x+len*fs*8, y, c, WHITE, BLACK, fs);
    sCopy /= 10;
    if(sCopy == 0 && len2 < 2) display.drawChar(x+(len-1)*fs*8, y, '0', WHITE, BLACK, fs);
  }
}

void hideMsg()
{
  displayMsg = false;
  cursorY = 64/2-4;
}

void clock()
{
  seconds++;
  int y = 64/2-4;
  int x = 64-8;
  if(secondsMsg > 0) secondsMsg--;
  if(displayMsg) if(secondsMsg == 0) hideMsg(); else return;
  display.clearDisplay();
  display.drawChar(x+2*8, y, ':', WHITE, BLACK, 1);
  display.drawChar(x-8, y, ':', WHITE, BLACK, 1);    
  drawInt(seconds/60/60, x-3*8, y, 1);
  drawInt((seconds/60)%60, x, y, 1);
  drawInt(seconds%60, x+3*8, y, 1);
  display.display();
}

void displayNotification(char*c, int len, int offset)
{
  secondsMsg = 5;
  bool prev = displayMsg;
  displayMsg = true;
  if(!prev) display.clearDisplay();
  
  display.setCursor(0, 0);
  int x = 128/2-(len/2+offset)*8;
  int y = cursorY;
  if(x < 0) x = 0;
  for(int i = offset; i <= len; i++) {
    display.drawChar(x, y, c[i], WHITE, BLACK, 1);
    x+=8;
    if(x >= 128) {
      int len2 = len-i-offset-1;
      x = 128/2-(len/2+offset)*8;
      y += 8;
      if(x < 0) x = 0;
    }
  } 
  cursorY = y;
  display.display();
}
