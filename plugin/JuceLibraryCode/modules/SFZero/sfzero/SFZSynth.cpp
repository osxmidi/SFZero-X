/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#include "SFZSynth.h"
#include "SFZSound.h"
#include "SFZVoice.h"

sfzero::Synth::Synth() : Synthesiser() {
 
for(int i=0;i<128;i++)
  {
  ccvalhandle[i] = 0;
  swupdownarray[i] = 0;
  }

  lastkeyval = 10000; 
  prevkeyval = 10000;
  prevvelval = 0.0;

  polyvalhandle[0] = 0;
  polyvalhandle[1] = 0;
  chanvalhandle = 0;

  setccinit = 0;
  
  voicert = nullptr;
  midiNoteNumberrt = 10000;

    /*
for(int i=0; i<16; i++)
    midiVolume_[i] = 127;
    */
}

void sfzero::Synth::triggernote(int midiChannel2, int controllerNumber, int controllerValue)
{
  int i;

  const juce::ScopedLock locker(lock);

  // First, stop any currently-playing sounds in the group.
  //*** Currently, this only pays attention to the first matching region.
  int group = 0;
 // sfzero::Sound *sound = dynamic_cast<sfzero::Sound *>(getSound(0))
 
  bool allowTailOff = false;
  int midiChannel = 1;
  int midiNoteNumber = 0;
  float velocity = 1.0;
  
  int midiVelocity = static_cast<int>(velocity * 127);
 
  sfzero::Sound *sound;
  juce::SynthesiserSound * gh = getSound(0);
  sound = (sfzero::Sound *)gh;
  
  if (sound)
  {  
      if(setccinit == 0)
      {
      for(int i2=0;i2<128;i2++)
      {
      ccvalhandle[i2] = sound->setcc[i2];
      setccinit = 1;
      }
      }

      float randomval = (float) rand() / (float) RAND_MAX;
      int numRegions = sound->getNumRegions();
      for (i = 0; i < numRegions; ++i)
      {
      sfzero::Region *region = sound->regionAt(i);
      if(region != nullptr)
      {
      if (region->matches4(ccvalhandle[region->ccnumlotrig], ccvalhandle[region->ccnumhitrig], randomval, midiChannel))
      { 
      for (int idx = voices.size(); --idx >= 0;)
      {
      sfzero::Voice *voice2 = dynamic_cast<sfzero::Voice *>(voices.getUnchecked(idx));
      if (voice2 == nullptr)
      {
        continue;
      }
      }      
      
      sfzero::Voice *voice = dynamic_cast<sfzero::Voice *>(findFreeVoice(sound, midiNoteNumber, midiChannel, false));
              
        if (voice)
        {
        Synthesiser::noteOff(midiChannel, midiNoteNumber, velocity, allowTailOff);
        voice->setRegion(region);             
        startVoice(voice, sound, midiChannel, midiNoteNumber, velocity);
        }
       }
      }
     }
    }
}


void sfzero::Synth::triggernote2()
{
  int i;

  const juce::ScopedLock locker(lock);

  // First, stop any currently-playing sounds in the group.
  //*** Currently, this only pays attention to the first matching region.
  int group = 0;
 // sfzero::Sound *sound = dynamic_cast<sfzero::Sound *>(getSound(0))
 
  bool allowTailOff = false;
  int midiChannel = 1;
  int midiNoteNumber = 0;
  float velocity = 1.0;
   
  for (i = voices.size(); --i >= 0;)
  {
  sfzero::Voice *voice = dynamic_cast<sfzero::Voice *>(voices.getUnchecked(i));
    
  if(voice)
  { 
  sfzero::Sound *sound;
  juce::SynthesiserSound * gh = voice->getCurrentlyPlayingSound ();
  sound = (sfzero::Sound *)gh;
  
  if (sound)
  {    
  if(voice->isPlayingButReleased()) 
  { 
  int midich = 16;
  int midch;

  for(midch=1;midch<=16;midch++)
  {
  if (voice->isPlayingChannel(midch))
  midich = midch;
  }
     
  float vel = voice->curVelocity_ / 127.0;
  int currentnote = voice->getCurrentlyPlayingNote();  
     
  noteOn2(midich, currentnote, vel, voice);
  }
  }
  }
  }
}

