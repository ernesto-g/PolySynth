#include "pwm_lib.h"
#include "PwmManager.h"
#include "DueTimer.h"
#include "Dco.h"
#include "AdsrManager.h"
using namespace arduino_due::pwm_lib;



// ********************************************** PWM Management ************************************************************
pwm<pwm_pin::PWML0_PC2> pwm_pin34;
pwm<pwm_pin::PWML1_PC4> pwm_pin36;
pwm<pwm_pin::PWML2_PC6> pwm_pin38;
pwm<pwm_pin::PWML3_PC8> pwm_pin40;

pwm<pwm_pin::PWML4_PC21> pwm_pin9;
pwm<pwm_pin::PWML5_PC22> pwm_pin8;
pwm<pwm_pin::PWML6_PC23> pwm_pin7;
pwm<pwm_pin::PWML7_PC24> pwm_pin6;

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

static uint32_t setupTimerPwm (byte pin, uint32_t frequency, unsigned dutyCycle);
static inline void pwmm_setValuePwmFast(unsigned char index,unsigned int value);
//___________________________________________________________________________________________________________________________

// ******************************************** Wave forms ******************************************************************
#include "WaveTables.c"

// ********************************************** DCO Management ************************************************************
#define VOICES_LEN    6
#define MIDI_C1_NOTE  24

struct S_ddsInfo
{
  unsigned char delta; // 1:octave C1. 2: octave C2. 4: octave C3. 8: octave C4 ....
  const volatile short unsigned int* table; // table pointer to specific note and waveform
  unsigned int tableLen;  
};
typedef struct S_ddsInfo DDSInfo;

static void dcoUpdateForWaveSamples(void);
static void dcoUpdateForADSRs(void);
inline static void dcoUpdateSamples(void);
static int searchFreeVoice(void);

static volatile unsigned int synthWaveform;
static volatile DDSInfo ddsInfo[VOICES_LEN];
static volatile signed int voices[VOICES_LEN];
static volatile unsigned int n[VOICES_LEN];
//_________________________________

//___________________________________________________________________________________________________________________________

void dco_init(void)
{
    int i;
    for(i=0;i<VOICES_LEN; i++)
    {
      voices[i]=0;
      n[i]=0;
    }

    synthWaveform = 7;
    //_________ 

    // prueba
    for(i=0;i<VOICES_LEN; i++)
    {
      // seteo de nota
      ddsInfo[i].delta=16; // octava
      ddsInfo[i].table=SAW_TABLE_D; // nota
      ddsInfo[i].tableLen = SAW_TABLE_D_LEN; // len de tabla
      //_____________      
    }
    //_______

    adsr_init();
    
    Timer3.attachInterrupt(dcoUpdateForWaveSamples).setFrequency(64000).start(); // freq update: 64Khz
    Timer4.attachInterrupt(dcoUpdateForADSRs).setFrequency(14400).start(); // freq update: 14.4Khz 
}




void dco_setNote(int note)
{
  /*
    if(note<MIDI_C1_NOTE)
        return;
    note = note-MIDI_C1_NOTE;
    
    int baseNote = note%12; // C:0 .... B:11
    int octave = note/12;

    int indexFreeVoice = searchFreeVoice();
    if(indexFreeVoice>=0)
    {
        n[indexFreeVoice]=0;
        ddsInfo[indexFreeVoice].table=waveTablesInfo[synthWaveform].table[baseNote]; //table
        ddsInfo[indexFreeVoice].delta=(1<<octave); // octave
        ddsInfo[indexFreeVoice].tableLen = waveTablesInfo[synthWaveform].len[baseNote]; // table len
    }
    */
    
                    
    int i;
    switch(note)
    {
        case 0:
                for(i=0;i<VOICES_LEN; i++)
                {
                    ddsInfo[i].delta=4; // octava
                    ddsInfo[i].table=ALESIS_FUSION_BASS_TABLE_C; // nota
                    ddsInfo[i].tableLen = ALESIS_FUSION_BASS_TABLE_C_LEN; // len de tabla
                    
                }
                break;
        case 1:
                for(i=0;i<VOICES_LEN; i++)
                {
                    ddsInfo[i].delta=4; // octava
                    ddsInfo[i].table=GUITAR_12STR_TABLE_C; // nota
                    ddsInfo[i].tableLen = GUITAR_12STR_TABLE_C_LEN; // len de tabla                    
                }
                break;
        case 2:
                for(i=0;i<VOICES_LEN; i++)
                {
                    ddsInfo[i].delta=4; // octava
                    ddsInfo[i].table=GUITAR_TABLE_C; // nota
                    ddsInfo[i].tableLen = GUITAR_TABLE_C_LEN; // len de tabla
                }
                break;
        case 3:
                for(i=0;i<VOICES_LEN; i++)
                {
                    ddsInfo[i].delta=4; // octava
                    ddsInfo[i].table=KORG_M3R_TABLE_C; // nota
                    ddsInfo[i].tableLen = KORG_M3R_TABLE_C_LEN; // len de tabla
                }
                break;
        case 4:
                for(i=0;i<VOICES_LEN; i++)
                {
                    ddsInfo[i].delta=4; // octava
                    ddsInfo[i].table=MARIMBA_TABLE_C; // nota
                    ddsInfo[i].tableLen = MARIMBA_TABLE_C_LEN; // len de tabla
                }
                break;                
                
    }
    
}

