#include <SPI.h>
#include <Wire.h>
#include "U8g2lib.h"
#include "U8x8lib.h"
#include "PwmManager.h"
#include "Dco.h"
#include "AdsrManager.h"

/**
 * COMPILATION NOTES
 * Edit ~\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\platform.txt and change flags from -Os to -O3 
 */

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI display(U8G2_R0, /* cs=*/ 53, /* dc=*/ 51, /* reset=*/ 49);
//U8G2_SH1106_128X64_NONAME_F_4W_SW_SPI display(U8G2_R0, /* CLK=*/ 30,/* MOSI=*/ 31,/* cs=*/ 7, /* dc=*/ 6, /* reset=*/ 8);
  
void setup() {  
    Serial.begin(9600);
    display.begin();
    display.setPowerSave(0);

    pinMode(24, OUTPUT); 

    pwmm_init();
    dco_init();

}

signed int fc=0;
int note=0;

void loop() {

  int c=0;
  while(1)
  {
      /*
      delay(1);
      adsr_stateMachineTick();
      c++;
      if(c==5000)
      {
        dco_setNote(48); // starts from C3
      }
      if(c==7000)
      {
        dco_releaseNote(48); // starts from C3
      }

      if(c==10000)
        c=0;
      */
      pwmm_setValuePwmSlow(0,0);
      delay(2000);
      pwmm_setValuePwmSlow(0,100);
      delay(2000);
      pwmm_setValuePwmSlow(0,200);
      delay(2000);
      pwmm_setValuePwmSlow(0,300);
      delay(2000);
      pwmm_setValuePwmSlow(0,400);
      delay(2000);
      pwmm_setValuePwmSlow(0,508);
      delay(2000);
      
      
  }


    // spi: 12ms
    // sw: 100ms

    //Serial.print("loop");
    /*
    display.firstPage();
    do 
    {
        display.setFont(u8g2_font_ncenB14_tr); 
        display.drawStr(0, 20, "Linea 1 Linea 1"); 
        display.drawStr(0, 40, "Linea 2 Linea 2"); 
        display.drawStr(0, 60, "Linea 3 Linea 3"); 
    }
    while(display.nextPage()); 
    */
    
    delay(5000);
    /*
     dco_setLpfFc(fc);
     fc+=10;
     if(fc>2000)
        fc=0;
    */

    //note++;
    //if(note>=4)
    //  note=0;
    dco_setNote(48); // starts from C3
    delay(2000);    
    dco_releaseNote(48); // starts from C3
    
      
}






