/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZSYNTH_H_INCLUDED
#define SFZSYNTH_H_INCLUDED

#include "SFZCommon.h"
#include "SFZVoice.h"

namespace sfzero
{

class Synth : public juce::Synthesiser
{
public:
  Synth();
  virtual ~Synth() {}

  void noteOn(int midiChannel, int midiNoteNumber, float velocity) override;
  void noteOn2(int midiChannel, int midiNoteNumber, float velocity, sfzero::Voice *voice);
  void noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff) override;
  
  void triggernote(int midiChannel, int controllerNumber, int controllerValue);
  void triggernote2();
  
  void handleController (int midiChannel, int controllerNumber, int controllerValue) override;

  void handleAftertouch (int midiChannel, int midiNoteNumber, int aftertouchValue) override;
 
  void handleChannelPressure (int midiChannel, int channelPressureValue) override;

  int numVoicesUsed();
  juce::String voiceInfoString();

  int noteVelocities_[128];

private:
 
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Synth)
  // int midiVolume_[16];
  public:
  int ccvalhandle[128];
  int polyvalhandle[2];
  int chanvalhandle;
  int setccinit;
  
  int swupdownarray[128];
  int lastkeyval;
  int prevkeyval;
  float prevvelval;  
  
  Voice *voicert;
  int midiNoteNumberrt;
};
}

#endif // SFZSYNTH_H_INCLUDED
