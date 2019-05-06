#include "MIDIReception.h"
#include "MIDIManager.h"
#include "SequencerManager.h"
#include "FrontPanel.h"


#define STEPS_LEN         64
#define TRACKS_LEN        4
#define TRACK_0           0
#define VOICES_IN_STEP    6

#define ST_REC_WAIT_KEY 0

#define SEC_TO_TICK(S)  (S*1000)

struct S_SeqStepInfo
{
    MidiInfo notes[VOICES_IN_STEP];
    short int recIndex;
    short int playIndex;
    unsigned char isSilence;
};
typedef struct S_SeqStepInfo  SeqStepInfo;


static void recordStateMachine(void);
static void playStateMachine(void);

static int state;
static int state0;
static int stateRecord;
static unsigned int tempo;
static unsigned int currentTempoTicks;

static SeqStepInfo tracks[TRACKS_LEN][STEPS_LEN];
static int recordIndexes[TRACKS_LEN];
static int playIndexes[TRACKS_LEN];
static int maxIndexes[TRACKS_LEN];

static int gateOnPercent;
static int syncInType;
static int externalSyncEvent;

#define    SEQ_SYNC_TYPE_INTERNAL   0
#define    SEQ_SYNC_TYPE_EXTENAL    1
#define    SEQ_SYNC_TYPE_MIDI       2

void seq_init(void)
{
    state = SEQ_STATE_OFF;
    state0 = SEQ_STATE_OFF;
    stateRecord=ST_REC_WAIT_KEY;
    maxIndexes[TRACK_0] = 0;
    seq_setBpmRate(120);
    seq_setGateOnPercent(SEQ_GATE_ON_PERCENT_50);

    syncInType = SEQ_SYNC_TYPE_EXTENAL;
    externalSyncEvent = 0;
}

static volatile int tempoCounter;
static volatile int timeoutRelease;
void seq_sysTick(void)
{
    if(syncInType==SEQ_SYNC_TYPE_INTERNAL)
    {
        if(tempoCounter>0)
            tempoCounter--;
    }

    if(timeoutRelease>0)
      timeoutRelease--;
}

void seq_keyEvent(MidiInfo* pMidiInfo)
{
    if(state==SEQ_STATE_RECORD && stateRecord==ST_REC_WAIT_KEY)
    {
        if( (pMidiInfo->cmd==MIDI_CMD_NOTE_ON) && (recordIndexes[TRACK_0]<STEPS_LEN) )
        {
            int index = tracks[TRACK_0][recordIndexes[TRACK_0]].recIndex;
            if(index<VOICES_IN_STEP)
            {
              tracks[TRACK_0][recordIndexes[TRACK_0]].isSilence=0;
              tracks[TRACK_0][recordIndexes[TRACK_0]].notes[index] = *pMidiInfo; // save note info in recIndex position
              
              maxIndexes[TRACK_0] = recordIndexes[TRACK_0]; // save playIndex step
              tracks[TRACK_0][recordIndexes[TRACK_0]].playIndex = tracks[TRACK_0][recordIndexes[TRACK_0]].recIndex; // save playIndex note
              
              tracks[TRACK_0][recordIndexes[TRACK_0]].recIndex++; // inc recIndex for note
            }
        }
    }
}

void seq_nextStepEvent(void)
{
    recordIndexes[TRACK_0]++;
    tracks[TRACK_0][recordIndexes[TRACK_0]].recIndex=0;
}

void seq_tapRestEvent(void)
{
    if(state==SEQ_STATE_RECORD && stateRecord==ST_REC_WAIT_KEY)
    {        
        //tracks[TRACK_0][recordIndexes[TRACK_0]].cmd = 0xFF; // -1 for silence
        tracks[TRACK_0][recordIndexes[TRACK_0]].isSilence=1;
        
        maxIndexes[TRACK_0] = recordIndexes[TRACK_0]; // save playIndex step
        
        seq_nextStepEvent();   
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

void seq_setGateOnPercent(int value)
{
    gateOnPercent = value;
}
int seq_getGateOnPercent(void)
{
    return gateOnPercent;
}

void seq_externalSyncEvent(void)
{
    externalSyncEvent=1;
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
              midi_clearAllKeysPressed(); // set dco off
            
              recordIndexes[TRACK_0]=0; // init record index to 0 (step 0)
              tracks[TRACK_0][recordIndexes[TRACK_0]].recIndex=0;  // init rec index for notes to 0 (note 0)
          }
          recordStateMachine();
          break;
      }
  }
}

static void playStateMachine(void)
{
    MidiInfo* pmi;
    int i;
    int notesMax;

    if(frontp_getExternalSyncPulse())
    {
        frontp_resetExternalSyncPulse();
        externalSyncEvent=1;
    }
    
    if(syncInType!=SEQ_SYNC_TYPE_INTERNAL)
    {    
        if(externalSyncEvent==1)
        {
            externalSyncEvent=0;
            tempoCounter=0; 
        }
        else
            tempoCounter=1;
    }
    
    if(tempoCounter==0)
    {
        tempoCounter = currentTempoTicks;
        if(tracks[TRACK_0][playIndexes[TRACK_0]].isSilence==0)
        {
            notesMax = tracks[TRACK_0][playIndexes[TRACK_0]].playIndex;
            for(i=0; i<=notesMax; i++)
            {
                    pmi = &tracks[TRACK_0][playIndexes[TRACK_0]].notes[i];
                    pmi->cmd = MIDI_CMD_NOTE_ON;
                    midi_analizeMidiInfo(pmi);
            }
        }
        switch(gateOnPercent)
        {
            case SEQ_GATE_ON_PERCENT_25 :timeoutRelease = tempoCounter/4; break; // 25% of tempo          
            case SEQ_GATE_ON_PERCENT_50 :timeoutRelease = tempoCounter/2; break; // 50% of tempo          
            case SEQ_GATE_ON_PERCENT_75 :timeoutRelease = ((tempoCounter*3)/4); break; // 75% of tempo          
            case SEQ_GATE_ON_PERCENT_90:timeoutRelease = ((tempoCounter*9)/10); break; // 90% of tempo          
        }             
    }  

    if(timeoutRelease==0)
    {
        timeoutRelease=-1;    

        if(tracks[TRACK_0][playIndexes[TRACK_0]].isSilence==0)
        {
            notesMax = tracks[TRACK_0][playIndexes[TRACK_0]].playIndex;
            for(i=0; i<=notesMax; i++)
            {
                  pmi = &tracks[TRACK_0][playIndexes[TRACK_0]].notes[i];
                  pmi->cmd = MIDI_CMD_NOTE_OFF;
                  midi_analizeMidiInfo(pmi);
            }
        }        
        
        playIndexes[TRACK_0]++;
        if(playIndexes[TRACK_0]>maxIndexes[TRACK_0])
        {
            playIndexes[TRACK_0]=0;
        }        
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