void sfzero::Synth::noteOn(int midiChannel, int midiNoteNumber, float velocity)
{
  int i;
  float velocity2 = velocity;

  const juce::ScopedLock locker(lock);
  
  voicert = nullptr;
  midiNoteNumberrt = 10000;

  int midiVelocity = static_cast<int>(velocity * 127);

  // First, stop any currently-playing sounds in the group.
  //*** Currently, this only pays attention to the first matching region.
  int group = 0;
 // sfzero::Sound *sound = dynamic_cast<sfzero::Sound *>(getSound(0));
  
  sfzero::Sound *sound;
  juce::SynthesiserSound * gh = getSound(0);
  sound = (sfzero::Sound *)gh;
  
  // Are any notes playing?  (Needed for first/legato trigger handling.)
  // Also stop any voices still playing this note.
  bool anyNotesPlaying = false;
  for (i = voices.size(); --i >= 0;)
  {
    sfzero::Voice *voice = dynamic_cast<sfzero::Voice *>(voices.getUnchecked(i));
    if (voice == nullptr)
    {
      continue;
    }
    if (voice->isPlayingChannel(midiChannel))
    {
      if (voice->isPlayingNoteDown())
      {
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
        {
          if (!voice->isPlayingOneShot())
          {
            voice->stopNoteQuick();         
          }
        }
        else
        {
          anyNotesPlaying = true;
        }
      }
    }
  }

  // Play *all* matching regions.
  sfzero::Region::Trigger trigger = (anyNotesPlaying ? sfzero::Region::legato : sfzero::Region::first);
  if (sound)
  {
    swupdownarray[midiNoteNumber] = 1;   
   
    if(setccinit == 0)
    {
    for(int i2=0;i2<128;i2++)
    {
    ccvalhandle[i2] = sound->setcc[i2];
    setccinit = 1;
    }
    }

    float randomval = (float) rand() / (float) RAND_MAX;
    int numRegions = sound->getNumRegions();
    for (i = 0; i < numRegions; ++i)
    {
      sfzero::Region *region = sound->regionAt(i);
      if(region != nullptr)
      {
      if(region->swlaston == 1)
      {
      if(region->sw_last == midiNoteNumber)
      {
      lastkeyval = region->sw_last;
      }
      }
      
      if(region->swprevon == 1)
      {
      if(region->sw_previous == midiNoteNumber)
      {
      prevkeyval = region->sw_previous;
      }
      }      
            
      for(int idx4=0;idx4<128;idx4++)
      {     
      region->swupdownarray[idx4] = swupdownarray[idx4];
      }       
       
      if (region->matches2(midiNoteNumber, midiVelocity, trigger, randomval, ccvalhandle[region->ccnumlo], ccvalhandle[region->ccnumhi], polyvalhandle[0], chanvalhandle, midiChannel, &lastkeyval, &prevkeyval))    
      {
      group = region->group;    
      if (group != 0)
      {       
      for (int idx = voices.size(); --idx >= 0;)
      {
      sfzero::Voice *voice2 = dynamic_cast<sfzero::Voice *>(voices.getUnchecked(idx));
      if (voice2 == nullptr)
      {
        continue;
      }
      if (voice2->getOffBy() == group)
      {
        voice2->stopNoteForGroup();
      }
      }
      }
        sfzero::Voice *voice =
            dynamic_cast<sfzero::Voice *>(findFreeVoice(sound, midiNoteNumber, midiChannel, isNoteStealingEnabled()));
        if (voice)
        {
          voicert = voice;
          midiNoteNumberrt = midiNoteNumber;
        
          for(int idx4=0;idx4<128;idx4++)
          region->ccvalmoved[idx4] = ccvalhandle[idx4];
          region->polyvalmoved[0] = polyvalhandle[0];
          region->polyvalmoved[1] = polyvalhandle[1];
          region->chanvalmoved = chanvalhandle; 
          voice->setRegion(region);
          // voice->setMidiVolume(midiVolume_[midiChannel-1]);
         
          if(region->swvelon == 1)
          {
          if(region->sw_vel == 1)
          velocity2 = prevvelval;           
          }         
         
          startVoice(voice, sound, midiChannel, midiNoteNumber, velocity2);       
        }
      }
    }
   } 
  }
  prevkeyval = midiNoteNumber;
  prevvelval = velocity; 

  noteVelocities_[midiNoteNumber] = midiVelocity;
}

