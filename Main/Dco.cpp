#include "pwm_lib.h"
#include "PwmManager.h"
#include "DueTimer.h"
#include "Dco.h"
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

const unsigned char SAW_TABLE_C[1957] PROGMEM ={0,0,0,0,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,10,10,10,10,10,10,10,10,11,11,11,11,11,11,11,11,12,12,12,12,12,12,12,13,13,13,13,13,13,13,13,14,14,14,14,14,14,14,14,15,15,15,15,15,15,15,16,16,16,16,16,16,16,16,17,17,17,17,17,17,17,17,18,18,18,18,18,18,18,19,19,19,19,19,19,19,19,20,20,20,20,20,20,20,20,21,21,21,21,21,21,21,21,22,22,22,22,22,22,22,23,23,23,23,23,23,23,23,24,24,24,24,24,24,24,24,25,25,25,25,25,25,25,26,26,26,26,26,26,26,26,27,27,27,27,27,27,27,27,28,28,28,28,28,28,28,29,29,29,29,29,29,29,29,30,30,30,30,30,30,30,30,31,31,31,31,31,31,31,32,32,32,32,32,32,32,32,33,33,33,33,33,33,33,33,34,34,34,34,34,34,34,35,35,35,35,35,35,35,35,36,36,36,36,36,36,36,36,37,37,37,37,37,37,37,38,38,38,38,38,38,38,38,39,39,39,39,39,39,39,39,40,40,40,40,40,40,40,41,41,41,41,41,41,41,41,42,42,42,42,42,42,42,42,43,43,43,43,43,43,43,44,44,44,44,44,44,44,44,45,45,45,45,45,45,45,45,46,46,46,46,46,46,46,47,47,47,47,47,47,47,47,48,48,48,48,48,48,48,48,49,49,49,49,49,49,49,50,50,50,50,50,50,50,50,51,51,51,51,51,51,51,51,52,52,52,52,52,52,52,53,53,53,53,53,53,53,53,54,54,54,54,54,54,54,54,55,55,55,55,55,55,55,56,56,56,56,56,56,56,56,57,57,57,57,57,57,57,57,58,58,58,58,58,58,58,59,59,59,59,59,59,59,59,60,60,60,60,60,60,60,60,61,61,61,61,61,61,61,62,62,62,62,62,62,62,62,63,63,63,63,63,63,63,63,64,64,64,64,64,64,64,64,65,65,65,65,65,65,65,66,66,66,66,66,66,66,66,67,67,67,67,67,67,67,67,68,68,68,68,68,68,68,69,69,69,69,69,69,69,69,70,70,70,70,70,70,70,70,71,71,71,71,71,71,71,72,72,72,72,72,72,72,72,73,73,73,73,73,73,73,73,74,74,74,74,74,74,74,75,75,75,75,75,75,75,75,76,76,76,76,76,76,76,76,77,77,77,77,77,77,77,78,78,78,78,78,78,78,78,79,79,79,79,79,79,79,79,80,80,80,80,80,80,80,81,81,81,81,81,81,81,81,82,82,82,82,82,82,82,82,83,83,83,83,83,83,83,84,84,84,84,84,84,84,84,85,85,85,85,85,85,85,85,86,86,86,86,86,86,86,87,87,87,87,87,87,87,87,88,88,88,88,88,88,88,88,89,89,89,89,89,89,89,90,90,90,90,90,90,90,90,91,91,91,91,91,91,91,91,92,92,92,92,92,92,92,93,93,93,93,93,93,93,93,94,94,94,94,94,94,94,94,95,95,95,95,95,95,95,96,96,96,96,96,96,96,96,97,97,97,97,97,97,97,97,98,98,98,98,98,98,98,99,99,99,99,99,99,99,99,100,100,100,100,100,100,100,100,101,101,101,101,101,101,101,102,102,102,102,102,102,102,102,103,103,103,103,103,103,103,103,104,104,104,104,104,104,104,105,105,105,105,105,105,105,105,106,106,106,106,106,106,106,106,107,107,107,107,107,107,107,107,108,108,108,108,108,108,108,109,109,109,109,109,109,109,109,110,110,110,110,110,110,110,110,111,111,111,111,111,111,111,112,112,112,112,112,112,112,112,113,113,113,113,113,113,113,113,114,114,114,114,114,114,114,115,115,115,115,115,115,115,115,116,116,116,116,116,116,116,116,117,117,117,117,117,117,117,118,118,118,118,118,118,118,118,119,119,119,119,119,119,119,119,120,120,120,120,120,120,120,121,121,121,121,121,121,121,121,122,122,122,122,122,122,122,122,123,123,123,123,123,123,123,124,124,124,124,124,124,124,124,125,125,125,125,125,125,125,125,126,126,126,126,126,126,126,127,127,127,127,127,127,127,127,128,128,128,128,128,128,128,128,129,129,129,129,129,129,129,130,130,130,130,130,130,130,130,131,131,131,131,131,131,131,131,132,132,132,132,132,132,132,133,133,133,133,133,133,133,133,134,134,134,134,134,134,134,134,135,135,135,135,135,135,135,136,136,136,136,136,136,136,136,137,137,137,137,137,137,137,137,138,138,138,138,138,138,138,139,139,139,139,139,139,139,139,140,140,140,140,140,140,140,140,141,141,141,141,141,141,141,142,142,142,142,142,142,142,142,143,143,143,143,143,143,143,143,144,144,144,144,144,144,144,145,145,145,145,145,145,145,145,146,146,146,146,146,146,146,146,147,147,147,147,147,147,147,148,148,148,148,148,148,148,148,149,149,149,149,149,149,149,149,150,150,150,150,150,150,150,150,151,151,151,151,151,151,151,152,152,152,152,152,152,152,152,153,153,153,153,153,153,153,153,154,154,154,154,154,154,154,155,155,155,155,155,155,155,155,156,156,156,156,156,156,156,156,157,157,157,157,157,157,157,158,158,158,158,158,158,158,158,159,159,159,159,159,159,159,159,160,160,160,160,160,160,160,161,161,161,161,161,161,161,161,162,162,162,162,162,162,162,162,163,163,163,163,163,163,163,164,164,164,164,164,164,164,164,165,165,165,165,165,165,165,165,166,166,166,166,166,166,166,167,167,167,167,167,167,167,167,168,168,168,168,168,168,168,168,169,169,169,169,169,169,169,170,170,170,170,170,170,170,170,171,171,171,171,171,171,171,171,172,172,172,172,172,172,172,173,173,173,173,173,173,173,173,174,174,174,174,174,174,174,174,175,175,175,175,175,175,175,176,176,176,176,176,176,176,176,177,177,177,177,177,177,177,177,178,178,178,178,178,178,178,179,179,179,179,179,179,179,179,180,180,180,180,180,180,180,180,181,181,181,181,181,181,181,182,182,182,182,182,182,182,182,183,183,183,183,183,183,183,183,184,184,184,184,184,184,184,185,185,185,185,185,185,185,185,186,186,186,186,186,186,186,186,187,187,187,187,187,187,187,188,188,188,188,188,188,188,188,189,189,189,189,189,189,189,189,190,190,190,190,190,190,190,191,191,191,191,191,191,191,191,192,192,192,192,192,192,192,192,193,193,193,193,193,193,193,193,194,194,194,194,194,194,194,195,195,195,195,195,195,195,195,196,196,196,196,196,196,196,196,197,197,197,197,197,197,197,198,198,198,198,198,198,198,198,199,199,199,199,199,199,199,199,200,200,200,200,200,200,200,201,201,201,201,201,201,201,201,202,202,202,202,202,202,202,202,203,203,203,203,203,203,203,204,204,204,204,204,204,204,204,205,205,205,205,205,205,205,205,206,206,206,206,206,206,206,207,207,207,207,207,207,207,207,208,208,208,208,208,208,208,208,209,209,209,209,209,209,209,210,210,210,210,210,210,210,210,211,211,211,211,211,211,211,211,212,212,212,212,212,212,212,213,213,213,213,213,213,213,213,214,214,214,214,214,214,214,214,215,215,215,215,215,215,215,216,216,216,216,216,216,216,216,217,217,217,217,217,217,217,217,218,218,218,218,218,218,218,219,219,219,219,219,219,219,219,220,220,220,220,220,220,220,220,221,221,221,221,221,221,221,222,222,222,222,222,222,222,222,223,223,223,223,223,223,223,223,224,224,224,224,224,224,224,225,225,225,225,225,225,225,225,226,226,226,226,226,226,226,226,227,227,227,227,227,227,227,228,228,228,228,228,228,228,228,229,229,229,229,229,229,229,229,230,230,230,230,230,230,230,231,231,231,231,231,231,231,231,232,232,232,232,232,232,232,232,233,233,233,233,233,233,233,234,234,234,234,234,234,234,234,235,235,235,235,235,235,235,235,236,236,236,236,236,236,236,236,237,237,237,237,237,237,237,238,238,238,238,238,238,238,238,239,239,239,239,239,239,239,239,240,240,240,240,240,240,240,241,241,241,241,241,241,241,241,242,242,242,242,242,242,242,242,243,243,243,243,243,243,243,244,244,244,244,244,244,244,244,245,245,245,245,245,245,245,245,246,246,246,246,246,246,246,247,247,247,247,247,247,247,247,248,248,248,248,248,248,248,248,249,249,249,249,249,249,249,250,250,250,250,250,250,250,250,251,251,251,251,251,251,251,251,252,252,252,252,252,252,252,253,253,253,253,253,253,253,253,254,254,254,254,254,254,254,254,255,255,255};

