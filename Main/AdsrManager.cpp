#include <Arduino.h>
#include "Dco.h"

#define PWM_MAX_VALUE 508  // 127*4 (a lower value is used so counters can add or substract a 4 value)

#define ATTACK_MAX_VALUE  (PWM_MAX_VALUE) 
#define SUSTAIN_MAX_VALUE  (PWM_MAX_VALUE) 

#define ADSR_LEN      8 // voice 0 to 5 + filter adsr + vca adsr


#define STATE_IDLE    0
#define STATE_ATTACK  1
#define STATE_DECAY   2
#define STATE_SUSTAIN 3
#define STATE_RELEASE 4



// Private variables
static volatile int state[ADSR_LEN];
static int volatile adsrValue[ADSR_LEN];

static int volatile attackRate[ADSR_LEN];
static int volatile decayRate[ADSR_LEN];
static int volatile sustainValue[ADSR_LEN];
static int volatile releaseRate[ADSR_LEN];

static int volatile attackRateCounter[ADSR_LEN];
static int volatile decayRateCounter[ADSR_LEN];
static int volatile releaseRateCounter[ADSR_LEN];

static int volatile flagEnvLowSpeed;
static int volatile lowSpeedDivider;

static volatile int adsrThresholdForFreeVoice;

// Private functions
static void setAdsrPwmValue(int i, int value);


void adsr_init(void)
{ 
  int i;
  for(i=0; i<ADSR_LEN; i++)
  {
    state[i]=STATE_IDLE;
    
    adsrValue[i] = 0;

    attackRate[i]=0; //64;
    decayRate[i]=0; //64;
    sustainValue[i] = ATTACK_MAX_VALUE; //ATTACK_MAX_VALUE/2;
    releaseRate[i]=0; //64;

    attackRateCounter[i]=attackRate[i];
    decayRateCounter[i]=decayRate[i];
    releaseRateCounter[i]=releaseRate[i];

    setAdsrPwmValue(i,adsrValue[i]);
  }

  adsrThresholdForFreeVoice = (ATTACK_MAX_VALUE/2)/10; // leer de configuracion
  flagEnvLowSpeed=0; // leer de entrada
  lowSpeedDivider=0;
}


void adsr_gateOnEvent(void)
{

}
void adsr_gateOffEvent(int index)
{
   // int i;
   // for(i=0; i<ADSR_LEN; i++)
    {
        releaseRateCounter[index] = releaseRate[index];
        state[index] = STATE_RELEASE;
    }
}

void adsr_triggerEvent(int index, int vel) // vel can be used to modulate attack rate
{
    if(state[index]!=STATE_SUSTAIN)
    {
        attackRateCounter[index]=attackRate[index];
        state[index] = STATE_ATTACK;
    }
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
    if(value>PWM_MAX_VALUE)
      value=PWM_MAX_VALUE;

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
          if(adsrValue[i]>=ATTACK_MAX_VALUE)
          {
            adsrValue[i] = ATTACK_MAX_VALUE;
            decayRateCounter[i] = decayRate[i];
            state[i] = STATE_DECAY;
          }
          setAdsrPwmValue(i,adsrValue[i]);
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
          setAdsrPwmValue(i,adsrValue[i]);        
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
              Serial.print("FIN DE ADSR Num:");
              Serial.print(i,DEC);
              Serial.print("\n"); 
          }
          
          setAdsrPwmValue(i,adsrValue[i]);                
        }
        break;
      }
    }
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
    sustainValue[i] = (value*SUSTAIN_MAX_VALUE)/127;
}


