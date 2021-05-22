/*************************************************************************************
* Original code copyright (C) 2012 Steve Folta
* Converted to Juce module (C) 2016 Leo Olivers
* Forked from https://github.com/stevefolta/SFZero
* For license info please see the LICENSE file distributed with this source code
*************************************************************************************/
#include "SFZDiskStreamer.h"
#include <algorithm>

sfzero::SFZDiskStreamer::SFZDiskStreamer(const juce::String& threadName, juce::File file, juce::AudioFormatManager *formatManager, int numChannels, juce::uint32 numSamples, int streamBufferSize): juce::Thread(threadName)
{
  file_=file;
  buffer_ = new juce::AudioSampleBuffer(numChannels, numSamples);
  formatManager_=formatManager;
  reader_ = formatManager_->createReaderFor(file_);
  numSamplesFilled=0;
}

sfzero::SFZDiskStreamer::~SFZDiskStreamer() {
  delete buffer_;
  delete reader_;
}

void sfzero::SFZDiskStreamer::run(){
  //std::cout << "stream " << file_.getFileName() << "\n";
  int samplesToRead = std::min(blockSize_, (int)(buffer_->getNumSamples()-numSamplesFilled));
  reader_->read(buffer_, numSamplesFilled, samplesToRead, numSamplesFilled, true, true);
  numSamplesFilled+=samplesToRead;
}

void sfzero::SFZDiskStreamer::copyBuffer(juce::AudioSampleBuffer *inBuffer){
  for(int ichnl=0; ichnl<buffer_->getNumChannels(); ichnl++)
    buffer_->copyFrom(ichnl,
                 0, //int destStartSample,
                 *inBuffer, //const AudioBuffer& source,
                 ichnl, //int sourceChannel,
                 0, //int sourceStartSample,
                 inBuffer->getNumSamples());// int numSamples)
  numSamplesFilled+=inBuffer->getNumSamples();
}

void sfzero::SFZDiskStreamer::setCurrentSample(double pos, int blockSize){
  if(!isThreadRunning() && numSamplesFilled<=pos+blockSize/2){
    blockSize_=blockSize;
    startThread();
  }
}
