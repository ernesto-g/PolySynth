
void dco_init(void);
int dco_setNote(int note, int vel);
void dco_releaseVoice(int voice);

void dco_disableVoice(int index);

void pwmm_setValuePwmSlow(unsigned char index,unsigned int value);

void dco_setWaveForm(int wf);
int dco_getWaveForm(void);

void dco_lfoOn(void);
void dco_lfoOff(void);
void dco_setLfoFreq(int freq);
int dco_getLfoFrq(void);

int dco_getLfoWaveForm(void);
void dco_setLfoWaveForm(int wf);



#define PWM_FAST_MAX_VALUE 572

