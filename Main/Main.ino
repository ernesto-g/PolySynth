#include <SPI.h>
#include <Wire.h>
#include "U8g2lib.h"
#include "U8x8lib.h"

// Para PWMs *****************************
#include <chip.h>
#include "pwm_lib.h"
using namespace arduino_due::pwm_lib;
// PWM pins
pwm<pwm_pin::PWML0_PC2> pwm_pin34;
pwm<pwm_pin::PWML1_PC4> pwm_pin36;
pwm<pwm_pin::PWML2_PC6> pwm_pin38;
pwm<pwm_pin::PWML3_PC8> pwm_pin40;

pwm<pwm_pin::PWML4_PC21> pwm_pin9;
pwm<pwm_pin::PWML5_PC22> pwm_pin8;
pwm<pwm_pin::PWML6_PC23> pwm_pin7;
pwm<pwm_pin::PWML7_PC24> pwm_pin6;

// defines
#define PWM_MAX_VALUE 572
//_______________________________________


static void setupTcPwm(void) ;



U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI display(U8G2_R0, /* cs=*/ 53, /* dc=*/ 51, /* reset=*/ 49);
//U8G2_SH1106_128X64_NONAME_F_4W_SW_SPI display(U8G2_R0, /* CLK=*/ 30,/* MOSI=*/ 31,/* cs=*/ 7, /* dc=*/ 6, /* reset=*/ 8);
  
void setup() {  
    Serial.begin(9600);
    display.begin();
    display.setPowerSave(0);

    pinMode(24, OUTPUT); 


  //*********** PWMs ****************************
  pwm_pin34.start(681, 340); // for 681 -> dutymax: 572. Freq: 146Khz
  pwm_pin36.start(681, 340);
  pwm_pin38.start(681, 340);
  pwm_pin40.start(681, 340);
  pwm_pin6.start(681, 340);
  pwm_pin7.start(681, 340);
  pwm_pin8.start(681, 340);
  pwm_pin9.start(681, 340);

  pwm_pin34.set_duty_fast(256);
  pwm_pin36.set_duty_fast(256);
  pwm_pin38.set_duty_fast(256);
  pwm_pin40.set_duty_fast(256);

  pwm_pin6.set_duty_fast(256);
  pwm_pin7.set_duty_fast(256);
  pwm_pin8.set_duty_fast(256);
  pwm_pin9.set_duty_fast(256);


  setupTcPwm(); 


}

void loop() {

    // spi: 12ms
    // sw: 100ms
    digitalWrite(24, HIGH);
    display.firstPage();
    do 
    {
        display.setFont(u8g2_font_ncenB14_tr); 
        display.drawStr(0, 20, "Linea 1 Linea 1"); 
        display.drawStr(0, 40, "Linea 2 Linea 2"); 
        display.drawStr(0, 60, "Linea 3 Linea 3"); 
    }
    while(display.nextPage()); 
    digitalWrite(24, LOW);

    
    delay(1000); 
}



// --------------------------------------------
//
// PWM with timers demo
//
// Bob Cousins, August 2014
// --------------------------------------------

typedef struct {
    Tc *pTC;        // TC0, TC1, or TC2
    byte channel;   // 0-2
    byte output;    // 0 = A, 1 = B
}  tTimerInfo;

tTimerInfo timerLookup [] =
{
  {NULL,0,0}, // 0
  {NULL,0,0},
  {TC0,0,0},  // pin 2 = TIOA0
  {TC2,1,0},  // pin 3 = TIOA7
  {TC2,0,1},  // pin 4 = TIOB6
  {TC2,0,0},  // pin 5 = TIOA6
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {TC2,1,1},  // pin 10 = TIOB7
  {TC2,2,0},  // pin 11 = TIOA8
  {TC2,2,1},  // pin 12 = TIOB8
  {TC0,0,1}   // pin 13 = TIOB0
};


// \brief set a pin for PWM using a timer channel
// \param pin       pin to use (0-13 only!)
// \param frequency the frequency
// \param dutyCyle  duty cycle 0-255
// \return          this function returns a count, which is the effective PWM resolution. Returns 0 if pin is not valid
//
uint32_t setupTimerPwm (byte pin, uint32_t frequency, unsigned dutyCycle)
{
  uint32_t count = VARIANT_MCK/2/frequency; // 42Mhz/freq
  tTimerInfo *pTimer = &timerLookup[pin];

  Serial.write("COUNT:");
  Serial.print(count,DEC);
  
  if (pTimer != NULL)
  {
    TC_SetRC (pTimer->pTC, pTimer->channel, count);
    if (pTimer->output == 0)
       TC_SetRA (pTimer->pTC, pTimer->channel, count * dutyCycle / 256);
    else
       TC_SetRB (pTimer->pTC, pTimer->channel, count * dutyCycle / 256);
  
    return count;
  }
  else
    return 0;
}

static void setupTcPwm(void) 
{
  // put your setup code here, to run once:

  // use the Arduino lib to do initial setup
  analogWrite (2, 128);
  analogWrite (13, 128);
  
  analogWrite (5, 128);
  analogWrite (4, 128);

  analogWrite (3, 128);
  analogWrite (10, 128);

  analogWrite (11, 128);
  analogWrite (12, 128);
 
  // pins 2 and 3 share the same timer so must have same frequency
  setupTimerPwm (2, 2000, 128); 
  setupTimerPwm (13, 2000, 64); 
  
  // pins 5 and 4 share the same timer
  setupTimerPwm (5, 3000, 128); 
  setupTimerPwm (4, 3000, 64); 

  // pins 3 and 10 share the same timer
  setupTimerPwm (3, 4000, 128); 
  setupTimerPwm (10, 4000, 64); 

  // pins 11 and 12 share the same timer
  setupTimerPwm (11, 5000, 128); 
  setupTimerPwm (12, 5000, 64); 

}




