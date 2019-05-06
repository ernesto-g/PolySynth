

void seq_init(void);
void seq_sysTick(void);
void seq_keyEvent(MidiInfo* pMidiInfo);
void seq_loop(void);
void seq_setState(int s);
void seq_setBpmRate(int rate);
void seq_tapRestEvent(void);
int seq_getBpmRate(void);
int seq_getState(void);
int seq_getCurrentRecordStep(void);
void seq_nextStepEvent(void);
void seq_setGateOnPercent(int value);
int seq_getGateOnPercent(void);
void seq_externalSyncEvent(void);
void seq_setExternalSyncType(int val);
int seq_getExternalSyncType(void);


#define SEQ_STATE_OFF     0
#define SEQ_STATE_PLAY    1
#define SEQ_STATE_RECORD  2

#define SEQ_GATE_ON_PERCENT_25   0
#define SEQ_GATE_ON_PERCENT_50   1
#define SEQ_GATE_ON_PERCENT_75   2
#define SEQ_GATE_ON_PERCENT_90   3

#define    SEQ_SYNC_TYPE_INTERNAL   0
#define    SEQ_SYNC_TYPE_EXTERNAL    1
#define    SEQ_SYNC_TYPE_MIDI       2



