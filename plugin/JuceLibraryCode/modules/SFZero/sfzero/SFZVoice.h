/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZVOICE_H_INCLUDED
#define SFZVOICE_H_INCLUDED

#include "SFZEG.h"

namespace sfzero
{
struct Region;

class Voice : public juce::SynthesiserVoice
{
public:
  Voice();
  virtual ~Voice();

  bool canPlaySound(juce::SynthesiserSound *sound) override;
  void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound *sound, int currentPitchWheelPosition) override;
  void stopNote(float velocity, bool allowTailOff) override;
  void stopNoteForGroup();
  void stopNoteQuick();
  void pitchWheelMoved(int newValue) override;
  void controllerMoved(int controllerNumber, int newValue) override;
  void aftertouchChanged(int newAftertouchValue) override;
  void channelPressureChanged(int newChannelPressureValue) override;
  void renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample, int numSamples) override;
  bool isPlayingNoteDown();
  bool isPlayingOneShot();
  // void setMidiVolume(int volume);

  int getGroup();
  juce::uint64 getOffBy();

  // Set the region to be used by the next startNote().
  void setRegion(Region *nextRegion);

  juce::String infoString();
  
public:

  float fadeout(int val);
  float fadein(int val); 
  float fadeinvel();
  float fadeoutvel(); 
  
  juce::dsp::IIR::Filter<float> FilterL, FilterR;
  juce::dsp::IIR::Coefficients<float> BandStop(float sr, float freq, float qval);  
  void startfilter(float cutoff, int reset); 
  int filterinit;
  float cutoffadj;
  
  void startlfopitch(float freq, float sr);
  void startlfotrem(float freq, float sr);
  void startlfofilter(float freq, float sr);
  
  void lfopitchcc(float freq, float sr);
  void lfotremcc(float freq, float sr);
  void lfofiltercc(float freq, float sr);
    
  void processlfopitch(int blocknum);
  void processlfotrem(int blocknum);
  void processlfofilter(int numlfo);
   
  float lfopitchlevel, lfopitchdelta;
  float lfotremlevel, lfotremdelta;  
  float lfofilterlevel, lfofilterdelta;   
  float pitchlevel;
  float tremlevel;
  float filterlevel;
  
  float fadepitchdelta;
  float fadepitchcount;
  int fadepitchdone;
  float pitchlfodelay;
  float fadeinpitchval;
  
  float fadetremdelta;
  float fadetremcount;
  int fadetremdone;
  float tremlfodelay;
  float fadeintremval;  
  
  float fadefilterdelta;
  float fadefiltercount;
  int fadefilterdone;
  float filterlfodelay;
  float fadeinfilterval;  
 
  float pitchnum, pitchden;
  
  int blocklfopitch;
  int blockresetpitch;
  int blocklfotrem;
  int blockresettrem;  
  int blocklfofilter;
  int blockresetfilter;
  
  float tremgain;
  
  int samplecount;
  int cutoffhold;
  
  static inline float timecents2Secs(float timecents);
   
  float curvel;

  int sustainoff;
  int loopdone;

  int curVelocity_;

  double sourceSamplePosition_;

  double sourceSamplePosition2;

  int sourceSamplePositionupdate_;

  int startedlate;

  float ampegGain2;
  float ampegSlope2;
  int samplesUntilNextAmpSegment2;
  bool ampSegmentIsExponential2;

  float pitchegGain2;
  float pitchegSlope2;
  int samplesUntilNextAmpSegmentpitch2;
  bool ampSegmentIsExponentialpitch2;
   
  float filteregGain2;
  float filteregSlope2;
  int samplesUntilNextAmpSegmentfilter2;
  bool ampSegmentIsExponentialfilter2; 

  float ampegGain3;
  float ampegSlope3;
  int samplesUntilNextAmpSegment3;
  bool ampSegmentIsExponential3;

  float pitchegGain3;
  float pitchegSlope3;
  int samplesUntilNextAmpSegmentpitch3;
  bool ampSegmentIsExponentialpitch3;
   
  float filteregGain3;
  float filteregSlope3;
  int samplesUntilNextAmpSegmentfilter3;
  bool ampSegmentIsExponentialfilter3; 
  
private:
  Region *region_;
  int trigger_;
  int curMidiNote_, curPitchWheel_;
  double pitchRatio_;
  float noteGainLeft_, noteGainRight_;
  
  EG ampeg_, pitcheg_, filtereg_;
  juce::int64 sampleEnd_;
  juce::int64 loopStart_, loopEnd_;
//  int midiVolume_;

  // Info only.
  int numLoops_;
 

  void calcPitchRatio();
  void killNote();
  inline double fractionalMidiNoteInHz(double note, double freqOfA = 440.0);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Voice)
};
}

#endif // SFZVOICE_H_INCLUDED
