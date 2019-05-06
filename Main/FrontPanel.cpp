#include <Arduino.h>
#include "RotaryEncoder.h"
#include "FrontPanel.h"

// Defines
#define PIN_1_ENC_D   33
#define PIN_2_ENC_D   35

#define PIN_1_ENC_C   37
#define PIN_2_ENC_C   39

#define PIN_1_ENC_B   41
#define PIN_2_ENC_B   43

#define PIN_1_ENC_A   45
#define PIN_2_ENC_A   47

#define PIN_SW_MI     32
#define PIN_SW_OJ     30  
#define PIN_SW_PK     28
#define PIN_SW_QL     26  
#define PIN_SW_BACK   22
#define PIN_SW_ENTER  24
#define PIN_SW_SHIFT  48

#define PIN_SYNC_IN   42

    
#define LEN_SW          7

#define TIMEOUT_BOUNCE      20 // 10ms
#define TIMEOUT_SHORT_PRESS 1000  // 1sec
#define TIMEOUT_LONG_PRESS  2000  // 2sec

#define STATE_IDLE                  0
#define STATE_PRESSED               1
#define STATE_WAIT_BOUNCE           2
#define STATE_PRESS_CONFIRMED       3
#define STATE_WAIT_RELEASE          4
#define STATE_WAIT_RELEASE2         5
#define STATE_WAIT_BOUNCE_RELEASE   6

// Private functions
static RotaryEncoder encoderA(PIN_1_ENC_A, PIN_2_ENC_A);
static RotaryEncoder encoderB(PIN_1_ENC_B, PIN_2_ENC_B);
static RotaryEncoder encoderC(PIN_1_ENC_C, PIN_2_ENC_C);
static RotaryEncoder encoderD(PIN_1_ENC_D, PIN_2_ENC_D);
static int getPin(int swIndex);
static void swStateMachine(int swIndex);

// Private variables
static unsigned char state[LEN_SW];
static unsigned char switchesState[LEN_SW];
static volatile unsigned int timeouts[LEN_SW];
static volatile int flagSyncInPulse=0;




void frontp_tick1Ms(void)
{
    unsigned char i;
    for(i=0; i<LEN_SW; i++)
    {
        if(timeouts[i]<=TIMEOUT_LONG_PRESS)
          timeouts[i]++;  
    }
}

static void isrEncA(void) {
    encoderA.tick();
}
static void isrEncB(void) {
    encoderB.tick();
}
static void isrEncC(void) {
    encoderC.tick();
}
static void isrEncD(void) {
    encoderD.tick();
}

static void isrSyncIn(void)
{
    flagSyncInPulse=1;
}

void frontp_init(void)
{
    pinMode(PIN_1_ENC_A, INPUT_PULLUP);
    pinMode(PIN_2_ENC_A, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_1_ENC_A),isrEncA, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_2_ENC_A),isrEncA, CHANGE);

    pinMode(PIN_1_ENC_B, INPUT_PULLUP);
    pinMode(PIN_2_ENC_B, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_1_ENC_B),isrEncB, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_2_ENC_B),isrEncB, CHANGE);

    pinMode(PIN_1_ENC_C, INPUT_PULLUP);
    pinMode(PIN_2_ENC_C, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_1_ENC_C),isrEncC, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_2_ENC_C),isrEncC, CHANGE);

    pinMode(PIN_1_ENC_D, INPUT_PULLUP);
    pinMode(PIN_2_ENC_D, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PIN_1_ENC_D),isrEncD, CHANGE);
    attachInterrupt(digitalPinToInterrupt(PIN_2_ENC_D),isrEncD, CHANGE);

    pinMode(PIN_SYNC_IN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_SYNC_IN),isrSyncIn, FALLING);

    // change priorities to lowest
    NVIC_SetPriority(PIOA_IRQn, 0xFF);
    NVIC_SetPriority(PIOB_IRQn, 0xFF);
    NVIC_SetPriority(PIOC_IRQn, 0xFF);
    NVIC_SetPriority(PIOD_IRQn, 0xFF);
    //_____________________________

    frontp_setEncoderPosition(0,0);
    frontp_setEncoderPosition(1,0);
    frontp_setEncoderPosition(2,0);
    frontp_setEncoderPosition(3,0);

    // swicthes
    pinMode(PIN_SW_MI, INPUT_PULLUP);
    pinMode(PIN_SW_OJ, INPUT_PULLUP);
    pinMode(PIN_SW_PK, INPUT_PULLUP);
    pinMode(PIN_SW_QL, INPUT_PULLUP);

    pinMode(PIN_SW_BACK, INPUT_PULLUP);
    pinMode(PIN_SW_ENTER, INPUT_PULLUP);
    pinMode(PIN_SW_SHIFT, INPUT_PULLUP);
    
}

