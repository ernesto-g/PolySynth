#include <Arduino.h>
#include "RotaryEncoder.h"
#include "FrontPanel.h"

#define PIN_1_ENC_A   33
#define PIN_2_ENC_A   35

#define PIN_1_ENC_B   37
#define PIN_2_ENC_B   39

#define PIN_1_ENC_C   41
#define PIN_2_ENC_C   43

#define PIN_1_ENC_D   45
#define PIN_2_ENC_D   47


static RotaryEncoder encoderA(PIN_1_ENC_A, PIN_2_ENC_A);
static RotaryEncoder encoderB(PIN_1_ENC_B, PIN_2_ENC_B);
static RotaryEncoder encoderC(PIN_1_ENC_C, PIN_2_ENC_C);
static RotaryEncoder encoderD(PIN_1_ENC_D, PIN_2_ENC_D);



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

    frontp_setEncoderPosition(0,0);
    frontp_setEncoderPosition(1,0);
    frontp_setEncoderPosition(2,0);
    frontp_setEncoderPosition(3,0);
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



