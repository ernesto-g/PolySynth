void adsr_stateMachineTick(void);
void adsr_init(void);

void adsr_gateOnEvent(void);
void adsr_gateOffEvent(void);
void adsr_triggerEvent(int index, int vel);
int adsr_getFreeAdsr(int indexMax);

void adsr_setMidiAttackRate(int i, int value);
void adsr_setMidiDecayRate(int i, int value);
void adsr_setMidiReleaseRate(int i, int value);
void adsr_setMidiSustainValue(int i, int value);


