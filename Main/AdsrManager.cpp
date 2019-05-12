#include <Arduino.h>
#include "Dco.h"
#include "MIDIReception.h"
#include "MIDIManager.h"

#define PWM_MAX_VALUE 508  // 127*4 (a lower value is used so counters can add or substract a 4 value)

#define ATTACK_MAX_VALUE  (PWM_MAX_VALUE) 
#define SUSTAIN_MAX_VALUE  (PWM_MAX_VALUE) 

#define ADSR_LEN        7 // voice 0 to 5 + filter adsr
#define ADSR_VOICES_LEN 6 // voice 0 to 5
#define ADSR_FOR_VCF    6 // index for vcf adsr

#define STATE_IDLE    0
#define STATE_ATTACK  1
#define STATE_DECAY   2
#define STATE_SUSTAIN 3
#define STATE_RELEASE 4

#define ADSR_VCF_MODE_0   0
#define ADSR_VCF_MODE_1   1



// Private variables
static volatile int state[ADSR_LEN];
static int volatile adsrValue[ADSR_LEN];

static int volatile attackRate[ADSR_LEN];
static int volatile decayRate[ADSR_LEN];
static int volatile sustainValue[ADSR_LEN];
static int volatile releaseRate[ADSR_LEN];
static int volatile attackMaxValue[ADSR_LEN];
static int volatile sustainMaxValue[ADSR_LEN];

static int volatile attackRateCounter[ADSR_LEN];
static int volatile decayRateCounter[ADSR_LEN];
static int volatile releaseRateCounter[ADSR_LEN];

static int volatile flagEnvLowSpeed;
static int volatile lowSpeedDivider;
static int adsrVcfMode;

static volatile int adsrThresholdForFreeVoice;
static volatile int adsrEnableVelocity;

// Private functions
static void setAdsrPwmValue(int i, int value);
static int calculateAttackMaxValue(int vel);
static int calculateSustainValue(int index, int vel);


void adsr_init(void)
{ 
  int i;
  for(i=0; i<ADSR_LEN; i++)
  {
    state[i]=STATE_IDLE;
    
    adsrValue[i] = 0;

    attackRate[i]=24; //64;
    decayRate[i]=64; //64;
    sustainValue[i] = ATTACK_MAX_VALUE*2/3; //ATTACK_MAX_VALUE/2;
    releaseRate[i]=64; //64;
    attackMaxValue[i] = ATTACK_MAX_VALUE;
    sustainMaxValue[i] = ATTACK_MAX_VALUE*2/3;

    attackRateCounter[i]=attackRate[i];
    decayRateCounter[i]=decayRate[i];
    releaseRateCounter[i]=releaseRate[i];

    setAdsrPwmValue(i,adsrValue[i]);
  }

  adsrThresholdForFreeVoice = ATTACK_MAX_VALUE; //(ATTACK_MAX_VALUE/2)/10; // leer de configuracion
  adsrEnableVelocity = 1; // leer de configuracion
  
  flagEnvLowSpeed=0;
  lowSpeedDivider=0;
  adsrVcfMode = ADSR_VCF_MODE_0;
}


void adsr_gateOnEvent(void)
{

}
int adsr_gateOffEvent(int index)
{
    //Serial.print("release index:");
    //Serial.print(index,DEC);
    //Serial.print(" \n");
    
    int ret=0;
    // release index voice
    releaseRateCounter[index] = releaseRate[index];
    state[index] = STATE_RELEASE;
    //____________________

    // release ADSR for VCF if all voices are idle
    int i;
    int flagBusyVoice=0;
    for(i=0; i<ADSR_VOICES_LEN; i++)
    {
        if(state[i]!=STATE_RELEASE && state[i]!=STATE_IDLE)
        {
            flagBusyVoice=1;
            break;
        }
    }
    if(flagBusyVoice==0)  // no active voices, release vcf adsr
    {
        releaseRateCounter[ADSR_FOR_VCF] = releaseRate[ADSR_FOR_VCF];
        state[ADSR_FOR_VCF] = STATE_RELEASE;
        ret=1;
    }
    //____________________________________________
    return ret;
}

