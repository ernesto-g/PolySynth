#include <Arduino.h>
#include "U8g2lib.h"
#include "U8x8lib.h"
#include "FrontPanel.h"
#include "Dco.h"

char* MAIN_MENU_TXTS[]= {"SAMPLES SYNTH","DRUM MACHINE","CLASSIC SYNTH","CONFIG"};
#define MAIN_MENU_TXTS_LEN  4

char* SAMPLES_NAMES_TXTS[]= {"CASIO_MT600","MARIMBA","GUITAR 12STR","FUSION BASS"};
#define SAMPLES_NAMES_TXTS_LEN  4


U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI display(U8G2_R0, /* cs=*/ 49, /* dc=*/ 51, /* reset=*/ 53);
static int mainMenuState;
static int mainSelectedItem;


static void printList(char* pList[],int listLen,int selectedItem);
static void showSamplesSinthScreen(void);
static void miniPianoTest(void);



#define STATE_MAIN          0
#define STATE_SAMPLES_SYNTH 1
#define STATE_DRUM_MACHINE  2
#define STATE_CLASSIC_SYNTH 3
#define STATE_CONFIG        4


void menu_init(void)
{
    display.begin();
    display.setPowerSave(0);
    mainMenuState = STATE_MAIN;
    mainSelectedItem = 0;
}

void menu_loop(void)
{
    static int mainSelectedItem0=-1;
    
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

            if(mainSelectedItem!=mainSelectedItem0)
            {               
                printList(MAIN_MENU_TXTS,MAIN_MENU_TXTS_LEN,mainSelectedItem);  
                mainSelectedItem0=mainSelectedItem;
            }
            if(frontp_getSwState(SW_ENTER)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
            {
                switch(mainSelectedItem)
                {
                    case 0: mainMenuState = STATE_SAMPLES_SYNTH;frontp_setEncoderPosition(0,0);break;
                    case 1: mainMenuState = STATE_DRUM_MACHINE;break;
                    case 2: mainMenuState = STATE_CLASSIC_SYNTH;break;
                    case 3: mainMenuState = STATE_CONFIG;break;
                }
            }
            break;
        }
        case STATE_SAMPLES_SYNTH:
        {
            // Read selected item from encoder
            int val = frontp_getEncoderPosition(0);
            static int val0=-1;
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

            if(val!=val0)
            {
                val0=val;          
                showSamplesSinthScreen();
            }

            
            // Mini piano test
            miniPianoTest();
        
            break;
        }
      
    }

  /*
    char line1[16];
  char line2[16];
  char line3[16];


        // test
      sprintf(line1,"A:%d B:%d",frontp_getEncoderPosition(0),frontp_getEncoderPosition(1));
      sprintf(line2,"C:%d D:%d",frontp_getEncoderPosition(2),frontp_getEncoderPosition(3));
      
      sprintf(line3,"SW OFF");            
      if(frontp_getSwState(SW_MI)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_MI ON");
      if(frontp_getSwState(SW_OJ)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_OJ ON");
      if(frontp_getSwState(SW_PK)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_PK ON");
      if(frontp_getSwState(SW_QL)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_QL ON");

      if(frontp_getSwState(SW_BACK)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
      {
        sprintf(line3,"SW_BACK ON");
        int voice = dco_setNote(35, 127);
        delay(4000);
        dco_releaseVoice(voice);
      }
      if(frontp_getSwState(SW_ENTER)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
      {
        sprintf(line3,"SW_ENTER ON");
        int voice = dco_setNote(24, 127); // 36:c2
        delay(4000);
        dco_releaseVoice(voice);
      }
      if(frontp_getSwState(SW_SHIFT)==FRONT_PANEL_SW_STATE_JUST_PRESSED)
        sprintf(line3,"SW_SHIFT ON");
      
      //___________________
     
   */
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

