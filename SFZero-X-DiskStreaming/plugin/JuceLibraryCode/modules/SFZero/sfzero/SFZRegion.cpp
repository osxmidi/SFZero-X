/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#include "SFZRegion.h"
#include "SFZSample.h"

void sfzero::EGParameters::clear()
{
  delay = 0.0;
  start = 0.0;
  attack = 0.0;
  hold = 0.0;
  decay = 0.0;
  sustain = 100.0;
  release = 0.0;
}

void sfzero::EGParameters::clearMod()
{
  // Clear for velocity or other modification.
  delay = start = attack = hold = decay = sustain = release = 0.0;
}

sfzero::Region::Region() { clear(); }

void sfzero::Region::clear()
{
  memset(this, 0, sizeof(*this));
  
  hirandomval = 1.0; 
  lorandomval = 0.0;
  ccnumhival = 127; 
  ccnumlovaltrig = -1; 
  ccnumhivaltrig = -1;   
  ccgain = 0.0; 
  lochan = 1;
  hichan = 16;  
  xfin_lokey = 0;
  xfin_hikey = 0;  
  xfout_lokey = 127;
  xfout_hikey = 127;    
  ccnumfadeinlokey = 0;
  ccnumfadeinhikey = 0;
  ccnumfadeoutlokey = 127;
  ccnumfadeouthikey = 127;
  xfin_lovel = 0;
  xfin_hivel = 0;  
  xfout_lovel = 127;
  xfout_hivel = 127;    
  
  cutoff = 0.0;
  resonance = 0.0; 
  
  lfofreq = 0.0;
  lfodepth = 0.0;
  pitchlfofreq = 0.0;
  pitchlfodepth = 0.0;
  tremlfofreq = 0.0;
  tremlfodepth = 0.0;
  
  delaypitchval = 0.0;
  fadepitchval = 0.0;  
  delayampval = 0.0; 
  delayfilval = 0.0;   
  fadeampval = 0.0;
  fadefilval = 0.0;
  
  pitchegdepth = 0.0;
  filteregdepth = 0.0;
  
  pitchlfo_depthccval = 0.0;
  pitchlfo_freqccval = 0.0;  
  fillfo_depthccval = 0.0;
  fillfo_freqccval = 0.0;  
  amplfo_depthccval = 0.0;
  amplfo_freqccval = 0.0;
    
  ampeg_delaycc = 0.0;
  ampeg_startcc = 0.0;
  ampeg_attackcc = 0.0;
  ampeg_holdcc = 0.0;
  ampeg_decaycc = 0.0;
  ampeg_sustaincc = 100.0;
  ampeg_releasecc = 0.0;
  
  delayegorg = 0.0;
  startegorg = 0.0;
  attackegorg = 0.0;
  holdegorg = 0.0;
  decayegorg = 0.0;
  sustainegorg = 100.0;
  releaseegorg = 0.0;
  
  amp_random = 0.0;
  fil_random = 0.0;
  delay_random = 0.0;
  pitch_random = 0.0;

  delaycc = 0.0;
  
  rt_decay = 0.0;
  
  for(int i=0;i<128;i++)
  ccvalmoved[i] = 0;

  cutoffpoly = 0.0;
  pitchlfo_depthpolyval = 0.0;
  pitchlfo_freqpolyval = 0.0;
  fillfo_depthpolyval = 0.0;
  fillfo_freqpolyval = 0.0;
  amplfo_depthpolyval = 0.0;
  amplfo_freqpolyval = 0.0;
  lopolyaft = 0;
  hipolyaft = 127;

  cutoffchan = 0.0;
  pitchlfo_depthchanval = 0.0;
  pitchlfo_freqchanval = 0.0;
  fillfo_depthchanval = 0.0;
  fillfo_freqchanval = 0.0;
  amplfo_depthchanval = 0.0;
  amplfo_freqchanval = 0.0;
  lochanaft = 0;
  hichanaft = 127;

  troff = 0;
  
  hikey = 127;
  hivel = 127;
  pitch_keycenter = 60; // C4
  pitch_keytrack = 100;
  bend_up = 200;
  bend_down = -200;
  volume = pan = delaytime = 0.0;
  amp_veltrack = 100.0;
  pitch_veltrack = 100.0;
  fil_veltrack = 100.0;
  ampeg.clear();
  ampeg_veltrack.clearMod();
  pitcheg.clear();
  pitcheg_veltrack.clearMod();
  filtereg.clear();
  filtereg_veltrack.clearMod();

  filtereg.release = 1.0;
  
  sw_hikey = 127;
  sw_last = 10000;
  sw_down = 10000;
  sw_up = 10000;
  sw_previous = 10000;
  
  lobend = -8192;
  hibend = 8192;  
}