static int calculateAttackMaxValue(int vel)
{
    if(adsrEnableVelocity==1)
    {
        return (ATTACK_MAX_VALUE*vel)/127;        
    }
    else
    {
        return ATTACK_MAX_VALUE;
    }
}
static int calculateSustainValue(int index, int vel)
{
    if(adsrEnableVelocity==1)
    {
        return (sustainMaxValue[index]*vel)/127;        
    }
    else
    {
        return sustainMaxValue[index];
    }   
}

void adsr_triggerEvent(int index, int vel) // vel can be used to modulate attack rate
{
    
    if(state[index]!=STATE_SUSTAIN)
    {
        attackRateCounter[index] = attackRate[index];
        attackMaxValue[index] = calculateAttackMaxValue(vel);
        sustainValue[index] = calculateSustainValue(index,vel);
        state[index] = STATE_ATTACK;
    }
    
    // retrigger ADSR for VCF
    switch(adsrVcfMode)
    {
        case ADSR_VCF_MODE_0:
        {
            if(state[ADSR_FOR_VCF]!=STATE_SUSTAIN) // only trigger once
            {
                attackRateCounter[ADSR_FOR_VCF]=attackRate[ADSR_FOR_VCF];
                attackMaxValue[ADSR_FOR_VCF]=calculateAttackMaxValue(vel);
                sustainValue[ADSR_FOR_VCF] = calculateSustainValue(ADSR_FOR_VCF,vel);
                state[ADSR_FOR_VCF] = STATE_ATTACK;
            }
            break;
        }
        case ADSR_VCF_MODE_1: // retrigger with each key pressed
        {
            attackRateCounter[ADSR_FOR_VCF]=attackRate[ADSR_FOR_VCF];
            attackMaxValue[ADSR_FOR_VCF]=calculateAttackMaxValue(vel);
            sustainValue[ADSR_FOR_VCF] = calculateSustainValue(ADSR_FOR_VCF,vel);
            state[ADSR_FOR_VCF] = STATE_ATTACK;          
            break;
        }
    }
    //_______________________
     
}

int adsr_getFreeAdsr(int indexMax)
{
    int i;
    for(i=0; i<indexMax; i++)
    {
        if(state[i]==STATE_IDLE)
          return i;
    }
    
    // there is no free voices, check if any of them is about to finish
    // First look for voice with minimun adsr value
    int adsrMinValue=999999;
    int indexMin=-1;
    for(i=0; i<indexMax; i++)
    {
        if(state[i]==STATE_RELEASE)
        {
            if(adsrValue[i]<adsrMinValue)
            {
                adsrMinValue = adsrValue[i];
                indexMin=i;
            }
        }
    }
    
    // check if this voice is below the threshold
    if(indexMin>=0)
    {
        if(adsrValue[indexMin]<adsrThresholdForFreeVoice)
          return indexMin; // consider this adsr(voice) as free
    }
    return -1;
}




static void setAdsrPwmValue(int i, int value)
{
    // LFO Modulation
    if(i<ADSR_VOICES_LEN)
    {
        value = value + ((dco_getLfoSignedValue()*dc0_getLfoAmplitudeAmt())/100);
    }
    //_______________

    // Limits
    if(value>PWM_MAX_VALUE)
      value=PWM_MAX_VALUE;
    else if(value<0)
      value=0;  
    //_______


    pwmm_setValuePwmSlow(i,(unsigned int)value);
}

