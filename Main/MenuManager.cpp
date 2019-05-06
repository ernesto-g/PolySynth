#include <Arduino.h>
#include "U8g2lib.h"
#include "U8x8lib.h"
#include "FrontPanel.h"
#include "Dco.h"
#include "AdsrManager.h"
#include "MIDIReception.h"
#include "SequencerManager.h"

char* MAIN_MENU_TXTS[]= {"SAMPLES SYNTH","DRUM MACHINE","CLASSIC SYNTH","SEQUENCER","CONFIG"};
#define MAIN_MENU_TXTS_LEN  5

char* SAMPLES_NAMES_TXTS[]= {"CASIO_MT600","MARIMBA","GUITAR 12STR","FUSION BASS"};
#define SAMPLES_NAMES_TXTS_LEN  4

char* LFO_WAVEFORM_NAMES[]= {"Random","Square","Exponential","Triangle","Sine"};


U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI display(U8G2_R0, /* cs=*/ 49, /* dc=*/ 51, /* reset=*/ 53);
static int mainMenuState;
static int mainSelectedItem;
static int shiftActive=0;

// Private functions
static void printList(char* pList[],int listLen,int selectedItem);
static void showSamplesSinthScreen(void);
static void showSamplesSynthMainScreen(int selectedItem);
static void showSequencerMainScreen(void);

static void miniPianoTest(void);

static void samplesSynthMainScreenManager(void);
static void sequencerMainScreenManager(void);
//___________________


#define STATE_MAIN          0
#define STATE_SAMPLES_SYNTH 1
#define STATE_DRUM_MACHINE  2
#define STATE_CLASSIC_SYNTH 3
#define STATE_SEQUENCER     4
#define STATE_CONFIG        5
#define STATE_SAMPLES_SYNTH_MAIN_SCREEN 6


void menu_init(void)
{
    display.begin();
    display.setPowerSave(0);
    mainMenuState = STATE_MAIN;
    mainSelectedItem = 0;
}