void sfzero::Region::clearForSF2()
{
  clear();
  pitch_keycenter = -1;
  loop_mode = no_loop;

  // SF2 defaults in timecents.
  ampeg.delay = -12000.0;
  ampeg.attack = -12000.0;
  ampeg.hold = -12000.0;
  ampeg.decay = -12000.0;
  ampeg.sustain = 0.0;
  ampeg.release = -12000.0;
  ampeg.depth = 0.0;
}

void sfzero::Region::clearForRelativeSF2()
{
  clear();
  pitch_keytrack = 0;
  amp_veltrack = 0.0;
  ampeg.sustain = 0.0;
}

void sfzero::Region::addForSF2(sfzero::Region *other)
{
  offset += other->offset;
  end += other->end;
  loop_start += other->loop_start;
  loop_end += other->loop_end;
  transpose += other->transpose;
  tune += other->tune;
  pitch_keytrack += other->pitch_keytrack;
  volume += other->volume;
  pan += other->pan;

  ampeg.delay += other->ampeg.delay;
  ampeg.attack += other->ampeg.attack;
  ampeg.hold += other->ampeg.hold;
  ampeg.decay += other->ampeg.decay;
  ampeg.sustain += other->ampeg.sustain;
  ampeg.release += other->ampeg.release;
}

void sfzero::Region::sf2ToSFZ()
{
  // EG times need to be converted from timecents to seconds.
  ampeg.delay = timecents2Secs(static_cast<int>(ampeg.delay));
  ampeg.attack = timecents2Secs(static_cast<int>(ampeg.attack));
  ampeg.hold = timecents2Secs(static_cast<int>(ampeg.hold));
  ampeg.decay = timecents2Secs(static_cast<int>(ampeg.decay));
  if (ampeg.sustain < 0.0f)
  {
    ampeg.sustain = 0.0f;
  }
  ampeg.sustain = 100.0f * juce::Decibels::decibelsToGain(-ampeg.sustain / 10.0f);
  ampeg.release = timecents2Secs(static_cast<int>(ampeg.release));

  // Pin very short EG segments.  Timecents don't get to zero, and our EG is
  // happier with zero values.
  if (ampeg.delay < 0.01f)
  {
    ampeg.delay = 0.0f;
  }
  if (ampeg.attack < 0.01f)
  {
    ampeg.attack = 0.0f;
  }
  if (ampeg.hold < 0.01f)
  {
    ampeg.hold = 0.0f;
  }
  if (ampeg.decay < 0.01f)
  {
    ampeg.decay = 0.0f;
  }
  if (ampeg.release < 0.01f)
  {
    ampeg.release = 0.0f;
  }
   
  // Pin values to their ranges.
  if (pan < -100.0f)
  {
    pan = -100.0f;
  }
  else if (pan > 100.0f)
  {
    pan = 100.0f;
  }
}

juce::String sfzero::Region::dump()
{
  juce::String info = juce::String::formatted("%d - %d, vel %d - %d", lokey, hikey, lovel, hivel);
  if (sample)
  {
    info << sample->getShortName();
  }
  info << "\n";
  return info;
}

float sfzero::Region::timecents2Secs(int timecents) { return static_cast<float>(pow(2.0, timecents / 1200.0)); }
