#include <Arduino.h>
#include "DueTimer.h"

#include <SPI.h>
#include <Wire.h>
#include "MenuManager.h"
#include "PwmManager.h"
#include "Dco.h"
#include "AdsrManager.h"
#include "MIDIReception.h"
#include "MIDIManager.h"
#include "FrontPanel.h"
#include "Memory.h"
#include "SequencerManager.h"

/**
 * COMPILATION NOTES
 * Edit ~\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.11\platform.txt and change flags from -Os to -O3 
 */

#define PIN_SD_CS   52



void systick(void)
{
     frontp_tick1Ms();
     seq_sysTick();
}
  
void setup() {  
    Serial.begin(9600);

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
    seq_init();

    dco_setWaveForm(0);

    Timer1.attachInterrupt(systick).setFrequency(1000).start(); // freq update: 1Khz 

    menu_init();


}


void loop() {

 
  while(1)
  {
      midircv_stateMachine();

      frontp_loop();

      seq_loop();

      menu_loop();

      if(adsr_areAllIdle())
      {
          // turn off LFO
          dco_lfoOff();
      }
    
  }
}






