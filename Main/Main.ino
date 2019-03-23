#include <Arduino.h>
#include "DueTimer.h"

#include <SPI.h>
#include <Wire.h>
#include "U8g2lib.h"
#include "U8x8lib.h"
#include "PwmManager.h"
#include "Dco.h"
#include "AdsrManager.h"
#include "MIDIReception.h"
#include "MIDIManager.h"
#include "FrontPanel.h"
#include "Memory.h"

/**
 * COMPILATION NOTES
 * Edit ~\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\platform.txt and change flags from -Os to -O3 
 */

#define PIN_SD_CS   52

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI display(U8G2_R0, /* cs=*/ 49, /* dc=*/ 51, /* reset=*/ 53);

void systick(void)
{
     frontp_tick1Ms();
}
  
void setup() {  
    Serial.begin(9600);


    display.begin();
    display.setPowerSave(0);

    // CS for SDcard disabled
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH); 
    //_______________________

    pwmm_init();
    dco_init();

    mem_init();

    midi_init();
    midircv_init();
    frontp_init();

    dco_setWaveForm(0);

    Timer1.attachInterrupt(systick).setFrequency(1000).start(); // freq update: 1Khz . VER QUE NO ROMPA LOS PWM!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    
}


void loop() {

  char line1[16];
  char line2[16];
  char line3[16];
 
  while(1)
  {
      midircv_stateMachine();

      frontp_loop();

      // test
      sprintf(line1,"A:%d B:%d",frontp_getEncoderPosition(0),frontp_getEncoderPosition(1));
      sprintf(line2,"C:%d D:%d",frontp_getEncoderPosition(2),frontp_getEncoderPosition(3));
      
      sprintf(line3,"SW OFF");            
      if(frontp_getSwState(SW_MI)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_MI ON");
      if(frontp_getSwState(SW_OJ)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_OJ ON");
      if(frontp_getSwState(SW_PK)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_PK ON");
      if(frontp_getSwState(SW_QL)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_QL ON");

      if(frontp_getSwState(SW_BACK)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_BACK ON");
      if(frontp_getSwState(SW_ENTER)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_ENTER ON");
      if(frontp_getSwState(SW_SHIFT)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_SHIFT ON");
      
      //___________________
     
      // Display update spi: 12ms 
      display.firstPage();
      do 
      {
          display.setFont(u8g2_font_ncenB14_tr); 
          display.drawStr(0, 20, line1); 
          display.drawStr(0, 40, line2); 
          display.drawStr(0, 60, line3); 
      }
      while(display.nextPage()); 
      //__________________________           
  }
}