static void dcoUpdateForWaveSamples(void)
{
    digitalWrite(24, HIGH);
  
    // Generate waveform
    dcoUpdateSamples();

    // Set PWMs
    int i;
    for(i=0;i<VOICES_LEN; i++)
      pwmm_setValuePwmFast(i,(voices[i]>>4));
    //_________  

    digitalWrite(24, LOW);
}

inline static void dcoUpdateSamples(void)
{
    int i;
    for(i=0;i<VOICES_LEN; i++)  // 4.45uS with 6 voices
    {
        n[i]+=ddsInfo[i].delta;
        
        if(n[i] >= ddsInfo[i].tableLen )
            n[i] = n[i]-ddsInfo[i].tableLen;

        voices[i] = ddsInfo[i].table[n[i]];
    } 
}



static void dcoUpdateForADSRs(void)
{
    adsr_stateMachineTick(); 
}


static int searchFreeVoice(void)
{
    int adsrIndex = adsr_getFreeAdsr(VOICES_LEN);
    if(adsrIndex>=0)
    {
        return adsrIndex;
    }
    return -1;
}


// ********************************************** PWM Management ************************************************************
void pwmm_init(void)
{
  // FAST PWMs 9bit@146Khz
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

  // SLOWs PWM. 9bit@82Khz
  analogWrite (2, 128);
  analogWrite (13, 128);
  
  analogWrite (5, 128);
  analogWrite (4, 128);

  analogWrite (3, 128);
  analogWrite (10, 128);

  analogWrite (11, 128);
  analogWrite (12, 128);
 
  // pins 2 and 3 share the same timer so must have same frequency
  setupTimerPwm (2, 82000, 128); 
  setupTimerPwm (13, 82000, 128); 
  
  // pins 5 and 4 share the same timer
  setupTimerPwm (5, 82000, 128); 
  setupTimerPwm (4, 82000, 128); 

  // pins 3 and 10 share the same timer
  setupTimerPwm (3, 82000, 128); 
  setupTimerPwm (10, 82000, 128); 

  // pins 11 and 12 share the same timer
  setupTimerPwm (11, 82000, 128); 
  setupTimerPwm (12, 82000, 128); 
}
static inline void pwmm_setValuePwmFast(unsigned char index,unsigned int value)
{
    switch(index)
    {
        case PWM_FAST_0: pwm_pin34.set_duty_fast(value); break;
        case PWM_FAST_1: pwm_pin36.set_duty_fast(value); break;
        case PWM_FAST_2: pwm_pin38.set_duty_fast(value); break;
        case PWM_FAST_3: pwm_pin40.set_duty_fast(value); break;
        case PWM_FAST_4: pwm_pin6.set_duty_fast(value); break;
        case PWM_FAST_5: pwm_pin7.set_duty_fast(value); break;
        case PWM_FAST_6: pwm_pin8.set_duty_fast(value); break;
        case PWM_FAST_7: pwm_pin9.set_duty_fast(value); break;
    }
}
void pwmm_setValuePwmSlow(unsigned char index,unsigned int value)
{
    switch(index)
    {
        case PWM_SLOW_0:  TC_SetRA (TC0, 0, value); break;
        case PWM_SLOW_1:  TC_SetRA (TC2, 1, value); break;
        case PWM_SLOW_2:  TC_SetRA (TC2, 0, value); break;
        case PWM_SLOW_3:  TC_SetRB (TC2, 0, value); break;
        case PWM_SLOW_4:  TC_SetRB (TC2, 1, value); break;
        case PWM_SLOW_5:  TC_SetRA (TC2, 2, value); break;
        case PWM_SLOW_6:  TC_SetRB (TC2, 2, value); break;
        case PWM_SLOW_7:  TC_SetRB (TC0, 0, value); break;
    }
}
static uint32_t setupTimerPwm (byte pin, uint32_t frequency, unsigned dutyCycle)
{
  uint32_t count = VARIANT_MCK/2/frequency; // 42Mhz/freq
  tTimerInfo *pTimer = &timerLookup[pin];

  //Serial.write("COUNT:");
  //Serial.print(count,DEC);
  
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
//___________________________________________________________________________________________________________________________

