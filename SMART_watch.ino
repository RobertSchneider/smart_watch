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
}

aci_evt_opcode_t laststatus = ACI_EVT_DISCONNECTED;
void loop() {
  
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
    if(cnt == 4) {
      char x = c[0];
      char y = c[1];
      char d = c[2];
      char d2 = c[3];
      display.setData((int16_t)x, (int16_t)y, (int16_t)(d<<8)+d2);
    }
    display.display();
    
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