void frontp_loop(void)
{   
    int i;
    for(i=0;i<LEN_SW; i++)
      swStateMachine(i);
}

int frontp_getExternalSyncPulse(void)
{
    return flagSyncInPulse;
}
void frontp_resetExternalSyncPulse(void)
{
    flagSyncInPulse=0;
}


int frontp_getEncoderPosition(int index)
{
    switch(index)
    {
        case 0: return encoderA.getPosition();
        case 1: return encoderB.getPosition();
        case 2: return encoderC.getPosition();
        case 3: return encoderD.getPosition();
    }
    return 0;
}

void frontp_setEncoderPosition(int index,int pos)
{
    switch(index)
    {  
        case 0: encoderA.setPosition(pos);break;
        case 1: encoderB.setPosition(pos);break;
        case 2: encoderC.setPosition(pos);break;
        case 3: encoderD.setPosition(pos);break;
    }
}

int frontp_getSwState(int swIndex)
{
    return switchesState[swIndex];
}
void frontp_resetSwState(int swIndex)
{
    switchesState[swIndex]=FRONT_PANEL_SW_STATE_IDLE;
}


static void swStateMachine(int swIndex)
{
    switch(state[swIndex])
    {
        case STATE_IDLE:
        {
            if(digitalRead(getPin(swIndex))==LOW)
            {
                // sw pressed
                state[swIndex] = STATE_PRESSED;
            }
            break; 
        }
        case STATE_PRESSED:
        {
            timeouts[swIndex]=0;
            state[swIndex] = STATE_WAIT_BOUNCE;            
            break;
        }
        case STATE_WAIT_BOUNCE:
        {
            if(timeouts[swIndex]>=TIMEOUT_BOUNCE)
            {
                if(digitalRead(getPin(swIndex))==LOW)
                {
                    state[swIndex] = STATE_PRESS_CONFIRMED;             
                }
                else
                    state[swIndex] = STATE_IDLE;  
            }
            else
            {
                if(digitalRead(getPin(swIndex))==HIGH)
                    state[swIndex] = STATE_IDLE; // bouncing   
            }
            break;
        }
        case STATE_PRESS_CONFIRMED:
        {
            // wait for short or long press
            timeouts[swIndex]=0;
            state[swIndex] = STATE_WAIT_RELEASE;
            switchesState[swIndex] = FRONT_PANEL_SW_STATE_JUST_PRESSED;
            break;
        }
        case STATE_WAIT_RELEASE:
        {
            if(digitalRead(getPin(swIndex))==HIGH) // released
            {
                // released, check time
                if(timeouts[swIndex]<TIMEOUT_SHORT_PRESS)
                    switchesState[swIndex] = FRONT_PANEL_SW_STATE_SHORT;
                else
                    switchesState[swIndex] = FRONT_PANEL_SW_STATE_LONG;
                // wait bounce again
                timeouts[swIndex]=0;
                state[swIndex] = STATE_WAIT_BOUNCE_RELEASE;                
            }
            if(timeouts[swIndex]>TIMEOUT_LONG_PRESS)
            {
                switchesState[swIndex] = FRONT_PANEL_SW_STATE_LONG;
                state[swIndex] = STATE_WAIT_RELEASE2;
            }
            
            break;
        }
        case STATE_WAIT_RELEASE2:
        {
            if(digitalRead(getPin(swIndex))==HIGH) // released
            {
                // wait bounce again
                timeouts[swIndex]=0;
                state[swIndex] = STATE_WAIT_BOUNCE_RELEASE;
            }          
            break; 
        }
        case STATE_WAIT_BOUNCE_RELEASE:
        {
            if(timeouts[swIndex]>=TIMEOUT_BOUNCE)
            {
              switchesState[swIndex] = FRONT_PANEL_SW_STATE_JUST_RELEASED;
              state[swIndex] = STATE_IDLE;
            }    
            break;
        }
    }
}


static int getPin(int swIndex)
{
    switch(swIndex)
    {
        case SW_MI:  return PIN_SW_MI;
        case SW_OJ:  return PIN_SW_OJ;
        case SW_PK:  return PIN_SW_PK;
        case SW_QL:  return PIN_SW_QL;        
        case SW_BACK:  return PIN_SW_BACK;        
        case SW_ENTER:  return PIN_SW_ENTER;        
        case SW_SHIFT:  return PIN_SW_SHIFT;        
    }
    return -1;
}

