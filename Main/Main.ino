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

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI display(U8G2_R0, /* cs=*/ 53, /* dc=*/ 51, /* reset=*/ 49);
//U8G2_SH1106_128X64_NONAME_F_4W_SW_SPI display(U8G2_R0, /* CLK=*/ 30,/* MOSI=*/ 31,/* cs=*/ 7, /* dc=*/ 6, /* reset=*/ 8);
  
void setup() {  
    Serial.begin(9600);


    display.begin();
    display.setPowerSave(0);

    pinMode(24, OUTPUT); 

    pwmm_init();
    dco_init();

    mem_init();

    midi_init();
    midircv_init();
    frontp_init();

    
}

signed int fc=0;
int note=0;

void loop() {

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

    /*
    // Test
    delay(20000);
    //note++;
    //if(note>=4)
    //  note=0;
    dco_setNote(48,127); // starts from C3
    dco_setNote(49,127); // starts from D3
    dco_setNote(50,127); // starts from D3
    dco_setNote(51,127); // starts from D3
    dco_setNote(52,127); // starts from D3
    dco_setNote(53,127); // starts from D3
    delay(5000);    
    dco_releaseVoice(0); // starts from C3
    dco_releaseVoice(1); // starts from C3
    dco_releaseVoice(2); // starts from C3
    dco_releaseVoice(3); // starts from C3
    dco_releaseVoice(4); // starts from C3
    dco_releaseVoice(5); // starts from C3
    //____
    */

/*
     int pos = frontp_getEncoderPosition(0);
     Serial.print("POS:");
     Serial.print(pos,DEC);
     Serial.print("\n"); 
     delay(100);
*/  
    //midircv_stateMachine();  
}