void sfzero::Synth::noteOn2(int midiChannel, int midiNoteNumber, float velocity, sfzero::Voice *voice3)
{
  int i;
  float velocity2 = velocity;

  const juce::ScopedLock locker(lock);
  
  voicert = nullptr;
  midiNoteNumberrt = 10000;

  int midiVelocity = static_cast<int>(velocity * 127);

  // First, stop any currently-playing sounds in the group.
  //*** Currently, this only pays attention to the first matching region.
  int group = 0;
 // sfzero::Sound *sound = dynamic_cast<sfzero::Sound *>(getSound(0));
  
  sfzero::Sound *sound;
  juce::SynthesiserSound * gh = getSound(0);
  sound = (sfzero::Sound *)gh;
  
  // Are any notes playing?  (Needed for first/legato trigger handling.)
  // Also stop any voices still playing this note.
  bool anyNotesPlaying = false;
  for (i = voices.size(); --i >= 0;)
  {
    sfzero::Voice *voice = dynamic_cast<sfzero::Voice *>(voices.getUnchecked(i));
    if (voice == nullptr)
    {
      continue;
    }
    if (voice->isPlayingChannel(midiChannel))
    {
      if (voice->isPlayingNoteDown())
      {
        if (voice->getCurrentlyPlayingNote() == midiNoteNumber)
        {
          if (!voice->isPlayingOneShot())
          {
            voice->stopNoteQuick();         
          }
        }
        else
        {
          anyNotesPlaying = true;
        }
      }
    }
  }

  // Play *all* matching regions.
  sfzero::Region::Trigger trigger = (anyNotesPlaying ? sfzero::Region::legato : sfzero::Region::first);
  if (sound)
  {
    swupdownarray[midiNoteNumber] = 1;   
   
    if(setccinit == 0)
    {
    for(int i2=0;i2<128;i2++)
    {
    ccvalhandle[i2] = sound->setcc[i2];
    setccinit = 1;
    }
    }

    float randomval = (float) rand() / (float) RAND_MAX;
    int numRegions = sound->getNumRegions();
    for (i = 0; i < numRegions; ++i)
    {
      sfzero::Region *region = sound->regionAt(i);
      if(region != nullptr)
      {
      if(region->swlaston == 1)
      {
      if(region->sw_last == midiNoteNumber)
      {
      lastkeyval = region->sw_last;
      }
      }
      
      if(region->swprevon == 1)
      {
      if(region->sw_previous == midiNoteNumber)
      {
      prevkeyval = region->sw_previous;
      }
      }      
            
      for(int idx4=0;idx4<128;idx4++)
      {     
      region->swupdownarray[idx4] = swupdownarray[idx4];
      }       
       
      if (region->matches2(midiNoteNumber, midiVelocity, trigger, randomval, ccvalhandle[region->ccnumlo], ccvalhandle[region->ccnumhi], polyvalhandle[0], chanvalhandle, midiChannel, &lastkeyval, &prevkeyval))    
      {
      group = region->group;    
      if (group != 0)
      {       
      for (int idx = voices.size(); --idx >= 0;)
      {
      sfzero::Voice *voice2 = dynamic_cast<sfzero::Voice *>(voices.getUnchecked(idx));
      if (voice2 == nullptr)
      {
        continue;
      }
      if (voice2->getOffBy() == group)
      {
        voice2->stopNoteForGroup();
      }
      }
      }
        sfzero::Voice *voice =
            dynamic_cast<sfzero::Voice *>(findFreeVoice(sound, midiNoteNumber, midiChannel, isNoteStealingEnabled()));
        if (voice)
        {
          voicert = voice;
          midiNoteNumberrt = midiNoteNumber;
        
          for(int idx4=0;idx4<128;idx4++)
          region->ccvalmoved[idx4] = ccvalhandle[idx4];
          region->polyvalmoved[0] = polyvalhandle[0];
          region->polyvalmoved[1] = polyvalhandle[1];
          region->chanvalmoved = chanvalhandle; 
          voice->setRegion(region);
          // voice->setMidiVolume(midiVolume_[midiChannel-1]);
         
          if(region->swvelon == 1)
          {
          if(region->sw_vel == 1)
          velocity2 = prevvelval;           
          }   
          
          voice->sourceSamplePositionupdate_ = 1;
          
          // SamplePosition;   
 
          if(voice3)
          {         
          voice->sourceSamplePositionupdate_ = 1;
          voice->sourceSamplePosition2 = voice3->sourceSamplePosition_;
          voice->startedlate = 1;
                   
 		  voice->ampegGain2 = voice3->ampegGain3;
          voice->ampegSlope2 = voice3->ampegSlope3;
          voice->samplesUntilNextAmpSegment2 = voice3->samplesUntilNextAmpSegment3;
          voice->ampSegmentIsExponential2 = voice3->ampSegmentIsExponential3;  
  
          voice->pitchegGain2 = voice3->pitchegGain3;
          voice->pitchegSlope2 = voice3->pitchegSlope3;
          voice->samplesUntilNextAmpSegmentpitch2 = voice3->samplesUntilNextAmpSegmentpitch3;
          voice->ampSegmentIsExponentialpitch2 = voice3->ampSegmentIsExponentialpitch3;
  
          voice->filteregGain2 = voice3->filteregGain3;
          voice->filteregSlope2 = voice3->filteregSlope3;
          voice->samplesUntilNextAmpSegmentfilter2 = voice3->samplesUntilNextAmpSegmentfilter3;
          voice->ampSegmentIsExponentialfilter2 = voice3->ampSegmentIsExponentialfilter3;          
          }
         
          startVoice(voice, sound, midiChannel, midiNoteNumber, velocity2);       
        }
      }
    }
   } 
  }
  prevkeyval = midiNoteNumber;
  prevvelval = velocity; 

  noteVelocities_[midiNoteNumber] = midiVelocity;
}



