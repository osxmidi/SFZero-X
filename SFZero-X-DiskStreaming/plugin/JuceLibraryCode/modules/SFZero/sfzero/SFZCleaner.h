/*
  ==============================================================================

    SFZCleaner.h
    Created: 4 Dec 2017 10:22:24pm
    Author:  malcolm

  ==============================================================================
*/

#ifndef SFZCLEANER_H_INCLUDED
#define SFZCLEANER_H_INCLUDED

#include "SFZDiskStreamer.h"

namespace sfzero
{
  class SFZCleaner    : public juce::Thread
  {
    public:
      SFZCleaner(const juce::String& threadName);
      ~SFZCleaner();
      void run() override;
      void addThread(SFZDiskStreamer* thread);
    private:
      juce::Array<sfzero::SFZDiskStreamer*, juce::CriticalSection> buffers;
      void checkForBuffersToFree();
      int lastCount;
      JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SFZCleaner)
  };
}

#endif // SFZCLEANER_H_INCLUDED
