
void dco_init(void);
void dco_setNote(int note);
void dco_releaseNote(int note);

void dco_disableVoice(int index);

void pwmm_setValuePwmSlow(unsigned char index,unsigned int value);

#define PWM_FAST_MAX_VALUE 572