void sfzero::Synth::noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff)
{
  int i;
  int scount;
  
  const juce::ScopedLock locker(lock);
  
  scount = 0;
  if(voicert && (midiNoteNumberrt == midiNoteNumber))
  scount = voicert->samplecount;
  voicert = nullptr;
  
  Synthesiser::noteOff(midiChannel, midiNoteNumber, velocity, allowTailOff);

  // Start release region.
  // sfzero::Sound *sound = dynamic_cast<sfzero::Sound *>(getSound(0));
  
  sfzero::Sound *sound;
  juce::SynthesiserSound * gh = getSound(0);
  sound = (sfzero::Sound *)gh;
   
  sfzero::Region::Trigger trigger = sfzero::Region::release;
  if (sound)
  {
    float randomval = (float) rand() / (float) RAND_MAX;
    int numRegions = sound->getNumRegions();
    for (i = 0; i < numRegions; ++i)
    {
      sfzero::Region *region = sound->regionAt(i);
      if(region != nullptr)
      {
      
      if(region->swlaston == 1)
      {
      if(region->sw_last == midiNoteNumber)
      {
      lastkeyval = region->sw_last;
      }
      }
      
      if(region->swprevon == 1)
      {
      if(region->sw_previous == midiNoteNumber)
      {
      prevkeyval = region->sw_previous;
      }
      }            
      
      for(int idx4=0;idx4<128;idx4++)
      {     
      region->swupdownarray[idx4] = swupdownarray[idx4];
      }                    
       
      if (region->matches3(midiNoteNumber, noteVelocities_[midiNoteNumber], trigger, randomval, ccvalhandle[region->ccnumlo], ccvalhandle[region->ccnumhi], polyvalhandle[0], chanvalhandle, midiChannel, &lastkeyval, &prevkeyval))
      {  
      sfzero::Voice *voice = dynamic_cast<sfzero::Voice *>(findFreeVoice(sound, midiNoteNumber, midiChannel, false));
      if (voice)
      { 
        region->scount = 0;
        if(region->rt_decayon == 1) 
        region->scount = scount; 
        // Synthesiser is too locked-down (ivars are private rt protected), so
        // we have to use a "setRegion()" mechanism.
        for(int idx4=0;idx4<128;idx4++)
        region->ccvalmoved[idx4] = ccvalhandle[idx4];
        region->polyvalmoved[0] = polyvalhandle[0];
        region->polyvalmoved[1] = polyvalhandle[1];
        region->chanvalmoved = chanvalhandle; 
        voice->setRegion(region);
        startVoice(voice, sound, midiChannel, midiNoteNumber, noteVelocities_[midiNoteNumber] / 127.0f);
      }
    }
   }
  }
 }
   swupdownarray[midiNoteNumber] = 0; 
}

