
#define FRONT_PANEL_SW_STATE_IDLE           0
#define FRONT_PANEL_SW_STATE_JUST_PRESSED   1
#define FRONT_PANEL_SW_STATE_SHORT          2
#define FRONT_PANEL_SW_STATE_LONG           3
#define FRONT_PANEL_SW_STATE_JUST_RELEASED  4

#define SW_MI     0
#define SW_OJ     1  
#define SW_PK     2
#define SW_QL     3  
#define SW_BACK   4
#define SW_ENTER  5
#define SW_SHIFT  6

void frontp_init(void);
int frontp_getEncoderPosition(int index);
void frontp_setEncoderPosition(int index,int pos);

int frontp_getSwState(int swIndex);
void frontp_resetSwState(int swIndex);

void frontp_loop(void);
void frontp_tick1Ms(void);

int frontp_getExternalSyncPulse(void);
void frontp_resetExternalSyncPulse(void);
