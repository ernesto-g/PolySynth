void adsr_stateMachineTick(void);
void adsr_init(void);

void adsr_gateOnEvent(void);
int adsr_gateOffEvent(int index);
void adsr_triggerEvent(int index, int vel);
int adsr_getFreeAdsr(int indexMax);

void adsr_setMidiAttackRate(int i, int value);
void adsr_setMidiDecayRate(int i, int value);
void adsr_setMidiReleaseRate(int i, int value);
void adsr_setMidiSustainValue(int i, int value);

int adsr_getMidiAttackRate(int i);
int adsr_getMidiDecayRate(int i);
int adsr_getMidiReleaseRate(int i);
int adsr_getMidiSustainValue(int i);

void adsr_setFlagEnvLowSpeed(int value);
int adsr_getFlagEnvLowSpeed(void);

void adsr_setVcfMode(int mode);
int adsr_getVcfMode(void);

int adsr_areAllIdle(void);

