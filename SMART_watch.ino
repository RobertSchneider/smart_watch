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
int32_t seconds = 0;

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

aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;
void loop() {
  clock();
  BTLEserial.pollACI();
  aci_evt_opcode_t status = BTLEserial.getState();
  if (status != laststatus) {
    if (status == ACI_EVT_DEVICE_STARTED) {
        Serial.println(F("* Advertising started"));
    }
    if (status == ACI_EVT_CONNECTED) {
        Serial.println(F("* Connected!"));
    }
    if (status == ACI_EVT_DISCONNECTED) {
        Serial.println(F("* Disconnected or advertising timed out"));
    }
    laststatus = status;
  }

  if (status == ACI_EVT_CONNECTED) {
    if (BTLEserial.available()) {
      Serial.print("* "); Serial.print(BTLEserial.available()); Serial.println(F(" bytes available from BTLE"));
    }
    char c[4]; int cnt = 0;
    while (BTLEserial.available()) {
      char c2 = BTLEserial.read();
      if(cnt <= 3) {
        c[cnt] = c2;
      }
      cnt++;
    }
    if(cnt == 5)
    {
      char cmd = c[0];
      if(cmd == 1) display.display();
      if(cmd == 2) 
      {
        int32_t t = 0;
        t = c[1]<<8;
        t <<= 8;
        t |= c[2]<<8;
        t |= c[3];
        seconds = t;
        Serial.println((int)c[1]);
        Serial.println((int)c[2]);
        Serial.println((int)c[3]);
        Serial.println(seconds);
      }
    }else {
      char x = c[1];
      char y = c[2];
      char d = c[3];
      char d2 = c[4];
      display.setData(x, (int16_t)y, (int)(d<<8)+d2);
    } 
    
    if (Serial.available()) {
      Serial.setTimeout(100); // 100 millisecond timeout
      String s = Serial.readString();
      uint8_t sendbuffer[20];
      s.getBytes(sendbuffer, 20);
      char sendbuffersize = min(20, s.length());
      Serial.print(F("\n* Sending -> \"")); Serial.print((char *)sendbuffer); Serial.println("\"");
      BTLEserial.write(sendbuffer, sendbuffersize);
    }
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
  display.display();
}

int32_t mseconds = 0;
const int secs = 28000;
void clock()
{
  mseconds++;
  if(mseconds >= secs) {
    mseconds = 0;
    seconds++;
    int y = 64/2-4;
    int x = 64-8;
    display.drawChar(x+2*8, y, ':', WHITE, BLACK, 1);
    display.drawChar(x-1*8, y, ':', WHITE, BLACK, 1);
    
    //drawInt(seconds, 64, 20, 1);
    
    drawInt(seconds/60/60, x-3*8, y, 1);
    drawInt((seconds/60)%60, x, y, 1);
    drawInt(seconds%60, x+3*8, y, 1);
  }
}
