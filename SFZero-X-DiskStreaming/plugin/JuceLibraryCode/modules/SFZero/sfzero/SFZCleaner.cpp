/*
  ==============================================================================

    SFZCleaner.cpp
    Created: 4 Dec 2017 10:22:24pm
    Author:  malcolm

  ==============================================================================
*/

#include "SFZCleaner.h"

//==============================================================================
sfzero::SFZCleaner::SFZCleaner(const juce::String& threadName) : juce::Thread(threadName)
{
  lastCount=-1;
}

sfzero::SFZCleaner::~SFZCleaner()
{
  stopThread(3000);
  checkForBuffersToFree();
}

void sfzero::SFZCleaner::run(){
  while (!threadShouldExit())
  {
    if(lastCount!=buffers.size()){
    //  std::cout << "cleaning " << buffers.size() << "\n";
      lastCount=buffers.size();
    }
    checkForBuffersToFree();
    wait (1000);
  }
}

void sfzero::SFZCleaner::checkForBuffersToFree()
{
  for (int i = buffers.size(); --i >= 0;)
  {
    sfzero::SFZDiskStreamer* buffer = buffers[i];
    if (!buffer->isThreadRunning()){
      delete buffer;
      buffers.remove(i);
      //std::cout << "Buffer cleaned " << buffers.size() << "\n";
    }
  }
}

void sfzero::SFZCleaner::addThread(SFZDiskStreamer* thread){
  buffers.add(thread);
}
