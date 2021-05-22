/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZSAMPLE_H_INCLUDED
#define SFZSAMPLE_H_INCLUDED

#include "SFZCommon.h"

namespace sfzero
{

class Sample
{
public:
  #ifdef SFZEROBUF
    Sample(const juce::File &fileIn) : file_(fileIn), buffer_(nullptr), preBufferSize_(4), sampleRate_(0), sampleLength_(0), loopStart_(0), loopEnd_(0), doStream_(false){
  char* buffersizeenv;

  if((buffersizeenv = getenv("SFZEROBUF")) == NULL)
  {
   preBufferSize_ = 22;
  }
  else
  preBufferSize_ = atoi(buffersizeenv);

  if((preBufferSize_ < 1) || (preBufferSize_ > 256))
  preBufferSize_ = 22;
  
  preBufferSize_ *= 1024;

  // printf("Buffer Size %d\n", preBufferSize_ / 1024);
 }
 #else
  Sample(const juce::File &fileIn) : file_(fileIn), buffer_(nullptr), sampleRate_(0), sampleLength_(0), loopStart_(0), loopEnd_(0), doStream_(false){}
 #endif
  Sample(double sampleRateIn) : buffer_(nullptr), sampleRate_(sampleRateIn), sampleLength_(0), loopStart_(0), loopEnd_(0), doStream_(false) {}
  virtual ~Sample();

  bool load(juce::AudioFormatManager *formatManager);

  juce::File getFile() { return (file_); }
  juce::AudioSampleBuffer *getBuffer() { return (buffer_); }
  double getSampleRate() { return (sampleRate_); }
  #ifdef SFZEROBUF
  int getpreBufferSize() { return (preBufferSize_); }  
  #endif
  juce::String getShortName();
  void setBuffer(juce::AudioSampleBuffer *newBuffer);
  juce::AudioSampleBuffer *detachBuffer();
  juce::String dump();
  juce::uint64 getSampleLength() const { return sampleLength_; }
  juce::uint64 getLoopStart() const { return loopStart_; }
  juce::uint64 getLoopEnd() const { return loopEnd_; }
  bool CanStream() const { return doStream_; }
  #ifndef SFZEROBUF
  static const int preBufferSize=44100;
  #endif
#ifdef JUCE_DEBUG
  void checkIfZeroed(const char *where);

#endif

private:
  juce::File file_;
  juce::AudioSampleBuffer *buffer_;
  double sampleRate_;
  #ifdef SFZEROBUF
  int preBufferSize_; 
  #endif
  juce::uint64 sampleLength_, loopStart_, loopEnd_;
  bool doStream_;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Sample)
};
}

#endif // SFZSAMPLE_H_INCLUDED
