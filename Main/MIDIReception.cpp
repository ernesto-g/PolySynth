#include <Arduino.h>
#include <HardwareSerial.h>
#include "MIDIReception.h"
#include "MIDIManager.h"
//#include "SequencerManager.h"

// USB MIDI lib
//#include "MIDIUSB/MIDIUSB.h"
//#include "MIDIUSB/MIDIUSB.cpp"
//_____________

#define MIDI_BUFFER_LEN   32
#define FROM_EXTERNAL_KEYBOARD  1

// Private functions
static void processMidiPacket(unsigned char* pData, int len, int fromKeyboard,MidiInfo* pMidiInfo);

// Private variables
static unsigned char bufferMidiExternalKeyboard[MIDI_BUFFER_LEN];
static unsigned int indexBufferExternal;

void midircv_sysTick(void)
{
  
}

void midircv_init(void)
{
  // External keyboard
  Serial2.begin(31250);
  Serial2.setTimeout(0);
  indexBufferExternal=0;
}

void midircv_stateMachine(void)
{
  unsigned char data;
  int c;

  c = Serial2.readBytes(&data,1); // 12uS
  if(c>0)
  {
    bufferMidiExternalKeyboard[indexBufferExternal] = data;
    indexBufferExternal++;
    if(indexBufferExternal==3)
    {
      MidiInfo midiInfo;
      processMidiPacket(bufferMidiExternalKeyboard,indexBufferExternal,FROM_EXTERNAL_KEYBOARD,&midiInfo);
      midi_analizeMidiInfo(&midiInfo);      
      indexBufferExternal=0;
    }
  }
}


static void processMidiPacket(unsigned char* pData, int len, int fromKeyboard,MidiInfo* pMidiInfo)
{
  // 1st byte
  pMidiInfo->channel = pData[0] & B00001111;
  pMidiInfo->cmd = pData[0] & B11110000;

  // 2nd byte
  pMidiInfo->note = pData[1];
  
  // 3rd byte
  pMidiInfo->vel = pData[2];


  // send to MIDI OUT
  Serial2.write(pData,len);
  //_________________        


  // Send by USB MIDI
  //midiEventPacket_t packet = {(pMidiInfo->cmd>>4), pMidiInfo->cmd | pMidiInfo->channel, pMidiInfo->note, pMidiInfo->vel};
  //MidiUSB.sendMIDI(packet);
  //MidiUSB.flush();
  //_________________

  // Send to sequencer manager
  //seq_keyEvent(pMidiInfo);
  //__________________________      
}



