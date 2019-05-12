#include "MIDIReception.h"
#include "MIDIManager.h"
#include "Dco.h"

// Types
typedef struct S_KeyPressedInfo {
  int note;
  int voice; 
  byte flagFree;
}KeyPressedInfo;

// Defines
#define KEYS_PRESSED_LEN    11

// Private variables
static int currentMidiChannel;
static KeyPressedInfo keysPressed[KEYS_PRESSED_LEN];

// Private functions
static int saveKey(int note, int voice);
static int getIndexOfPressedKey(int note);
static int deleteKey(int note);



void midi_init(void)
{
    int i;
    currentMidiChannel=0; // read from eeprom configuration

    for(i=0; i<KEYS_PRESSED_LEN; i++)
      keysPressed[i].flagFree=1;
}

void midi_analizeMidiInfo(MidiInfo * pMidiInfo)
{
    if(pMidiInfo->channel==currentMidiChannel)
    {
        if(pMidiInfo->cmd==MIDI_CMD_NOTE_ON)
        {
            int assignedVoice = dco_setNote(pMidiInfo->note,pMidiInfo->vel);
            if(assignedVoice>=0)
            {
                saveKey(pMidiInfo->note,assignedVoice);
            }
        }
        else if(pMidiInfo->cmd==MIDI_CMD_NOTE_OFF)
        {
             int assignedVoice = deleteKey(pMidiInfo->note);
             if(assignedVoice>=0)
                dco_releaseVoice(assignedVoice);
        }
    }
}

void midi_voiceFinishedEvent(int voice) // adsr freed a voice, simulate key released for the key using this voice
{
    int i;
    for(i=0; i<KEYS_PRESSED_LEN; i++)
    {
      if(keysPressed[i].flagFree==0)
      {
          if(keysPressed[i].voice == voice)
          {
              keysPressed[i].flagFree=1;
          }
      }
    }
}

void midi_clearAllKeysPressed(void)
{
  byte i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==0)
    {
        keysPressed[i].flagFree=1;
    }
  } 

  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
      dco_releaseAllVoices();
  }
}

/// Keys management
static int saveKey(int note, int voice)
{
  int i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==1)
    {
        keysPressed[i].flagFree=0;
        keysPressed[i].note = note;
        keysPressed[i].voice = voice;
        return 0;
    }
  }
  return -1; // no more space
}
static int getIndexOfPressedKey(int note)
{
  int i;
  for(i=0; i<KEYS_PRESSED_LEN; i++)
  {
    if(keysPressed[i].flagFree==0)
    {
        if(keysPressed[i].note == note)
          return i;
    }
  }
  return -1; 
}
static int deleteKey(int note)
{
    int index = getIndexOfPressedKey(note);
    if(index>=0 && index<KEYS_PRESSED_LEN)
    {
      keysPressed[index].flagFree=1;
      return keysPressed[index].voice;
    }
    return -1;
}

