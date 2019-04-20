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
static uint32_t setupTimerPwmInitTc (byte pin);
//___________________________________________________________________________________________________________________________

// ******************************************** Wave forms ******************************************************************
#include "WaveTables.c"

// ********************************************** DCO Management ************************************************************
#define VOICES_LEN    6
#define MIDI_C1_NOTE  24
#define SAMPLING_FREQ 64000

struct S_ddsInfo
{
  unsigned char delta; // 1:octave C1. 2: octave C2. 4: octave C3. 8: octave C4 ....
  const volatile short unsigned int* table; // table pointer to specific note and waveform
  unsigned int tableLen;  
  unsigned char enabled;
  unsigned int counterMax;
  unsigned int counter;
};
typedef struct S_ddsInfo DDSInfo;

static void dcoUpdateForWaveSamples(void);
static void dcoUpdateForADSRs(void);
inline static void dcoUpdateSamples(void);
static int searchFreeVoice(void);
static float getFreqByNote(int note);

static void lfoUpdate(void);

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
      ddsInfo[i].enabled=0;
    }
    synthWaveform = 0;
    //_________ 

    adsr_init();
    
    Timer3.attachInterrupt(dcoUpdateForWaveSamples).setFrequency(SAMPLING_FREQ).start();
    Timer4.attachInterrupt(dcoUpdateForADSRs).setFrequency(14400).start(); // freq update: 14.4Khz 
}


void dco_setWaveForm(int wf)
{
    synthWaveform = wf;
}
int dco_getWaveForm(void)
{
    return synthWaveform;
}


int dco_setNote(int note, int vel)
{
  
    if(note<MIDI_C1_NOTE)
        return -1;
    note = note-MIDI_C1_NOTE;
    
    int baseNote = note%12; // C:0 .... B:11
    int octave = note/12;

    //Serial.print("llego nota C3, busco voice libre");
    int indexFreeVoice = searchFreeVoice();
    if(indexFreeVoice>=0)
    {
        n[indexFreeVoice]=0;
        ddsInfo[indexFreeVoice].table=waveTablesInfo[synthWaveform].table[baseNote]; //table
        ddsInfo[indexFreeVoice].delta=(1<<octave); // octave
        ddsInfo[indexFreeVoice].tableLen = waveTablesInfo[synthWaveform].len[baseNote]; // table len
        ddsInfo[indexFreeVoice].enabled = 1;

        adsr_triggerEvent(indexFreeVoice, vel);

        dco_lfoOn();
    }
    return indexFreeVoice;
}
void dco_releaseVoice(int voice)
{
    adsr_gateOffEvent(voice);
}

void dco_disableVoice(int index)
{
    ddsInfo[index].enabled=0;
}

static void dcoUpdateForWaveSamples(void)
{
    //digitalWrite(24, HIGH);
  
    // Generate waveform
    dcoUpdateSamples();

    // Set PWMs
    int i;
    for(i=0;i<VOICES_LEN; i++)
      pwmm_setValuePwmFast(i,(voices[i]>>4));
    //_________  

    //digitalWrite(24, LOW);
}

inline static void dcoUpdateSamples(void)
{
    int i;
    for(i=0;i<VOICES_LEN; i++)  // 4.45uS with 6 voices
    {
        
        if(ddsInfo[i].enabled==0)
        {
            voices[i] = PWM_FAST_MAX_VALUE/2;
            continue;
        } 
/*
        ddsInfo[i].counter++;
        if(ddsInfo[i].counter>=ddsInfo[i].counterMax)
        {
            ddsInfo[i].counter=0;
            n[i]+=(ddsInfo[i].delta*2);
        }
        else
          n[i]+=ddsInfo[i].delta;
*/

        n[i]+=ddsInfo[i].delta;
        
        if(n[i] >= ddsInfo[i].tableLen )
            n[i] = n[i]-ddsInfo[i].tableLen;

        voices[i] = ddsInfo[i].table[n[i]];
    } 
}