void adsr_stateMachineTick(void) // freq update: 14,4Khz
{
  // low speed mode (89ms to 11sec) (x10)
  if(flagEnvLowSpeed)
  {
    lowSpeedDivider++;
    if(lowSpeedDivider<10)
      return;
  }
  lowSpeedDivider=0;
  //______________________________

  
  int i;
  for(i=0; i<ADSR_LEN; i++)
  {
    switch(state[i])
    {
      case STATE_IDLE:
      {
        // idle, wait gate on, level=0
        break; 
      }
      case STATE_ATTACK:
      {
        // rising at attack rate, wait level to reach max
        attackRateCounter[i]--;
        if(attackRateCounter[i]<=0)
        {
          attackRateCounter[i]=attackRate[i];
          adsrValue[i]+=4;
          /*
          if(adsrValue[i]>=ATTACK_MAX_VALUE)
          {
            adsrValue[i] = ATTACK_MAX_VALUE;
            decayRateCounter[i] = decayRate[i];
            state[i] = STATE_DECAY;
          }
          */
          if(adsrValue[i]>=attackMaxValue[i])
          {
            adsrValue[i] = attackMaxValue[i];
            decayRateCounter[i] = decayRate[i];
            state[i] = STATE_DECAY;
          }
          
          //setAdsrPwmValue(i,adsrValue[i]);
        }
        break;
      }
      case STATE_DECAY:
      {
        // falling at decay rate, wait level to reach sustain level
        decayRateCounter[i]--;
        if(decayRateCounter[i]<=0)
        {
          decayRateCounter[i] = decayRate[i];
          adsrValue[i]-=4;
          if(adsrValue[i]<=sustainValue[i])
          {
            adsrValue[i] = sustainValue[i];
            state[i] = STATE_SUSTAIN;  
          }
          //setAdsrPwmValue(i,adsrValue[i]);        
        }
        break;
      }
      case STATE_SUSTAIN:
      {
        // wait gate off, to go to realease state
        break;
      }
      case STATE_RELEASE:
      {
        // falling at release rate, wait level to reach zero.  
        releaseRateCounter[i]--;
        if(releaseRateCounter[i]<=0)
        {
          releaseRateCounter[i] = releaseRate[i];
          adsrValue[i]-=4;
          if(adsrValue[i]<=0)
          {  
              adsrValue[i]=0;
              state[i] = STATE_IDLE;
              dco_disableVoice(i);
              midi_voiceFinishedEvent(i);   
              //Serial.print("FIN DE ADSR Num:");
              //Serial.print(i,DEC);
              //Serial.print("\n"); 
          }
          
          //setAdsrPwmValue(i,adsrValue[i]);                
        }
        break;
      }
    }

    setAdsrPwmValue(i,adsrValue[i]);
  }
}



void adsr_setMidiAttackRate(int i, int value)
{  
    attackRate[i] = value;
}
void adsr_setMidiDecayRate(int i, int value)
{
    decayRate[i] = value;
}
void adsr_setMidiReleaseRate(int i, int value)
{
    releaseRate[i] = value;
}
void adsr_setMidiSustainValue(int i, int value)
{
    sustainMaxValue[i] = (value*SUSTAIN_MAX_VALUE)/127;
}

int adsr_getMidiAttackRate(int i)
{  
    return attackRate[i];
}
int adsr_getMidiDecayRate(int i)
{
    return decayRate[i];
}
int adsr_getMidiReleaseRate(int i)
{
    return releaseRate[i];
}
int adsr_getMidiSustainValue(int i)
{
    return (127*sustainMaxValue[i])/SUSTAIN_MAX_VALUE;
}

void adsr_setFlagEnvLowSpeed(int value)
{
    flagEnvLowSpeed = value;
}

int adsr_getFlagEnvLowSpeed(void)
{
    return flagEnvLowSpeed;
}

void adsr_setVcfMode(int mode)
{
    adsrVcfMode = mode; 
}

int adsr_getVcfMode(void)
{
    return adsrVcfMode;
}

int adsr_areAllIdle(void)
{
    int flagBusyVoice=0;
    int i;
    for(i=0; i<ADSR_LEN; i++)
    {
        if(state[i]!=STATE_IDLE)
        {
            flagBusyVoice=1;
            break;
        }
    }
    if(flagBusyVoice)
      return 0;

    return 1;
}