void menu_loop(void)
{
    if(frontp_getSwState(SW_SHIFT)==FRONT_PANEL_SW_STATE_JUST_PRESSED || frontp_getSwState(SW_SHIFT)==FRONT_PANEL_SW_STATE_LONG)
        shiftActive = 1;
    else
        shiftActive = 0;

    
    switch(mainMenuState)
    {
        case STATE_MAIN:
        {
            // Read selected item from encoder
            mainSelectedItem = frontp_getEncoderPosition(0);
            if(mainSelectedItem<0)
            {
                mainSelectedItem = 0;
                frontp_setEncoderPosition(0,mainSelectedItem);
            }
            if(mainSelectedItem>=MAIN_MENU_TXTS_LEN)
            {
                mainSelectedItem = MAIN_MENU_TXTS_LEN-1;
                frontp_setEncoderPosition(0,mainSelectedItem);
            }
            //________________________________

            printList(MAIN_MENU_TXTS,MAIN_MENU_TXTS_LEN,mainSelectedItem);  

            if(frontp_getSwState(SW_ENTER)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
            {
                frontp_resetSwState(SW_ENTER);
                switch(mainSelectedItem)
                {
                    case 0: mainMenuState = STATE_SAMPLES_SYNTH;frontp_setEncoderPosition(0,0);break;
                    case 1: mainMenuState = STATE_DRUM_MACHINE;break;
                    case 2: mainMenuState = STATE_CLASSIC_SYNTH;break;
                    case 3: mainMenuState = STATE_SEQUENCER; frontp_setEncoderPosition(0,seq_getBpmRate()); break;
                    case 4: mainMenuState = STATE_CONFIG;break;
                }
            }
            break;
        }
        case STATE_SAMPLES_SYNTH:
        {
            // Read selected item from encoder
            int val = frontp_getEncoderPosition(0);
            if(val<0)
            {
                val = 0;
                frontp_setEncoderPosition(0,val);
            }
            if(val>=MAIN_MENU_TXTS_LEN)
            {
                val = MAIN_MENU_TXTS_LEN-1;
                frontp_setEncoderPosition(0,val);
            }
            dco_setWaveForm(val);
            //________________________________

            showSamplesSinthScreen();

            if(frontp_getSwState(SW_ENTER)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
            {
                frontp_resetSwState(SW_ENTER);
                mainMenuState = STATE_SAMPLES_SYNTH_MAIN_SCREEN;

                // load encoders initial values
                frontp_setEncoderPosition(0,adsr_getMidiAttackRate(0));
                frontp_setEncoderPosition(1,adsr_getMidiDecayRate(0));
                frontp_setEncoderPosition(2,adsr_getMidiSustainValue(0));
                frontp_setEncoderPosition(3,adsr_getMidiReleaseRate(0));
                //_____________________
            }
            if(frontp_getSwState(SW_BACK)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
            {
                frontp_resetSwState(SW_BACK);
                mainMenuState = STATE_MAIN;
                frontp_setEncoderPosition(0,0);
            }
            
            // Mini piano test
            miniPianoTest();
        
            break;
        }
        case STATE_SEQUENCER:
        {
            sequencerMainScreenManager();
            break;
        }
        case STATE_SAMPLES_SYNTH_MAIN_SCREEN:
        {
            samplesSynthMainScreenManager();
            break;
        }
      
    }
}


static void samplesSynthMainScreenManager(void)
{
    int val;
    int i;
    static int selectedItem=0;
    static int flagReloadEncoders=0;
    static int shiftActive0=-1;
  
    showSamplesSynthMainScreen(selectedItem);

    if(shiftActive!=shiftActive0)
    {
        shiftActive0=shiftActive;
        flagReloadEncoders=1;
    }

    // controls
    if(frontp_getSwState(SW_BACK)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
        frontp_resetSwState(SW_BACK);
        frontp_setEncoderPosition(0,dco_getWaveForm()); 
        mainMenuState = STATE_SAMPLES_SYNTH;
        selectedItem = 0;
        return;
    }

    // Item selected
    if(frontp_getSwState(SW_MI)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
         frontp_resetSwState(SW_MI);
         selectedItem++;
         if(selectedItem>=2)
          selectedItem=0;  

         flagReloadEncoders=1;
    }

    // adsr fast/slow
    if(frontp_getSwState(SW_OJ)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
         frontp_resetSwState(SW_OJ);
         if(shiftActive==0)
         {   // Configure adsr fast/slow setting
             int f = adsr_getFlagEnvLowSpeed();
             f++;
             if(f>=2)
                f = 0;
             adsr_setFlagEnvLowSpeed(f); 
         }
         else
         {
             // Configure adsr VCF mode setting
             int m = adsr_getVcfMode();
             m++;
             if(m>=2)
                m = 0;
             adsr_setVcfMode(m);          
         }
    }

    if(selectedItem==0)
    {
          if(flagReloadEncoders)
          {
              flagReloadEncoders=0;
              if(shiftActive==0)
              {
                  frontp_setEncoderPosition(0,adsr_getMidiAttackRate(0));
                  frontp_setEncoderPosition(1,adsr_getMidiDecayRate(0));
                  frontp_setEncoderPosition(2,adsr_getMidiSustainValue(0));
                  frontp_setEncoderPosition(3,adsr_getMidiReleaseRate(0));
              }
              else
              {
                  frontp_setEncoderPosition(0,adsr_getMidiAttackRate(6));
                  frontp_setEncoderPosition(1,adsr_getMidiDecayRate(6));
                  frontp_setEncoderPosition(2,adsr_getMidiSustainValue(6));
                  frontp_setEncoderPosition(3,adsr_getMidiReleaseRate(6));                
              }
          }
          // Modify ADSR1 (for voices amplitude)
          
          // update attack value
          val = frontp_getEncoderPosition(0);
          if(val>127) {
              val=127;
              frontp_setEncoderPosition(0,val);    
          }
          else if(val <0) {
              val=0;
              frontp_setEncoderPosition(0,val);          
          }
          if(shiftActive==0)
          {
            for(i=0; i<6;i++)
              adsr_setMidiAttackRate(i,val); // attack for voices
          }
          else
          {
              adsr_setMidiAttackRate(6,val); // attack for VCF            
          }
          //_____________________      
          
          // update decay value
          val = frontp_getEncoderPosition(1);
          if(val>127) {
              val=127;
              frontp_setEncoderPosition(1,val);    
          }
          else if(val <0) {
              val=0;
              frontp_setEncoderPosition(1,val);          
          }
          if(shiftActive==0)
          {
              for(i=0; i<6;i++)
                adsr_setMidiDecayRate(i,val); // decay for voices
          }
          else
            adsr_setMidiDecayRate(6,val); // decay for vcf
          //_____________________      
          
          // update sustain value
          val = frontp_getEncoderPosition(2);
          if(val>127) {
              val=127;
              frontp_setEncoderPosition(2,val);    
          }
          else if(val <0) {
              val=0;
              frontp_setEncoderPosition(2,val);          
          }
          if(shiftActive==0)
          {
              for(i=0; i<6;i++)
                adsr_setMidiSustainValue(i,val); // sustain for voices
          }
          else
            adsr_setMidiSustainValue(6,val); // sustain for vcf
          //_____________________     
      
          // update release value
          val = frontp_getEncoderPosition(3);
          if(val>127) {
              val=127;
              frontp_setEncoderPosition(3,val);    
          }
          else if(val <0) {
              val=0;
              frontp_setEncoderPosition(3,val);          
          }
          if(shiftActive==0)
          {
            for(i=0; i<6;i++)
              adsr_setMidiReleaseRate(i,val); // release for voices
          }
          else
            adsr_setMidiReleaseRate(6,val); // release for voices
          //_____________________      
    }
    else
    {
          if(flagReloadEncoders)
          {
              flagReloadEncoders=0;
              frontp_setEncoderPosition(0,dco_getLfoFrq());
              frontp_setEncoderPosition(1,dco_getLfoWaveForm());
              frontp_setEncoderPosition(2,dc0_getLfoAmplitudeAmt());
          }
                
         // LFO settegins
          val = frontp_getEncoderPosition(0);
          if(val>30){
              val=30;
              frontp_setEncoderPosition(0,val);
          }
          else if(val<1)
          {
              val=1;
              frontp_setEncoderPosition(0,val);
          }          
          dco_setLfoFreq(val);

          val = frontp_getEncoderPosition(1);
          if(val>4){
              val=4;
              frontp_setEncoderPosition(1,val);
          }
          else if(val<0)
          {
              val=0;
              frontp_setEncoderPosition(1,val);
          }          
          dco_setLfoWaveForm(val);    

          val = frontp_getEncoderPosition(2);
          if(val>100){
              val=100;
              frontp_setEncoderPosition(2,val);
          }
          else if(val<0)
          {
              val=0;
              frontp_setEncoderPosition(2,val);
          }          
          dc0_setLfoAmplitudeAmt(val);                  
    }
}


static void sequencerMainScreenManager(void)
{
    int val;
    int i;
    static int flagReloadEncoders=0;
    static int shiftActive0=-1;
  
    showSequencerMainScreen();

    if(shiftActive!=shiftActive0)
    {
        shiftActive0=shiftActive;
        flagReloadEncoders=1;
    }
    if(frontp_getSwState(SW_BACK)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
        frontp_resetSwState(SW_BACK);
        frontp_setEncoderPosition(0,3);
        mainMenuState = STATE_MAIN;
        return;
    }
    

    // Change sequencer mode (play/record/off)
    if(frontp_getSwState(SW_MI)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
          frontp_resetSwState(SW_MI);
          int s = seq_getState();
          s++;
          if(s>SEQ_STATE_RECORD)
            s = SEQ_STATE_OFF;  
          seq_setState(s);       
    } 

    // next step
    if(frontp_getSwState(SW_OJ)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
          frontp_resetSwState(SW_OJ);
          seq_nextStepEvent();
    } 
    
    // rest
    if(frontp_getSwState(SW_PK)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
          frontp_resetSwState(SW_PK);
          seq_tapRestEvent();
    } 

    // sync int type / gate on percent
    if(frontp_getSwState(SW_QL)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
          frontp_resetSwState(SW_QL);
          if(shiftActive)
          {
              // sync type selection
              int val = seq_getExternalSyncType();
              val++;
              if(val>SEQ_SYNC_TYPE_MIDI)
                val=SEQ_SYNC_TYPE_INTERNAL;
                
              seq_setExternalSyncType(val);
          }
          else
          {
              // gate on percent selection 
              int val = seq_getGateOnPercent();
              val++;
              if(val>SEQ_GATE_ON_PERCENT_90)
                  val = SEQ_GATE_ON_PERCENT_25;
                  
              seq_setGateOnPercent(val);     
          }
    }
    
    
    // BPM value
    val = frontp_getEncoderPosition(0);
    if(val>350) {
        val=350;
        frontp_setEncoderPosition(0,val);    
    }
    else if(val <0) {
        val=0;
        frontp_setEncoderPosition(0,val);          
    }
    seq_setBpmRate(val);
    //_________

              
}




static void printList(char* pList[],int listLen,int selectedItem)
{
      char* pLines[4];
      if(selectedItem<listLen)
        pLines[0] = pList[selectedItem];
      else
        pLines[0] = ""; 
      selectedItem++;
      if(selectedItem<listLen)
        pLines[1] = pList[selectedItem];
      else
        pLines[1] = ""; 
      selectedItem++;
      if(selectedItem<listLen)
        pLines[2] = pList[selectedItem];
      else
        pLines[2] = ""; 
      selectedItem++;
      if(selectedItem<listLen)
        pLines[3] = pList[selectedItem];
      else
        pLines[3] = ""; 
   
      // Display update spi: 12ms 
      display.firstPage();
      do 
      {
          display.setFont(u8g2_font_8x13B_tf); 
          display.drawStr(0, 16, pLines[0]); 
          display.drawStr(0, 32, pLines[1]); 
          display.drawStr(0, 48, pLines[2]); 
          display.drawStr(0, 64, pLines[3]); 

          display.setFont(u8g2_font_open_iconic_arrow_2x_t); 
          display.drawStr(112, 18, "\x45");
          
      }
      while(display.nextPage()); 
      //__________________________   
}

static void showSamplesSinthScreen(void)
{
    printList(SAMPLES_NAMES_TXTS,SAMPLES_NAMES_TXTS_LEN,dco_getWaveForm());
}
static void showSamplesSynthMainScreen(int selectedItem)
{
      char* pTitle = SAMPLES_NAMES_TXTS[dco_getWaveForm()];
      char txtAdsr0[32];
      char txtAdsr1[32];
      char txtLFOLine0[32];
      char txtLFOLine1[32];
      char txtLFOLine2[32];
      char txtShift[3]={'s','h','\0'};
      char txtMode[3]={'P','O','\0'};
      char txtAdsrSlow[7]={"(slow)"};

      if(adsr_getFlagEnvLowSpeed()==0)
        txtAdsrSlow[0]=0;
      
      if(selectedItem==0)
      {
          if(shiftActive==0)
          {
              sprintf(txtAdsr1,"ADSR1 SETTINGS%s",txtAdsrSlow);
              sprintf(txtAdsr0,"%03d %03d %03d %03d",adsr_getMidiAttackRate(0),adsr_getMidiDecayRate(0),adsr_getMidiSustainValue(0),adsr_getMidiReleaseRate(0));
          }
          else
          {
              sprintf(txtAdsr1,"ADSR2 SETTINGS%s M%d",txtAdsrSlow,adsr_getVcfMode());
              sprintf(txtAdsr0,"%03d %03d %03d %03d",adsr_getMidiAttackRate(6),adsr_getMidiDecayRate(6),adsr_getMidiSustainValue(6),adsr_getMidiReleaseRate(6));
          }
      }
      else
      {
          sprintf(txtAdsr1,"ADSR1 SETTINGS%s",txtAdsrSlow);
          sprintf(txtAdsr0,"%03d %03d %03d %03d",adsr_getMidiAttackRate(0),adsr_getMidiDecayRate(0),adsr_getMidiSustainValue(0),adsr_getMidiReleaseRate(0));        
      }
      sprintf(txtLFOLine0,"%02dHz",dco_getLfoFrq());
      sprintf(txtLFOLine1,"WAVEFORM:%s",LFO_WAVEFORM_NAMES[dco_getLfoWaveForm()]);
      sprintf(txtLFOLine2,"AMT:%03d",dc0_getLfoAmplitudeAmt());
      
      // Display update spi: 12ms 
      display.firstPage();
      do 
      {
          display.setFont(u8g2_font_5x8_tf); 
          display.drawStr(0, 8, pTitle); 

          display.drawStr(100, 8, txtMode); 
          if(shiftActive==1)
            display.drawStr(116, 8, txtShift); 

          display.drawLine(0, 8, 127, 8); // line separator
   

          display.setFont(u8g2_font_5x8_tf); 
          display.drawStr(5, 17, txtAdsr1);
          display.drawStr(5, 37, "LFO SETTINGS");
          
          display.setFont(u8g2_font_8x13_tf); 
          display.drawStr(5, 27, txtAdsr0); // ADSR values
          display.drawStr(5, 48, txtLFOLine0); // LFO
          display.drawStr(70, 48, txtLFOLine2); // LFO

          display.setFont(u8g2_font_6x13_tf); 
          display.drawStr(5, 61, txtLFOLine1); // LFO
          

          if(selectedItem==0)
              display.drawBox(0, 12, 4, 4); // selected item 
          else
              display.drawBox(0, 32, 4, 4); // selected item 
      
           
      }
      while(display.nextPage()); 
      //__________________________   
}

static void showSequencerMainScreen(void)
{
      char txtSteps[32];
      char txtSync[32];
      char txtBpm[32];
      char play[2]={0x45,0};
      char record[2]={0x46,0};
      char next[2]={0x4A,0};
      char rest[2]={0x44,0};
      char stopTxt[2]={0x4B,0};
      char gateOnTxt[16];

      sprintf(txtSteps,"STEP:%02d",seq_getCurrentRecordStep()); 
      sprintf(txtBpm,"BPM:%03d",seq_getBpmRate());
      switch(seq_getGateOnPercent())
      {
          case SEQ_GATE_ON_PERCENT_25: sprintf(gateOnTxt,"Gate:25%%");break;
          case SEQ_GATE_ON_PERCENT_50: sprintf(gateOnTxt,"Gate:50%%");break;
          case SEQ_GATE_ON_PERCENT_75: sprintf(gateOnTxt,"Gate:75%%");break;
          case SEQ_GATE_ON_PERCENT_90: sprintf(gateOnTxt,"Gate:90%%");break;
      }
      switch(seq_getExternalSyncType())
      {
          case SEQ_SYNC_TYPE_INTERNAL:sprintf(txtSync,"SYNC:%s","INT");break;
          case SEQ_SYNC_TYPE_EXTERNAL:sprintf(txtSync,"SYNC:%s","EXT");break;
          case SEQ_SYNC_TYPE_MIDI:sprintf(txtSync,"SYNC:%s","MIDI");break;
      }
      
                
      // Display update spi: 12ms 
      display.firstPage();
      do 
      {
          display.setFont(u8g2_font_5x8_tf); 
          display.drawStr(0, 8, "SEQUENCER"); 
          if(shiftActive==1)
            display.drawStr(116, 8, "sh"); 
          display.drawLine(0, 8, 127, 8); // line separator

          display.drawStr(85, 18, gateOnTxt );

          display.setFont(u8g2_font_6x13_tf); 
          display.drawStr(0, 56, txtSync); // sync type 

          display.setFont(u8g2_font_8x13B_tf); 
          display.drawStr(0, 21, txtSteps); // Step value
          display.drawStr(70, 56, txtBpm); // Bpm value 
         
          // buttons
          display.setFont(u8g2_font_open_iconic_play_2x_t);
          if(seq_getState()==SEQ_STATE_PLAY)
            display.drawStr(10, 41, play);
          else if(seq_getState()==SEQ_STATE_RECORD)
            display.drawStr(10, 41, record);
          else if(seq_getState()==SEQ_STATE_OFF)
            display.drawStr(10, 41, stopTxt);
                     
          display.drawStr(52, 41, next); 
          display.setFont(u8g2_font_open_iconic_check_2x_t);          
          display.drawStr(96, 41, rest); 
          //________                   

      }
      while(display.nextPage()); 
      //__________________________   
}



static void miniPianoTest(void)
{
    static int voiceSw0;
    static int voiceSw1;
    static int voiceSw2;
    static int voiceSw3;
    
    if(frontp_getSwState(SW_MI)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
        Serial.print("press C4\n");
        voiceSw0 = dco_setNote(60, 127); // 60:C4
        frontp_resetSwState(SW_MI);
    }
    if(frontp_getSwState(SW_MI)==FRONT_PANEL_SW_STATE_JUST_RELEASED)
    {
        Serial.print("release C4\n");
        dco_releaseVoice(voiceSw0);
        frontp_resetSwState(SW_MI);
    }

    if(frontp_getSwState(SW_OJ)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
        voiceSw1 = dco_setNote(62, 127); // 62:D4
        frontp_resetSwState(SW_OJ);
    }
    if(frontp_getSwState(SW_OJ)==FRONT_PANEL_SW_STATE_JUST_RELEASED)
    {
        dco_releaseVoice(voiceSw1);
        frontp_resetSwState(SW_OJ);
    }


    if(frontp_getSwState(SW_PK)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
        voiceSw2 = dco_setNote(64, 127); // 64:E4
        frontp_resetSwState(SW_PK);
    }
    if(frontp_getSwState(SW_PK)==FRONT_PANEL_SW_STATE_JUST_RELEASED)
    {
        dco_releaseVoice(voiceSw2);
        frontp_resetSwState(SW_PK);
    }

    if(frontp_getSwState(SW_QL)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
    {
        voiceSw3 = dco_setNote(66, 127); // 6g:F4
        frontp_resetSwState(SW_QL);
    }
    if(frontp_getSwState(SW_QL)==FRONT_PANEL_SW_STATE_JUST_RELEASED)
    {
        dco_releaseVoice(voiceSw3);
        frontp_resetSwState(SW_QL);
    }        
}