// ********************************************** DCO Management ************************************************************
#define VOICES_LEN  6

static void dcoUpdateForWaveSamples(void);
static void dcoUpdateForADSRs(void);

static void dcoUpdateSynth(void);
static void dcoUpdatePiano(void);
static void dcoUpdateMello(void);


static volatile unsigned char synthMode;
static volatile signed int voices[VOICES_LEN];
static volatile signed int voicesPrev[VOICES_LEN];

// Private variables for synth mode
static volatile unsigned int squareCounters[VOICES_LEN];
static volatile unsigned int squareCounterValueHalf[VOICES_LEN];
static volatile unsigned int squareCounterValue[VOICES_LEN];
static volatile unsigned int synthWaveform;
static volatile unsigned int n[VOICES_LEN];
//_________________________________

volatile signed int filterFc[VOICES_LEN]; // 20000hz
//volatile signed int filterFc[VOICES_LEN]; // 100hz
//___________________________________________________________________________________________________________________________

void dco_init(void)
{
    synthMode = DCO_SYNTH_MODE_SYNTH;

    int i;
    for(i=0;i<VOICES_LEN; i++)
    {
      voices[i]=0;
      voicesPrev[i]=0;
      squareCounters[i]=0;
      n[i]=0;
    }
    synthWaveform=SYNTH_WAVEFORM_SQUARE;
    //_________ 

    // prueba
    for(i=0;i<VOICES_LEN; i++)
    {
        squareCounterValue[i]=256; // 64:1Khz , 2000: 30Hz
        squareCounterValueHalf[i]=squareCounterValue[i]/2;

        filterFc[i]=2000; // 20Khz
    }
    //_______
    
    Timer3.attachInterrupt(dcoUpdateForWaveSamples).setFrequency(64000).start(); // freq update: 64Khz
    Timer4.attachInterrupt(dcoUpdateForADSRs).setFrequency(1536).start(); // freq update: 1536Hz 


}

