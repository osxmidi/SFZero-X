/*************************************************************************************
* Original code copyright (C) 2012 Steve Folta
* Converted to Juce module (C) 2016 Leo Olivers
* Forked from https://github.com/stevefolta/SFZero
* For license info please see the LICENSE file distributed with this source code
*************************************************************************************/
#ifndef SFZDISKSTREAMER_H_INCLUDED
#define SFZDISKSTREAMER_H_INCLUDED

namespace sfzero
{

  class SFZDiskStreamer : public juce::Thread
  {
    public:
      SFZDiskStreamer(const juce::String& threadName, juce::File file, juce::AudioFormatManager *formatManager, int numChannels, juce::uint32 numSamples, int streamBufferSize);
      virtual ~SFZDiskStreamer();
      void run() override;
      juce::AudioSampleBuffer* GetVoiceBuffer(){return buffer_;}
      void copyBuffer(juce::AudioSampleBuffer *buffer);
      void setCurrentSample(double pos, int blockSize);
      juce::uint32 getNumSamplesFilled(){return(numSamplesFilled);}
    private:
      juce::File file_;
      juce::AudioSampleBuffer *buffer_;
      juce::AudioFormatManager *formatManager_;
      juce::uint32 numSamplesFilled;
      juce::AudioFormatReader *reader_;
      int blockSize_;
  };
}

#endif // SFZDISKSTREAMER_H_INCLUDED