void sfzero::Synth::handleController (int midiChannel, int controllerNumber, int controllerValue)
{

  /*
  switch(controllerNumber){
    case 7:
      midiVolume_[midiChannel-1] = controllerValue;
      break;
  }
  */
    
  ccvalhandle[controllerNumber] = controllerValue;
  
  Synthesiser::handleController (midiChannel, controllerNumber, controllerValue);
  
  if((ccvalhandle[64] > 0) || (ccvalhandle[66] > 0)) 
  {
  triggernote2();  
  }
  
  triggernote(midiChannel, controllerNumber, controllerValue);  
}

void sfzero::Synth::handleAftertouch (int midiChannel, int midiNoteNumber, int aftertouchValue)
{
  /*
  switch(controllerNumber){
    case 7:
      midiVolume_[midiChannel-1] = controllerValue;
      break;
  }
  */
   
  polyvalhandle[0] = aftertouchValue;
  polyvalhandle[1] = midiNoteNumber;
  
  Synthesiser::handleAftertouch (midiChannel, midiNoteNumber, aftertouchValue);
}

void sfzero::Synth::handleChannelPressure (int midiChannel, int channelPressureValue)
{
  /*
  switch(controllerNumber){
    case 7:
      midiVolume_[midiChannel-1] = controllerValue;
      break;
  }
  */
    
  chanvalhandle = channelPressureValue;
  
  Synthesiser::handleChannelPressure (midiChannel, channelPressureValue);
}


int sfzero::Synth::numVoicesUsed()
{
  int numUsed = 0;

  for (int i = voices.size(); --i >= 0;)
  {
    if (voices.getUnchecked(i)->getCurrentlyPlayingNote() >= 0)
    {
      numUsed += 1;
    }
  }
  return numUsed;
}

juce::String sfzero::Synth::voiceInfoString()
{
  enum
  {
    maxShownVoices = 20,
  };

  juce::StringArray lines;
  int numUsed = 0, numShown = 0;
  for (int i = voices.size(); --i >= 0;)
  {
    sfzero::Voice *voice = dynamic_cast<sfzero::Voice *>(voices.getUnchecked(i));
    if (voice->getCurrentlyPlayingNote() < 0)
    {
      continue;
    }
    numUsed += 1;
    if (numShown >= maxShownVoices)
    {
      continue;
    }
    lines.add(voice->infoString());
  }
  lines.insert(0, "voices used: " + juce::String(numUsed));
  return lines.joinIntoString("\n");
}