void dco_setLpfFc(signed int fcValue)
{
    int i;
    for(i=0;i<VOICES_LEN; i++)
    {
        filterFc[i]=(fcValue/10); // 20Khz
    }   
}

static void dcoUpdateForWaveSamples(void)
{
    int i;
  
    
  
    // Generate waveform
    switch(synthMode)
    {
        case DCO_SYNTH_MODE_SYNTH:dcoUpdateSynth(); break;
        case DCO_SYNTH_MODE_PIANO:dcoUpdatePiano(); break;
        case DCO_SYNTH_MODE_MELLOTRON:dcoUpdateMello(); break;
    }
    //__________________

    // Filters  3.75uS @ 6voices
    for(i=0;i<VOICES_LEN; i++)
    {
        voices[i] = voicesPrev[i] + ( (filterFc[i] * (voices[i]-voicesPrev[i]))/6400 ); // alpha = fc/fs = fc / 64Khz
  
        if(voices[i]<0)
          voices[i]=0;
        if(voices[i]>(PWM_MAX_VALUE*16))
            voices[i]=(PWM_MAX_VALUE*16);
              
        voicesPrev[i] = voices[i];
    }    
    //______
    

    // Set PWMs
    for(i=0;i<VOICES_LEN; i++)
      pwmm_setValuePwmFast(i,(voices[i]>>4));
    //_________  

    
}

