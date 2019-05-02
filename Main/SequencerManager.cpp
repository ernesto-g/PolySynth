#include "MIDIReception.h"
#include "MIDIManager.h"
#include "SequencerManager.h"

#define STEPS_LEN   64
#define TRACKS_LEN  4
#define TRACK_0     0

#define ST_REC_WAIT_KEY 0

#define SEC_TO_TICK(S)  (S*1000)

static void recordStateMachine(void);
static void playStateMachine(void);

static int state;
static int state0;
static int stateRecord;
static unsigned int tempo;
static unsigned int currentTempoTicks;

static MidiInfo tracks[TRACKS_LEN][STEPS_LEN];
static int recordIndexes[TRACKS_LEN];
static int playIndexes[TRACKS_LEN];
static int maxIndexes[TRACKS_LEN];


void seq_init(void)
{
    state = SEQ_STATE_OFF;
    state0 = SEQ_STATE_OFF;
    stateRecord=ST_REC_WAIT_KEY;
    maxIndexes[TRACK_0] = 0;
    seq_setBpmRate(120);
}

static volatile int tempoCounter;
static volatile int timeoutRelease;
void seq_sysTick(void)
{
    if(tempoCounter>0)
      tempoCounter--;

    if(timeoutRelease>0)
      timeoutRelease--;
}

void seq_keyEvent(MidiInfo* pMidiInfo)
{
    if(state==SEQ_STATE_RECORD && stateRecord==ST_REC_WAIT_KEY)
    {
        if( (pMidiInfo->cmd==MIDI_CMD_NOTE_ON) && (recordIndexes[TRACK_0]<STEPS_LEN) )
        {
            tracks[TRACK_0][recordIndexes[TRACK_0]] = *pMidiInfo;
            maxIndexes[TRACK_0] = recordIndexes[TRACK_0];
            recordIndexes[TRACK_0]++;
        }
    }
}

void seq_tapRestEvent(void)
{
    if(state==SEQ_STATE_RECORD && stateRecord==ST_REC_WAIT_KEY)
    {
        tracks[TRACK_0][recordIndexes[TRACK_0]].cmd = 0xFF; // -1 for silence
        maxIndexes[TRACK_0] = recordIndexes[TRACK_0];
        recordIndexes[TRACK_0]++;    
    }  
}

void seq_setState(int s)
{
    state=s;
}
void seq_setBpmRate(int rate)
{
    tempo = rate;
    currentTempoTicks = (unsigned int)SEC_TO_TICK( (60.0/tempo) );
}
int seq_getBpmRate(void)
{
    return tempo;
}
int seq_getState(void)
{
    return state;
}
int seq_getCurrentRecordStep(void)
{
    return recordIndexes[TRACK_0];
}


void seq_loop(void)
{
  int flagNewState=0;
  
  if(state!=state0)
  {
      state0=state;
      flagNewState=1;
  }
  
  switch(state)
  {
      case SEQ_STATE_OFF:
      {
          if(flagNewState==1)
          {
              midi_clearAllKeysPressed(); // set dco off
          } 
          break;
      }
      case SEQ_STATE_PLAY:
      {
          if(flagNewState==1)
          {
              playIndexes[TRACK_0] = 0;   
          }
          playStateMachine();
          break;
      }
      case SEQ_STATE_RECORD:
      {
          if(flagNewState==1)
          {
              recordIndexes[TRACK_0]=0; // init record index to 0 
          }
          recordStateMachine();
          break;
      }
  }
}

static void playStateMachine(void)
{
    MidiInfo* pmi;
    if(tempoCounter==0)
    {
        tempoCounter = currentTempoTicks;
        pmi = &tracks[TRACK_0][playIndexes[TRACK_0]];
        if(pmi->cmd!=0xFF)
        {
            pmi->cmd = MIDI_CMD_NOTE_ON;
            midi_analizeMidiInfo(pmi);
        }
        timeoutRelease = tempoCounter/4; // 25% of tempo        
        //outs_set(OUT_LED_SEQ_RATE,1); // led on
    }  

    if(timeoutRelease==0)
    {
        timeoutRelease=-1;      
        pmi = &tracks[TRACK_0][playIndexes[TRACK_0]];
        if(pmi->cmd!=0xFF)
        {
            pmi->cmd = MIDI_CMD_NOTE_OFF;
            midi_analizeMidiInfo(pmi);
        }
        
        playIndexes[TRACK_0]++;
        if(playIndexes[TRACK_0]>maxIndexes[TRACK_0])
        {
            playIndexes[TRACK_0]=0;
        }        
        //outs_set(OUT_LED_SEQ_RATE,0); // led off
    }
}

static void recordStateMachine(void)
{
    switch(stateRecord)
    {
        case ST_REC_WAIT_KEY:
        {
            break;  
        }
    }
}