static int lfoDivider=0;
static void dcoUpdateForADSRs(void)
{
    adsr_stateMachineTick(); 

    lfoDivider++;
    if(lfoDivider>10)
    {
        lfoUpdate();
        lfoDivider=0;
    }
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

static float getFreqByNote(int note)
{
    return NOTES_FREQ_TABLE[note];
}


// LFO management ************************************************************************************
#define LFO_WAVE_TYPE_SINE        4
#define LFO_WAVE_TYPE_TRIANGLE    3
#define LFO_WAVE_TYPE_EXP         2
#define LFO_WAVE_TYPE_SQUARE      1
#define LFO_WAVE_TYPE_RANDOM      0

static int lfoCounter=0;
static int lfoFreqMultiplier=40;
static int lfoSampleAndHoldNewSampleFlag=0;
static int lfoOn=1;
static int lfoWaitZero=1;
static int lfoWaveType=LFO_WAVE_TYPE_SINE;


static void lfoUpdate(void)
{
    lfoCounter += lfoFreqMultiplier;
    if (lfoCounter >= LFO_TABLE_SIZE) {
      lfoCounter = lfoCounter - LFO_TABLE_SIZE;
      lfoSampleAndHoldNewSampleFlag = 1;  
    }
  
    int val;
    switch (lfoWaveType)
    {
      case LFO_WAVE_TYPE_SINE:
        val = SINETABLE[lfoCounter];
        break;
      case LFO_WAVE_TYPE_TRIANGLE:
        val = TRIANGLETABLE[lfoCounter] ;
        break;
      case LFO_WAVE_TYPE_EXP:
        val = EXPTABLE[lfoCounter] ;
        break;
      case LFO_WAVE_TYPE_SQUARE:
        if (lfoCounter < (LFO_TABLE_SIZE / 2))
          val = PWM_MAX_VALUE;
        else
          val = 0;
        break;
      case LFO_WAVE_TYPE_RANDOM:
      /*
        if(lfoSampleAndHoldNewSampleFlag==1) // sample new value
        {
          lfoSampleAndHoldValue = RANDOMTABLE[randomCounter]; // hold value
          lfoSampleAndHoldNewSampleFlag = 0;
        } 
        val = lfoSampleAndHoldValue ; 
       */ 
        break;
    }
  
    if(val>PWM_MAX_VALUE)
      val = PWM_MAX_VALUE;

    if(lfoOn==0)
    {
        if(lfoWaitZero)
        {
            if(val<=0)
              lfoWaitZero=0;
        }
        else
          val = 0;      
    }
    pwmm_setValuePwmSlow(PWM_SLOW_7,val); // set lfo PWM 
    
  
    //int midiVal = (val*128) / 512;
    //val = val - (512/2); // convert to signed value
    
}
void dco_lfoOn(void)
{
  if(lfoOn==0)
  {
      lfoCounter=0;
  }
  
  lfoOn=1;
  lfoWaitZero=0;
}
void dco_lfoOff(void)
{
  if(lfoOn)
  {
    lfoWaitZero=1;  
    lfoOn=0;
  }
}
void dco_setLfoFreq(int freq)
{
    lfoFreqMultiplier = freq;
}
int dco_getLfoFrq(void)
{
    return lfoFreqMultiplier;
}
int dco_getLfoWaveForm(void)
{
    return lfoWaveType;
}
void dco_setLfoWaveForm(int wf)
{
    lfoWaveType = wf;
}

//___________________________________________________________________________________________________________________________

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
  pinMode(2, OUTPUT); //no
  pinMode(3, OUTPUT); //no
  pinMode(4, OUTPUT); // OK
  pinMode(5, OUTPUT); //no
  pinMode(10, OUTPUT); // OK
  pinMode(11, OUTPUT); //no
  pinMode(12, OUTPUT); // ok
  pinMode(13, OUTPUT); // ok
  
  analogWrite (2, 128);
  analogWrite (13, 128);
  
  analogWrite (5, 128);
  analogWrite (4, 128);

  analogWrite (3, 128);
  analogWrite (10, 128);

  analogWrite (11, 128);
  analogWrite (12, 128);

  // pins 2 and 3 share the same timer so must have same frequency
  setupTimerPwmInitTc (2);
  setupTimerPwm (2, 82000, 128); 
  setupTimerPwm (13, 82000, 128); 
  
  // pins 5 and 4 share the same timer
  setupTimerPwmInitTc (5);
  setupTimerPwm (5, 82000, 128); 
  setupTimerPwm (4, 82000, 128); 

  // pins 3 and 10 share the same timer
  setupTimerPwmInitTc (3);
  setupTimerPwm (3, 82000, 128); 
  setupTimerPwm (10, 82000, 128); 

  // pins 11 and 12 share the same timer
  setupTimerPwmInitTc (11);
  setupTimerPwm (11, 82000, 128); 
  setupTimerPwm (12, 82000, 128); 
 
}
static inline void pwmm_setValuePwmFast(unsigned char index,unsigned int value)
{
    switch(index)
    {
        case PWM_FAST_0: pwm_pin34.set_duty_fast(value); break; // OK
        case PWM_FAST_1: pwm_pin36.set_duty_fast(value); break; // OK
        case PWM_FAST_2: pwm_pin38.set_duty_fast(value); break; // OK
        case PWM_FAST_3: pwm_pin40.set_duty_fast(value); break; // OK
        case PWM_FAST_4: pwm_pin6.set_duty_fast(value); break;
        case PWM_FAST_5: pwm_pin7.set_duty_fast(value); break;
        case PWM_FAST_6: pwm_pin8.set_duty_fast(value); break;
        case PWM_FAST_7: pwm_pin9.set_duty_fast(value); break;
    }
}
void pwmm_setValuePwmSlow(unsigned char index,unsigned int value)
{
    value = value +1;
    /*
    Serial.print("\nSet PWM:");
    Serial.print(index,DEC);
    Serial.print(" valor:");
    Serial.print(value,DEC);
    Serial.print("\n");
      */    
    int pin;
    switch(index)
    {
        case PWM_SLOW_0:  pin=2; break; //TC_SetRA (TC0, 0, value); break; // OK
        case PWM_SLOW_1:  pin=3; break; //TC_SetRA (TC2, 1, value); break; // OK
        case PWM_SLOW_2:  pin=4; break; //TC_SetRA (TC2, 0, value); break;  // OK
        case PWM_SLOW_3:  pin=5; break; //TC_SetRB (TC2, 0, value); break; // OK
        case PWM_SLOW_4:  pin=10; break; //TC_SetRB (TC2, 1, value); break; // OK
        case PWM_SLOW_5:  pin=11; break; //TC_SetRA (TC2, 2, value); break; // OK
        case PWM_SLOW_6:  pin=12; break; //TC_SetRB (TC2, 2, value); break;
        case PWM_SLOW_7:  pin=13; break; //TC_SetRB (TC0, 0, value); break;
    }

    tTimerInfo *pTimer = &timerLookup[pin];

    if (pTimer->output == 0)
       TC_SetRA (pTimer->pTC, pTimer->channel, value);
    else
       TC_SetRB (pTimer->pTC, pTimer->channel, value);
    
}

