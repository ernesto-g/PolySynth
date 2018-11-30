void dco_init(void);
void dco_setLpfFc(signed int fcValue);

#define DCO_SYNTH_MODE_SYNTH      0
#define DCO_SYNTH_MODE_PIANO      1
#define DCO_SYNTH_MODE_MELLOTRON  2

#define SYNTH_WAVEFORM_SQUARE     0
#define SYNTH_WAVEFORM_SAW        1
#define SYNTH_WAVEFORM_TRIANGLE   2


void dco_setNote(int note);