inline static void dcoUpdateSynth(void)
{digitalWrite(24, HIGH);
    switch(synthWaveform)
    {  
        case SYNTH_WAVEFORM_SQUARE:
        {
            int i;
            for(i=0;i<VOICES_LEN; i++)  // 5.42uS with 6 voices
            {
                squareCounters[i]++;
                if (squareCounters[i] < squareCounterValueHalf[i])
                {
                    voices[i] = 0;
                }
                else if (squareCounters[i] < squareCounterValue[i])
                {
                    voices[i] = (PWM_MAX_VALUE*16) ;
                }
                else
                {
                    squareCounters[i] = 0;
                }
            }
            break;
        }
        case SYNTH_WAVEFORM_SAW:
        {
          /*
            int i;
            for(i=0;i<VOICES_LEN; i++)  // @ 1us per voice
            {
                squareCounters[i]++;
                if (squareCounters[i] < squareCounterValue[i])
                {
                    voices[i] =  ((PWM_MAX_VALUE*16)*squareCounters[i])/squareCounterValue[i] ;
                }
                else
                {
                    squareCounters[i] = 0;
                    voices[i]=0;
                }
            }  
           */
            int i;
            for(i=0;i<VOICES_LEN; i++)  // 4.98uS with 6 voices
            {
                n[i]+=16;
                if(n[i] >= 1957 )
                    n[i] = n[i]-1957;

                voices[i] = SAW_TABLE_C[n[i]]<<5;
            } 
            break;
        }  
        case SYNTH_WAVEFORM_TRIANGLE:
        {
            
            break;
        }              
    } 
    digitalWrite(24, LOW);  
}

inline static void dcoUpdatePiano(void)
{
  
}

inline static void dcoUpdateMello(void)
{
  
}
static void dcoUpdateForADSRs(void)
{
  
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

  // SLOWs PWM. 8bit@164Khz
  analogWrite (2, 128);
  analogWrite (13, 128);
  
  analogWrite (5, 128);
  analogWrite (4, 128);

  analogWrite (3, 128);
  analogWrite (10, 128);

  analogWrite (11, 128);
  analogWrite (12, 128);
 
  // pins 2 and 3 share the same timer so must have same frequency
  setupTimerPwm (2, 164000, 128); 
  setupTimerPwm (13, 164000, 128); 
  
  // pins 5 and 4 share the same timer
  setupTimerPwm (5, 164000, 128); 
  setupTimerPwm (4, 164000, 128); 

  // pins 3 and 10 share the same timer
  setupTimerPwm (3, 164000, 128); 
  setupTimerPwm (10, 164000, 128); 

  // pins 11 and 12 share the same timer
  setupTimerPwm (11, 164000, 128); 
  setupTimerPwm (12, 164000, 128); 
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