static void TC_SetCMR_ChannelA(Tc *tc, uint32_t chan, uint32_t v)
{
  tc->TC_CHANNEL[chan].TC_CMR = (tc->TC_CHANNEL[chan].TC_CMR & 0xFFF0FFFF) | v;
}
static void TC_SetCMR_ChannelB(Tc *tc, uint32_t chan, uint32_t v)
{
  tc->TC_CHANNEL[chan].TC_CMR = (tc->TC_CHANNEL[chan].TC_CMR & 0xF0FFFFFF) | v;
}

static uint32_t setupTimerPwmInitTc (byte pin)
{
  tTimerInfo* pTimer = &timerLookup[pin];

  static const uint32_t channelToId[] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8 };

  uint32_t interfaceID = channelToId[pTimer->channel];
  pmc_enable_periph_clk(TC_INTERFACE_ID + interfaceID);

  
  TC_Configure(pTimer->pTC, pTimer->channel,
        TC_CMR_TCCLKS_TIMER_CLOCK1 |
        TC_CMR_WAVE |         // Waveform mode
        TC_CMR_WAVSEL_UP_RC | // Counter running up and reset when equals to RC
        TC_CMR_EEVT_XC0 |     // Set external events from XC0 (this setup TIOB as output)
        TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_CLEAR |
        TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_CLEAR);

}

static uint32_t setupTimerPwm (byte pin, uint32_t frequency, unsigned dutyCycle)
{
  uint32_t count = VARIANT_MCK/2/frequency; // 42Mhz/freq
  tTimerInfo* pTimer = &timerLookup[pin];

  /*
  static const uint32_t channelToId[] = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8 };
  uint32_t interfaceID = channelToId[pTimer->channel];
  pmc_enable_periph_clk(TC_INTERFACE_ID + interfaceID);

  
  TC_Configure(pTimer->pTC, pTimer->channel,
        TC_CMR_TCCLKS_TIMER_CLOCK1 |
        TC_CMR_WAVE |         // Waveform mode
        TC_CMR_WAVSEL_UP_RC | // Counter running up and reset when equals to RC
        TC_CMR_EEVT_XC0 |     // Set external events from XC0 (this setup TIOB as output)
        TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_CLEAR |
        TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_CLEAR);
  
  */
        
  if (pTimer != NULL)
  {
     TC_SetRC (pTimer->pTC, pTimer->channel, count);

    if (pTimer->output == 0)
    {
       TC_SetRA (pTimer->pTC, pTimer->channel, count * dutyCycle / 256);
       TC_SetCMR_ChannelA(pTimer->pTC, pTimer->channel, TC_CMR_ACPA_CLEAR | TC_CMR_ACPC_SET);
    }
    else
    {
       TC_SetRB (pTimer->pTC, pTimer->channel, count * dutyCycle / 256);
       TC_SetCMR_ChannelB(pTimer->pTC, pTimer->channel, TC_CMR_BCPB_CLEAR | TC_CMR_BCPC_SET);
    }

    TC_Start( pTimer->pTC, pTimer->channel );
  
    return count;
  }
  else
    return 0;
}
//___________________________________________________________________________________________________________________________

