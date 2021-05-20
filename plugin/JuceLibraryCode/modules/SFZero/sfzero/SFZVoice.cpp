/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#include "SFZDebug.h"
#include "SFZRegion.h"
#include "SFZSample.h"
#include "SFZSound.h"
#include "SFZVoice.h"
#include <math.h>

static const float globalGain = -1.0;

sfzero::Voice::Voice()
    : region_(nullptr), trigger_(0), curMidiNote_(0), curPitchWheel_(0), pitchRatio_(0), noteGainLeft_(0), noteGainRight_(0), sourceSamplePosition_(0), sampleEnd_(0), loopStart_(0), loopEnd_(0), numLoops_(0), curVelocity_(0), filterinit(0), blocklfopitch(0), blocklfotrem(0), blocklfofilter(0), blockresetpitch(0), blockresettrem(0), blockresetfilter(0), cutoffadj(0), samplecount(0), cutoffhold(0), sourceSamplePositionupdate_(0), startedlate(0), ampegGain2(0.0f),ampegSlope2(0.0f), samplesUntilNextAmpSegment2(0), ampSegmentIsExponential2(false),
    pitchegGain2(0.0f),pitchegSlope2(0.0f), samplesUntilNextAmpSegmentpitch2(0), ampSegmentIsExponentialpitch2(false),
filteregGain2(0.0f),filteregSlope2(0.0f), samplesUntilNextAmpSegmentfilter2(0), ampSegmentIsExponentialfilter2(false),    
ampegGain3(0.0f),ampegSlope3(0.0f), samplesUntilNextAmpSegment3(0), ampSegmentIsExponential3(false),
    pitchegGain3(0.0f),pitchegSlope3(0.0f), samplesUntilNextAmpSegmentpitch3(0), ampSegmentIsExponentialpitch3(false),
filteregGain3(0.0f),filteregSlope3(0.0f), samplesUntilNextAmpSegmentfilter3(0), ampSegmentIsExponentialfilter3(false)    
        // , midiVolume_(127)
{        
  ampeg_.setExponentialDecay(true);
  pitcheg_.setExponentialDecay(true);
  filtereg_.setExponentialDecay(true);
  
  curvel = 127.0;
}

sfzero::Voice::~Voice() {}

bool sfzero::Voice::canPlaySound(juce::SynthesiserSound *sound) { return dynamic_cast<sfzero::Sound *>(sound) != nullptr; }

juce::dsp::IIR::Coefficients<float> sfzero::Voice::BandStop(float sr, float freq, float qval)
{	
	float w0 = (2.0f * M_PI * freq) / sr;
	float cw0 = cos (w0);
	float sw0 = sin (w0);
	float al = sw0 / (2 * qval);
	
	return juce::dsp::IIR::Coefficients<float> (1.0f, -2*cw0, 1.0f, 1.0f + al, -2.0f * cw0, 1.0f - al);
}

void sfzero::Voice::startlfopitch(float freq, float sr)
{
	lfopitchdelta = 2 * M_PI / (sr / freq);
	lfopitchlevel = 0.0;	
	pitchlevel = 0.0;
    fadepitchdelta = 0.0;
    fadepitchcount = 0.0;
    fadeinpitchval = 0.0;
    fadepitchdone = 0;
    pitchlfodelay = 0.0;
    if(region_->delaylfopitchon == 1)
    pitchlfodelay = region_->delaypitchval * getSampleRate();
}

void sfzero::Voice::lfopitchcc(float freq, float sr)
{
	lfopitchdelta = 2 * M_PI / (sr / freq);
}

void sfzero::Voice::startlfotrem(float freq, float sr)
{
	lfotremdelta = 2 * M_PI / (sr / freq);
	lfotremlevel = 0.0;	
    tremlevel = 0.0;
    fadetremdelta = 0.0;
    fadetremcount = 0.0;
    fadeintremval = 0.0;
    fadetremdone = 0;
    tremlfodelay = 0.0;
    if(region_->delaylfoampon == 1)
    tremlfodelay = region_->delayampval * getSampleRate();
}

void sfzero::Voice::lfotremcc(float freq, float sr)
{
	lfotremdelta = 2 * M_PI / (sr / freq);
}

void sfzero::Voice::startlfofilter(float freq, float sr)
{
	lfofilterdelta = 2 * M_PI / (sr / freq);
	lfofilterlevel = 0.0;	
	filterlevel = 0.0;
    fadefilterdelta = 0.0;
    fadefiltercount = 0.0;
    fadeinfilterval = 0.0;
    fadefilterdone = 0;
    filterlfodelay = 0.0;
    if(region_->delaylfofilon == 1)
    filterlfodelay = region_->delayfilval * getSampleRate();	
}

void sfzero::Voice::lfofiltercc(float freq, float sr)
{
	lfofilterdelta = 2 * M_PI / (sr / freq);
}

void sfzero::Voice::processlfopitch(int addnum)
{
    if(region_->delaylfopitchon == 1)
    {
	if(pitchlfodelay > 0.0)
	{
	pitchlfodelay -= addnum;
	return;
	}
	}
	  
    if((region_->fadelfopitchon == 1) && (fadepitchdone == 0))
    {       
    pitchlevel += lfopitchdelta * addnum;
    if (pitchlevel > 2*M_PI) 
    pitchlevel -= 2*M_PI;
    lfopitchlevel = sin(pitchlevel);
    
    if((fadepitchcount / getSampleRate()) >= 0.1)
    {    
    fadepitchcount = 0.0;
    fadepitchdelta += 0.1;
    
    if(fadepitchdelta <= region_->fadepitchval)
    {
 //   fadeinpitchval = fadepitchdelta / region_->fadepitchval;   
    fadeinpitchval = sin(0.5 * M_PI * (fadepitchdelta / region_->fadepitchval));
    }
    else
    fadepitchdone = 1;
    }
    lfopitchlevel *= fadeinpitchval;   
    }
    else
    {   
    pitchlevel += lfopitchdelta * addnum;
    if (pitchlevel > 2*M_PI) 
    pitchlevel -= 2*M_PI;
    lfopitchlevel = sin(pitchlevel);       
    }
}

void sfzero::Voice::processlfotrem(int addnum)
{
    if(region_->delaylfoampon == 1)
    {
	if(tremlfodelay > 0.0)
	{
	tremlfodelay -= addnum;
	return;
	}
	}

    if((region_->fadelfoampon == 1) && (fadetremdone == 0))
    {       
    tremlevel += lfotremdelta * addnum;
    if (tremlevel > 2*M_PI) 
    tremlevel -= 2*M_PI;
    lfotremlevel = sin(tremlevel);
    
    if((fadetremcount / getSampleRate()) >= 0.1)
    {    
    fadetremcount = 0.0;
    fadetremdelta += 0.1;
    
    if(fadetremdelta <= region_->fadeampval)
    {
 //   fadeintremval = fadetremdelta / region_->fadeampval;   
    fadeintremval = sin(0.5 * M_PI * (fadetremdelta / region_->fadeampval));
    }
    else
    fadetremdone = 1;
    }
    lfotremlevel *= fadeintremval;   
    }
    else
    {   
    tremlevel += lfotremdelta * addnum;
    if (tremlevel > 2*M_PI) 
    tremlevel -= 2*M_PI;
    lfotremlevel = sin(tremlevel);       
    }
}

void sfzero::Voice::processlfofilter(int addnum)
{ 
    if(region_->delaylfofilon == 1)
    {
	if(filterlfodelay > 0.0)
	{
	filterlfodelay -= addnum;
	return;
	}
	}
	
    if((region_->fadelfofilon == 1) && (fadefilterdone == 0))
    {       
    filterlevel += lfofilterdelta * addnum;
    if (filterlevel > 2*M_PI) 
    filterlevel -= 2*M_PI;
    lfofilterlevel = sin(filterlevel);
    
    if((fadefiltercount / getSampleRate()) >= 0.1)
    {    
    fadefiltercount = 0.0;
    fadefilterdelta += 0.1;
    
    if(fadefilterdelta <= region_->fadefilval)
    {
 //   fadeinfilterval = fadefilterdelta / region_->fadefilval;   
    fadeinfilterval = sin(0.5 * M_PI * (fadefilterdelta / region_->fadefilval));
    }
    else
    fadefilterdone = 1;
    }
    lfofilterlevel *= fadeinfilterval;   
    }
    else
    {   
    filterlevel += lfofilterdelta * addnum;
    if (filterlevel > 2*M_PI) 
    filterlevel -= 2*M_PI;
    lfofilterlevel = sin(filterlevel);       
    }	
}

void sfzero::Voice::startfilter(float cutoff, int reset)
{
float resonance = 0.0;
float resonanceDB = 0.0;
float qval = 0.0;

if(cutoffhold == 1)
return;

if(cutoff > getSampleRate() / 2)
return;

if(cutoff < 0.0)
return;

if((region_->cutoff > 0) && (region_->filteron == 1))
{
if(reset == 1)
{
FilterL.reset();
FilterR.reset();
// FilterL.snapToZero();
// FilterR.snapToZero();
}	
filterinit = 1;	

if(region_->resonance > 0.0)
{
resonanceDB = region_->resonance;
qval = powf(10.0, (resonanceDB * 0.05f));
qval = qval / sqrt(2);
}

if(region_->filter == sfzero::Region::lpf_1p)
{
  FilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(getSampleRate(), cutoff);
  
  FilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderLowPass(getSampleRate(), cutoff);
}
else if (region_->filter == sfzero::Region::hpf_1p)
{
  FilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(getSampleRate(), cutoff);
  
  FilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(getSampleRate(), cutoff);
}
else if(region_->filter == sfzero::Region::lpf_2p)
{
if(region_->resonance > 0.0)
{
  FilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), cutoff, qval);
  FilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), cutoff, qval); 
} 
else
{
  FilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), cutoff);
  FilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(getSampleRate(), cutoff);
}
}
else if (region_->filter == sfzero::Region::hpf_2p)
{
if(region_->resonance > 0.0)
{
  FilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), cutoff, qval);
  
  FilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), cutoff, qval);
}
else
{
  FilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), cutoff);
  
  FilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(getSampleRate(), cutoff);
}   
}
else if (region_->filter == sfzero::Region::bpf_2p)
{
if(region_->resonance > 0.0)
{
  FilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(getSampleRate(), cutoff, qval);
  
  FilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(getSampleRate(), cutoff, qval);
}
else
{
  FilterL.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(getSampleRate(), cutoff);
  
  FilterR.coefficients = juce::dsp::IIR::Coefficients<float>::makeBandPass(getSampleRate(), cutoff);
}    
}
else if (region_->filter == sfzero::Region::brf_2p)
{
if(region_->resonance > 0.0)
{
  *FilterL.coefficients = BandStop(getSampleRate(), cutoff, qval);     
  *FilterR.coefficients = BandStop(getSampleRate(), cutoff, qval);   
}
else
{
  *FilterL.coefficients = BandStop(getSampleRate(), cutoff, 1.0 / sqrt(2.0));     
  *FilterR.coefficients = BandStop(getSampleRate(), cutoff, 1.0 / sqrt(2.0));   
}  
}
else
filterinit = 0;	
}
}

void sfzero::Voice::startNote(int midiNoteNumber, float floatVelocity, juce::SynthesiserSound *soundIn,
                              int currentPitchWheelPosition)
{
  sfzero::Sound *sound = dynamic_cast<sfzero::Sound *>(soundIn);
	
  if(region_->pitchbendon)
  {
    if((curPitchWheel_ >= (region_->lobend + 8192)) && (curPitchWheel_ <= (region_->hibend + 8192)))
    {}
    else
    {
    killNote();
    return;
    } 
  }	

  if (sound == nullptr)
  {
    killNote();
    return;
  }

  curvel = floatVelocity * 127.0;
  int velocity = static_cast<int>(floatVelocity * 127.0);
  curVelocity_ = velocity;

  if ((region_ == nullptr) || (region_->sample == nullptr) || (region_->sample->getBuffer() == nullptr))
  {
    killNote();
    return;
  }
  if (region_->negative_end)
  {
    killNote();
    return;
  }

  // Pitch.
  curMidiNote_ = midiNoteNumber;
  curPitchWheel_ = currentPitchWheelPosition;
  calcPitchRatio();

  // Gain.
  double noteGainDB = globalGain + region_->volume;
  // Thanks to <http:://www.drealm.info/sfz/plj-sfz.xhtml> for explaining the
  // velocity curve in a way that I could understand, although they mean
  // "log10" when they say "log".
  double velocityGainDB = -20.0 * log10((127.0 * 127.0) / (velocity * velocity));
  velocityGainDB *= region_->amp_veltrack / 100.0;
  noteGainDB += velocityGainDB;
  
  float notetime = 0.0;
  float rt_decaygain = 0.0;
 
  if(region_->amp_randomon == 1)
  {
  float randomval = (float) rand() / (float) RAND_MAX;
  noteGainDB += randomval * region_->amp_random;
  if(noteGainDB > 24.0)
  noteGainDB = 24.0;
  } 
        
  if(region_->ampeg_delayccon == 1)
  {
  float ampegdelay = (region_->ampeg_delaycc * region_->ccvalmoved[region_->ampeg_delayccnum]) / 127.0;     
  ampegdelay += region_->delayegorg;     
  if(ampegdelay < 0.0)
  ampegdelay = 0.0;     
  if(ampegdelay > 100.0)
  ampegdelay = 100.0;     
  region_->ampeg.delay = ampegdelay;
  }  
  if(region_->ampeg_startccon == 1)
  {
  float ampegstart = (region_->ampeg_startcc * region_->ccvalmoved[region_->ampeg_startccnum]) / 127.0;     
  ampegstart += region_->startegorg;     
  if(ampegstart < 0.0)
  ampegstart = 0.0;     
  if(ampegstart > 100.0)
  ampegstart = 100.0;     
  region_->ampeg.start = ampegstart;
  }  
  if(region_->ampeg_attackccon == 1)
  {
  float ampegattack = (region_->ampeg_attackcc * region_->ccvalmoved[region_->ampeg_attackccnum]) / 127.0;     
  ampegattack += region_->attackegorg;     
  if(ampegattack < 0.0)
  ampegattack = 0.0;     
  if(ampegattack > 100.0)
  ampegattack = 100.0;     
  region_->ampeg.attack = ampegattack;
  }  
  if(region_->ampeg_holdccon == 1)
  {
  float ampeghold = (region_->ampeg_holdcc * region_->ccvalmoved[region_->ampeg_holdccnum]) / 127.0;     
  ampeghold += region_->holdegorg;     
  if(ampeghold < 0.0)
  ampeghold = 0.0;     
  if(ampeghold > 100.0)
  ampeghold = 100.0;     
  region_->ampeg.hold = ampeghold;
  }  
  if(region_->ampeg_decayccon == 1)
  {
  float ampegdecay = (region_->ampeg_decaycc * region_->ccvalmoved[region_->ampeg_decayccnum]) / 127.0;     
  ampegdecay += region_->decayegorg;     
  if(ampegdecay < 0.0)
  ampegdecay = 0.0;     
  if(ampegdecay > 100.0)
  ampegdecay = 100.0;     
  region_->ampeg.decay = ampegdecay;
  }  
  if(region_->ampeg_sustainccon == 1)
  {
  float ampegsustain = (region_->ampeg_sustaincc * region_->ccvalmoved[region_->ampeg_sustainccnum]) / 127.0;     
  ampegsustain += region_->sustainegorg;     
  if(ampegsustain < 0.0)
  ampegsustain = 0.0;     
  if(ampegsustain > 100.0)
  ampegsustain = 100.0;     
  region_->ampeg.sustain = ampegsustain;
  }  
  if(region_->ampeg_releaseccon == 1)
  {
  float ampegrelease = (region_->ampeg_releasecc * region_->ccvalmoved[region_->ampeg_releaseccnum]) / 127.0;     
  ampegrelease += region_->releaseegorg;     
  if(ampegrelease < 0.0)
  ampegrelease = 0.0;     
  if(ampegrelease > 100.0)
  ampegrelease = 100.0;     
  region_->ampeg.release = ampegrelease;
  }  

  float delaytime = 0.0;

  delaytime = region_->delaytime;
  
  if(region_->delay_randomon == 1)
  {
  float randomval = (float) rand() / (float) RAND_MAX;
  delaytime += randomval * region_->delay_random;
  }

  if(region_->delayccon == 1)
  {
  delaytime += (region_->delaycc * region_->ccvalmoved[region_->delayccnum]) / 127.0;    
  }

  if(delaytime > 100.0)
  delaytime = 100.0;
  if(delaytime < 0.0)
  delaytime = 0.0;
    
  if((region_->rt_decayon == 1) && (region_->scount > 0))
  {
  notetime = region_->scount / getSampleRate();
  rt_decaygain = pow(10, (-region_->rt_decay * (notetime + delaytime)) / 20);
  noteGainLeft_ = noteGainRight_ = static_cast<float>(juce::Decibels::decibelsToGain(noteGainDB)) * rt_decaygain;
  }
  else
  noteGainLeft_ = noteGainRight_ = static_cast<float>(juce::Decibels::decibelsToGain(noteGainDB));  
  // The SFZ spec is silent about the pan curve, but a 3dB pan law seems
  // common.  This sqrt() curve matches what Dimension LE does; Alchemy Free
  // seems closer to sin(adjustedPan * pi/2).
  double adjustedPan = (region_->pan + 100.0) / 200.0;
  noteGainLeft_ *= static_cast<float>(sqrt(1.0 - adjustedPan));
  noteGainRight_ *= static_cast<float>(sqrt(adjustedPan));    
      
  ampeg_.startNote(&region_->ampeg, midiNoteNumber, floatVelocity, getSampleRate(), delaytime, &region_->ampeg_veltrack);
    
  pitcheg_.startNote(&region_->pitcheg, midiNoteNumber, floatVelocity, getSampleRate(), region_->delaytime, &region_->pitcheg_veltrack);
    
  filtereg_.startNote(&region_->filtereg, midiNoteNumber, floatVelocity, getSampleRate(), region_->delaytime, &region_->filtereg_veltrack);  

  // Offset/end.

  double sourceSamplePosition2_ = static_cast<double>(region_->offset);
  
  if(region_->offset_randomon == 1)
  {
  float randomval = (float) rand() / (float) RAND_MAX;
  sourceSamplePosition2_ += randomval * static_cast<double>(region_->offset_random);
  }
 
  if(region_->offsetccon == 1)
  {
  sourceSamplePosition2_ += (static_cast<double>(region_->offsetcc) * region_->ccvalmoved[region_->offsetccnum]) / 127.0;    
  }  

  sourceSamplePosition_ = sourceSamplePosition2_;

  if(sourceSamplePosition_ < 0.0)
  sourceSamplePosition_ = 0.0;

  if(sourceSamplePosition_ > region_->sample->getSampleLength())
  sourceSamplePosition_ = region_->sample->getSampleLength();

  sampleEnd_ = region_->sample->getSampleLength();
  if ((region_->end > 0) && (region_->end < sampleEnd_))
  {
    sampleEnd_ = region_->end + 1;
  }

  // Loop.
  loopStart_ = loopEnd_ = 0;
  sfzero::Region::LoopMode loopMode = region_->loop_mode;
  if (loopMode == sfzero::Region::sample_loop)
  {
    if (region_->sample->getLoopStart() < region_->sample->getLoopEnd())
    {
      loopMode = sfzero::Region::loop_continuous;
    }
    else
    {
      loopMode = sfzero::Region::no_loop;
    }
  }
  if ((loopMode != sfzero::Region::no_loop) && (loopMode != sfzero::Region::one_shot))
  {
    if (region_->loop_start < region_->loop_end)
    {
      loopStart_ = region_->loop_start;
      loopEnd_ = region_->loop_end;
    }
    else
    {
      loopStart_ = region_->sample->getLoopStart();
      loopEnd_ = region_->sample->getLoopEnd();
    }
  }
  numLoops_ = 0;
  
  cutoffhold = 0;
      
  if(region_->filteron == 1)
  {
  if(region_->fil_randomon == 1)
  {
  float randomval = (float) rand() / (float) RAND_MAX;
  cutoffadj = region_->cutoff + (randomval * region_->fil_random);
  }
  else
  cutoffadj = region_->cutoff;
  if(region_->filveltrackon == 1)
  cutoffadj *= timecents2Secs((curvel * region_->fil_veltrack) / 127.0);    
  startfilter(cutoffadj, 1);  
  }
  
  blocklfopitch = 0;
  blockresetpitch = 0; 
  blocklfotrem = 0;
  blockresettrem = 0; 
  blocklfofilter = 0;
  blockresetfilter = 0;
  
  tremgain = 1.0;
  
  samplecount = 0;
  
  if((region_->pitchlfofilteron == 1) && (region_->pitchlfodepthon == 1))
  {
  startlfopitch(region_->pitchlfofreq, getSampleRate());
  }
  
  if((region_->tremlfofilteron == 1) && (region_->tremlfodepthon == 1))
  {
  startlfotrem(region_->tremlfofreq, getSampleRate());
  } 
  
  if((region_->lfofilteron == 1) && (region_->lfodepthon == 1))
  {
  startlfofilter(region_->lfofreq, getSampleRate());
  } 
}

void sfzero::Voice::stopNote(float /*velocity*/, bool allowTailOff)
{
  if (!allowTailOff || (region_ == nullptr))
  {
    killNote();
    return;
  }	
	
  if(region_)
  {
  if((region_->filteregdepthon == 1) && (region_->filtereg.release < 1.0))
  cutoffhold = 1;
  }
 
  if (region_->loop_mode != sfzero::Region::one_shot)
  {
    ampeg_.noteOff();
    pitcheg_.noteOff();
    filtereg_.noteOff();
  }
  if (region_->loop_mode == sfzero::Region::loop_sustain)
  {
    // Continue playing, but stop looping.
    loopEnd_ = loopStart_;
  }
}

void sfzero::Voice::stopNoteForGroup()
{
  if(isSustainPedalDown() || isSostenutoPedalDown()) 
  return;
	
  if(region_)
  {
  if((region_->filteregdepthon == 1) && (region_->filtereg.release < 1.0))
  cutoffhold = 1;
  }
  
//  if (region_->off_mode == sfzero::Region::fast)
  if (region_->offmode == 0)
  {
    ampeg_.fastRelease();
    pitcheg_.fastRelease();
    filtereg_.fastRelease();
  }
  else
  {
    ampeg_.noteOff();
    pitcheg_.noteOff();
    filtereg_.noteOff();
  }
}

void sfzero::Voice::stopNoteQuick() 
{ 
    if(isSustainPedalDown() || isSostenutoPedalDown()) 
    return;
	
    if(region_)
    {
    if((region_->filteregdepthon == 1) && (region_->filtereg.release < 1.0))
    cutoffhold = 1;
    }
  
    ampeg_.fastRelease(); 
    pitcheg_.fastRelease(); 
    filtereg_.fastRelease(); 
}

void sfzero::Voice::pitchWheelMoved(int newValue)
{
  if (region_ == nullptr)
  {
    return;
  }

  curPitchWheel_ = newValue;
  calcPitchRatio();
}

void sfzero::Voice::controllerMoved(int controllerNumber, int newValue)
{
  /*
  if(controllerNumber == 7)
      setMidiVolume(newValue);
  else
  */
      region_->ccvalmoved[controllerNumber] = newValue;
    
    /*
  switch(controllerNumber){
    case 7:
      setMidiVolume(newValue);
      break;
  }
  */
}

void sfzero::Voice::aftertouchChanged(int newAftertouchValue)
{
   region_->polyvalmoved[0] = newAftertouchValue;
}

void sfzero::Voice::channelPressureChanged(int newChannelPressureValue)
{
   region_->chanvalmoved = newChannelPressureValue;
}

float sfzero::Voice::fadein(int ccfadeval)
{
int midinote;
int lokey;
int hikey;

if(region_->fadeon == 0)
return 1.0;

float xfadein;

if(region_->ccnumfade == 1)
{
midinote = ccfadeval;
lokey = region_->ccnumfadeinlokey;
hikey = region_->ccnumfadeinhikey;
}
else
{
midinote = curMidiNote_;
lokey = region_->xfin_lokey;
hikey = region_->xfin_hikey;
}

if(midinote <= lokey)
{
xfadein = 0.0;
return xfadein;
}

if (midinote >= hikey)
{
xfadein = 1.0;
return xfadein;
}

float keydiff = midinote - lokey;
float notespan = hikey - lokey;
xfadein = keydiff / notespan;

if((region_->fadetype == 1) || (region_->fadetypecc == 1))
{
 xfadein = sin(xfadein * 0.5 * M_PI);
}
return xfadein;
}

float sfzero::Voice::fadeout(int ccfadeval)
{
int midinote;
int lokey;
int hikey;

if(region_->fadeon == 0)
return 1.0;

float xfadeout;

if(region_->ccnumfade == 1)
{
midinote = ccfadeval;
lokey = region_->ccnumfadeoutlokey;
hikey = region_->ccnumfadeouthikey;
}
else
{
midinote = curMidiNote_;
lokey = region_->xfout_lokey;
hikey = region_->xfout_hikey;
}

if(midinote >= hikey)
{
xfadeout = 0.0;
return xfadeout;
}

if (midinote <= lokey)
{
xfadeout = 1.0;
return xfadeout;
}

float keydiff = midinote - lokey;
float notespan = hikey - lokey;
xfadeout = 1.0 - keydiff / notespan;

if((region_->fadetype == 1) || (region_->fadetypecc == 1))
{
xfadeout = sin(xfadeout * 0.5 * M_PI);
}
return xfadeout;
}

float sfzero::Voice::fadeinvel()
{
int midivel;
int lovel;
int hivel;

if(region_->fadeonvel == 0)
return 1.0;

float xfadeinvel;

midivel = curvel;
lovel = region_->xfin_lovel;
hivel = region_->xfin_hivel;

if(midivel <= lovel)
{
xfadeinvel = 0.0;
return xfadeinvel;
}

if (midivel >= hivel)
{
xfadeinvel = 1.0;
return xfadeinvel;
}

float veldiff = midivel - lovel;
float velspan = hivel - lovel;
xfadeinvel = veldiff / velspan;

if(region_->fadetypevel == 1)
{
 xfadeinvel = sin(xfadeinvel * 0.5 * M_PI);
}
return xfadeinvel;
}

float sfzero::Voice::fadeoutvel()
{
int midivel;
int lovel;
int hivel;

if(region_->fadeonvel == 0)
return 1.0;

float xfadeoutvel;

midivel = curvel;
lovel = region_->xfout_lovel;
hivel = region_->xfout_hivel;

if(midivel >= hivel)
{
xfadeoutvel = 0.0;
return xfadeoutvel;
}

if (midivel <= lovel)
{
xfadeoutvel = 1.0;
return xfadeoutvel;
}

float veldiff = midivel - lovel;
float velspan = hivel - lovel;
xfadeoutvel = 1.0 - veldiff / velspan;

if(region_->fadetypevel == 1)
{
xfadeoutvel = sin(xfadeoutvel * 0.5 * M_PI);
}
return xfadeoutvel;
}

void sfzero::Voice::renderNextBlock(juce::AudioSampleBuffer &outputBuffer, int startSample, int numSamples)
{
  if (region_ == nullptr)
  {
    return;
  }
  
  float gainLeft;
  float gainRight;
	
  int s2z;	
  
  juce::AudioSampleBuffer *buffer = region_->sample->getBuffer();
  const float *inL = buffer->getReadPointer(0, 0);
  const float *inR = buffer->getNumChannels() > 1 ? buffer->getReadPointer(1, 0) : nullptr;

  float *outL = outputBuffer.getWritePointer(0, startSample);
  float *outR = outputBuffer.getNumChannels() > 1 ? outputBuffer.getWritePointer(1, startSample) : nullptr;

  int bufferNumSamples = buffer->getNumSamples(); // leoo

  // Cache some values, to give them at least some chance of ending up in
  // registers.
  
  double sourceSamplePosition;
  
  float ampegGain;
  float ampegSlope;
  int samplesUntilNextAmpSegment; 
  bool ampSegmentIsExponential; 
  
  float pitchegGain; 
  float pitchegSlope; 
  int samplesUntilNextAmpSegmentpitch; 
  bool ampSegmentIsExponentialpitch; 
  
  float filteregGain; 
  float filteregSlope; 
  int samplesUntilNextAmpSegmentfilter; 
  bool ampSegmentIsExponentialfilter; 
    
  if(sourceSamplePositionupdate_ == 1)
  {
  sourceSamplePosition = sourceSamplePosition2;  
  sourceSamplePositionupdate_ = 0; 
 
  ampegGain = ampegGain2;
  ampegSlope = ampegSlope2;
  samplesUntilNextAmpSegment = samplesUntilNextAmpSegment2;
  ampSegmentIsExponential = ampSegmentIsExponential2;  
  
  pitchegGain = pitchegGain2;
  pitchegSlope = pitchegSlope2;
  samplesUntilNextAmpSegmentpitch = samplesUntilNextAmpSegmentpitch2;
  ampSegmentIsExponentialpitch = ampSegmentIsExponentialpitch2;
    
  filteregGain = filteregGain2;
  filteregSlope = filteregSlope2;
  samplesUntilNextAmpSegmentfilter = samplesUntilNextAmpSegmentfilter2;
  ampSegmentIsExponentialfilter = ampSegmentIsExponentialfilter2; 
 
  ampeg_.setLevel(ampegGain);
  ampeg_.setSamplesUntilNextSegment(samplesUntilNextAmpSegment);
  pitcheg_.setLevel(pitchegGain);
  pitcheg_.setSamplesUntilNextSegment(samplesUntilNextAmpSegmentpitch);
  filtereg_.setLevel(filteregGain);
  filtereg_.setSamplesUntilNextSegment(samplesUntilNextAmpSegmentfilter);
   
  loopEnd_ = loopStart_;
  }
  else
  {
  sourceSamplePosition = this->sourceSamplePosition_;  
  
  ampegGain = ampeg_.getLevel();
  ampegSlope = ampeg_.getSlope();
  samplesUntilNextAmpSegment = ampeg_.getSamplesUntilNextSegment();
  ampSegmentIsExponential = ampeg_.getSegmentIsExponential();
  
  pitchegGain = pitcheg_.getLevel();
  pitchegSlope = pitcheg_.getSlope();
  samplesUntilNextAmpSegmentpitch = pitcheg_.getSamplesUntilNextSegment();
  ampSegmentIsExponentialpitch = pitcheg_.getSegmentIsExponential();
  
  filteregGain = filtereg_.getLevel();
  filteregSlope = filtereg_.getSlope();
  samplesUntilNextAmpSegmentfilter = filtereg_.getSamplesUntilNextSegment();
  ampSegmentIsExponentialfilter = filtereg_.getSegmentIsExponential();  
  }
  
  float loopStart = static_cast<float>(this->loopStart_);
  float loopEnd = static_cast<float>(this->loopEnd_);
  float sampleEnd = static_cast<float>(this->sampleEnd_);
    
  // float midiVolumeGainDB = -20.0 * log10((127.0 * 127.0) / (midiVolume_ * midiVolume_));
  // float midiVolumeGain = static_cast<float>(juce::Decibels::decibelsToGain(midiVolumeGainDB));
   
  float ccvolume = (region_->ccgain * region_->ccvalmoved[region_->ccnum]) / 127.0;
  float ccvolumegain = static_cast<float>(juce::Decibels::decibelsToGain(ccvolume));
    
  float fadeingain = fadein(region_->ccvalmoved[region_->ccnumfadein]);
  float fadeoutgain = fadeout(region_->ccvalmoved[region_->ccnumfadeout]);  
  
  float fadeinvelgain = fadeinvel();
  float fadeoutvelgain = fadeoutvel(); 
    
  s2z = 0;	
      
  int controlratemod = 64;
    
  while (--numSamples >= 0)
  {
    int pos = static_cast<int>(sourceSamplePosition);
    jassert(pos >= 0 && pos < bufferNumSamples); // leoo
    float alpha = static_cast<float>(sourceSamplePosition - pos);
    float invAlpha = 1.0f - alpha;
    int nextPos = pos + 1;
    if ((loopStart < loopEnd) && (nextPos > loopEnd))
    {
      nextPos = static_cast<int>(loopStart);
    }

    // Simple linear interpolation with buffer overrun check
    float nextL = nextPos < bufferNumSamples ? inL[nextPos] : inL[pos];
    float nextR = inR ? (nextPos < bufferNumSamples ? inR[nextPos] : inR[pos]) : nextL;
    float l = (inL[pos] * invAlpha + nextL * alpha);
    float r = inR ? (inR[pos] * invAlpha + nextR * alpha) : l;

    //// Simple linear interpolation, old version (possible buffer overrun with non-loop??)
    // float l = (inL[pos] * invAlpha + inL[nextPos] * alpha);
    // float r = inR ? (inR[pos] * invAlpha + inR[nextPos] * alpha) : l;

    /*  
    float gainLeft = noteGainLeft_  * ampegGain * midiVolumeGain;
    float gainRight = noteGainRight_ * ampegGain * midiVolumeGain;
    */
    // float gainLeft = noteGainLeft_  * ampegGain * midiVolumeGain * ccvolumegain * fadeingain * fadeoutgain; 
    // float gainRight = noteGainRight_ * ampegGain * midiVolumeGain * ccvolumegain * fadeingain * fadeoutgain;  
    
     if((region_->fadelfopitchon == 1) && (fadepitchdone == 0))
     fadepitchcount += 1.0;
     
     if((region_->fadelfoampon == 1) && (fadetremdone == 0))
     fadetremcount += 1.0;
     
     if((region_->fadelfofilon == 1) && (fadefilterdone == 0))
     fadefiltercount += 1.0;     
     
     if((region_->tremlfofilteron == 1) && (region_->tremlfodepthon == 1))
     {   
     int rem = blocklfotrem % controlratemod; 
     float ampdepth = 0.0; 
     float amplfoccfreq = 0.0;
     
     if(rem == 0)
     {     
     if(region_->amplfo_depthccon == 1)
     ampdepth += (region_->amplfo_depthccval * region_->ccvalmoved[region_->amplfo_depthccnum]) / 127.0;

     if((region_->amplfo_depthpolyon == 1) && (region_->polyvalmoved[1] == curMidiNote_))
     ampdepth += (region_->amplfo_depthpolyval * region_->polyvalmoved[0]) / 127.0;
     if(region_->amplfo_depthchanon == 1)
     ampdepth += (region_->amplfo_depthchanval * region_->chanvalmoved) / 127.0;
    
     ampdepth += region_->tremlfodepth;
     
     if(ampdepth < -10.0)
     ampdepth = -10.0;
     
     if(ampdepth > 10.0)
     ampdepth = 10.0;
     
     if(region_->amplfo_freqccon == 1)
     {
     amplfoccfreq += (region_->amplfo_freqccval * region_->ccvalmoved[region_->amplfo_freqccnum]) / 127.0;
     
     if((region_->amplfo_freqpolyon == 1) && (region_->polyvalmoved[1] == curMidiNote_))
     amplfoccfreq += (region_->amplfo_freqpolyval * region_->polyvalmoved[0]) / 127.0;  
     if(region_->amplfo_freqchanon == 1)
     amplfoccfreq += (region_->amplfo_freqchanval * region_->chanvalmoved) / 127.0;  

     amplfoccfreq += region_->tremlfofreq;
     
     if(amplfoccfreq < 0.0)
     amplfoccfreq = 0.0;
     
     if(amplfoccfreq > 200.0)
     amplfoccfreq = 200.0;   
     
     lfotremcc(amplfoccfreq, getSampleRate());
     }           
         
     tremgain = static_cast<float>(juce::Decibels::decibelsToGain(ampdepth * lfotremlevel)); 
     processlfotrem(controlratemod);  
     }  
     blocklfotrem++;
     }

     if(((region_->pitchlfofilteron == 1) && (region_->pitchlfodepthon == 1)) && (region_->troff == 0) || (region_->pitchegdepthon == 1))
     {   
     int rem = blocklfopitch % controlratemod;
     float pitchdepth = 0.0; 
     float pitchlfoccfreq = 0.0;
     
     if(rem == 0)
     {
     if(region_->pitchlfofilteron == 1)
     {
     if(region_->pitchlfo_depthccon == 1)
     pitchdepth += (region_->pitchlfo_depthccval * region_->ccvalmoved[region_->pitchlfo_depthccnum]) / 127.0;

     if((region_->pitchlfo_depthpolyon == 1)  && (region_->polyvalmoved[1] == curMidiNote_))
     pitchdepth += (region_->pitchlfo_depthpolyval * region_->polyvalmoved[0]) / 127.0;
     if(region_->pitchlfo_depthchanon == 1)
     pitchdepth += (region_->pitchlfo_depthchanval * region_->chanvalmoved) / 127.0;
     
     pitchdepth += region_->pitchlfodepth;
     
     if(pitchdepth < -1200.0)
     pitchdepth = -1200.0;
     
     if(pitchdepth > 1200.0)
     pitchdepth = 1200.0;
     
     if(region_->pitchlfo_freqccon == 1 || region_->pitchlfo_freqpolyon || region_->pitchlfo_freqchanon)
     {
     pitchlfoccfreq += (region_->pitchlfo_freqccval * region_->ccvalmoved[region_->pitchlfo_freqccnum]) / 127.0;

     if((region_->pitchlfo_freqpolyon == 1) && (region_->polyvalmoved[1] == curMidiNote_))
     pitchlfoccfreq += (region_->pitchlfo_freqpolyval * region_->polyvalmoved[0]) / 127.0;  
     if(region_->pitchlfo_freqchanon == 1)
     pitchlfoccfreq += (region_->pitchlfo_freqchanval * region_->chanvalmoved) / 127.0;  
     
     pitchlfoccfreq += region_->pitchlfofreq;
     
     if(pitchlfoccfreq < 0.0)
     pitchlfoccfreq = 0.0;
     
     if(pitchlfoccfreq > 200.0)
     pitchlfoccfreq = 200.0;   
     
     lfopitchcc(pitchlfoccfreq, getSampleRate());
     }  
     }

     if((region_->pitchegdepthon == 1) && (region_->pitchlfofilteron == 0))
     pitchRatio_ = timecents2Secs(pitchnum + (pitchegGain * region_->pitchegdepth)) * pitchden;    
     else if((region_->pitchegdepthon == 1) && (region_->pitchlfofilteron == 1))
     pitchRatio_ = timecents2Secs(pitchnum + (lfopitchlevel * pitchdepth) + (pitchegGain * region_->pitchegdepth)) * pitchden;  
     else if((region_->pitchegdepthon == 0) && (region_->pitchlfofilteron == 1))
 	 pitchRatio_ = timecents2Secs(pitchnum + (lfopitchlevel * pitchdepth)) * pitchden;
     if(region_->pitchlfofilteron == 1)
 	 processlfopitch(controlratemod);	
 	 }    
     blocklfopitch++;  
     }      

     if(((region_->lfofilteron == 1) && (region_->lfodepthon == 1)) || (region_->filteregdepthon == 1))
     {   
     int rem = blocklfofilter % controlratemod; 
     float filterdepth = 0.0; 
     float filterlfoccfreq = 0.0;  
     float egdepth = 0.0;
     float cutoffin = 0.0;
     int beginfilter = 0;
      
     if(rem == 0)
     {
     if(region_->lfofilteron == 1)
     {
     if(region_->fillfo_depthccon == 1)
     filterdepth += (region_->fillfo_depthccval * region_->ccvalmoved[region_->fillfo_depthccnum]) / 127.0;

     if((region_->fillfo_depthpolyon == 1) && (region_->polyvalmoved[1] == curMidiNote_))
     filterdepth += (region_->fillfo_depthpolyval * region_->polyvalmoved[0]) / 127.0;
     if(region_->fillfo_depthchanon == 1)
     filterdepth += (region_->fillfo_depthchanval * region_->chanvalmoved) / 127.0;
    
     filterdepth += region_->lfodepth;
     
     if(filterdepth < -1200.0)
     filterdepth = -1200.0;
     
     if(filterdepth > 1200.0)
     filterdepth = 1200.0;
    
     if(region_->fillfo_freqccon == 1 || region_->fillfo_freqpolyon || region_->fillfo_freqchanon)
     {
     if(region_->fillfo_freqccon == 1)
     filterlfoccfreq += (region_->fillfo_freqccval * region_->ccvalmoved[region_->fillfo_freqccnum]) / 127.0;

     if((region_->fillfo_freqpolyon == 1) && (region_->polyvalmoved[1] == curMidiNote_))
     filterlfoccfreq += (region_->fillfo_freqpolyval * region_->polyvalmoved[0]) / 127.0;  
     if(region_->fillfo_freqchanon == 1)
     filterlfoccfreq += (region_->fillfo_freqchanval * region_->chanvalmoved) / 127.0;  
     
     filterlfoccfreq += region_->lfofreq;
     
     if(filterlfoccfreq < 0.0)
     filterlfoccfreq = 0.0;
     
     if(filterlfoccfreq > 200.0)
     filterlfoccfreq = 200.0;   
     
     lfofiltercc(filterlfoccfreq, getSampleRate());
     }   
     }
 
     cutoffin = cutoffadj;  

     if((region_->cutoff_polyafton == 1) || (region_->cutoff_chanafton == 1))
     {
     if((region_->cutoff_polyafton == 1) && (region_->polyvalmoved[1] == curMidiNote_))
     {
     cutoffin += (region_->cutoffpoly * region_->polyvalmoved[0]) / 127.0;
     beginfilter = 1;
     }
     if(region_->cutoff_chanafton == 1)
     {
     cutoffin += (region_->cutoffchan * region_->chanvalmoved) / 127.0; 
     beginfilter = 1;
     }
     }

     if((region_->filteregdepthon == 1) && (region_->lfofilteron == 0))
     {
     egdepth = region_->filteregdepth;
     cutoffin *= timecents2Secs(egdepth * filteregGain); 
     beginfilter = 1;
     }
     else if((region_->filteregdepthon == 1) && (region_->lfofilteron == 1))
     {
     cutoffin *= timecents2Secs((filterdepth * lfofilterlevel) + (egdepth * filteregGain));
     beginfilter = 1;
     }
     else if((region_->filteregdepthon == 0) && (region_->lfofilteron == 1))
     {
     cutoffin *= timecents2Secs(filterdepth * lfofilterlevel); 
     beginfilter = 1;
     }  

     if(beginfilter == 1)  
     startfilter(cutoffin, 0);   
     if(region_->lfofilteron == 1)             
     processlfofilter(controlratemod); 
     }
     blocklfofilter++;
     }
    float gainout = ampegGain * ccvolumegain * fadeingain * fadeoutgain * fadeinvelgain * fadeoutvelgain;
    
    if((region_->tremlfofilteron == 1) && (region_->tremlfodepthon == 1))
    {    
    gainLeft = noteGainLeft_ * gainout * tremgain; 
    gainRight = noteGainRight_ * gainout * tremgain;  
    }      
    else
    {
    gainLeft = noteGainLeft_  * gainout; 
    gainRight = noteGainRight_ * gainout;             
    }     
  
    if((region_->cutoff > 0) && (region_->filteron == 1) && (filterinit == 1) && (ampegGain > 0.0))
    {   
    FilterL.snapToZero();
    FilterR.snapToZero();     
    l = FilterL.processSample(l);
    r = FilterR.processSample(r);
    /*	    
    ++s2z;	    
    if(s2z == 8)
    {	
    FilterL.snapToZero();
    FilterR.snapToZero();  
    s2z = 0;	    
    }	    
    }     
 */
    } 
        
    l *= gainLeft;
    r *= gainRight;
    // Shouldn't we dither here?

    if (outR)
    {
      *outL++ += l;
      *outR++ += r;
    }
    else
    {
      *outL++ += (l + r) * 0.5f;
    }
    
    samplecount++;

    // Next sample.
    sourceSamplePosition += pitchRatio_;
    if ((loopStart < loopEnd) && (sourceSamplePosition > loopEnd))
    {
      sourceSamplePosition = loopStart;
      numLoops_ += 1;
    }
    
    // Update EG.
    if (ampSegmentIsExponential)
    {
      ampegGain *= ampegSlope;
    }
    else
    {
      ampegGain += ampegSlope;
    }
    if (--samplesUntilNextAmpSegment < 0)
    {
      ampeg_.setLevel(ampegGain);
      ampeg_.nextSegment();
      ampegGain = ampeg_.getLevel();
      ampegSlope = ampeg_.getSlope();
      samplesUntilNextAmpSegment = ampeg_.getSamplesUntilNextSegment();
      ampSegmentIsExponential = ampeg_.getSegmentIsExponential();
    }
    
    if (ampSegmentIsExponentialpitch)
    {
      pitchegGain *= pitchegSlope;
    }
    else
    {
      pitchegGain += pitchegSlope;
    }
    if (--samplesUntilNextAmpSegmentpitch < 0)
    {
      pitcheg_.setLevel(pitchegGain);
      pitcheg_.nextSegment();
      pitchegGain = pitcheg_.getLevel();
      pitchegSlope = pitcheg_.getSlope();
      samplesUntilNextAmpSegmentpitch = pitcheg_.getSamplesUntilNextSegment();
      ampSegmentIsExponentialpitch = pitcheg_.getSegmentIsExponential();
    }
    
    if (ampSegmentIsExponentialfilter)
    {
      filteregGain *= filteregSlope;
    }
    else
    {
      filteregGain += filteregSlope;
    }
    if (--samplesUntilNextAmpSegmentfilter < 0)
    {
      filtereg_.setLevel(filteregGain);
      filtereg_.nextSegment();
      filteregGain = filtereg_.getLevel();
      filteregSlope = filtereg_.getSlope();
      samplesUntilNextAmpSegmentfilter = filtereg_.getSamplesUntilNextSegment();
      ampSegmentIsExponentialfilter = filtereg_.getSegmentIsExponential();
    }
         
    if(startedlate == 1)
    {
    if(region_->ccvalmoved[64] == 0)  
    {
    startedlate = 2;
    }   
    }  
    
    if ((sourceSamplePosition >= sampleEnd) || ampeg_.isDone() || startedlate == 2)
    {
      startedlate = 0;
      killNote();
      break;
    }
  }

  this->sourceSamplePosition_ = sourceSamplePosition;
  ampeg_.setLevel(ampegGain);
  ampeg_.setSamplesUntilNextSegment(samplesUntilNextAmpSegment);
  pitcheg_.setLevel(pitchegGain);
  pitcheg_.setSamplesUntilNextSegment(samplesUntilNextAmpSegmentpitch);
  filtereg_.setLevel(filteregGain);
  filtereg_.setSamplesUntilNextSegment(samplesUntilNextAmpSegmentfilter);
   
  ampegGain2 = ampegGain;
  ampegSlope2 = ampegSlope;
  samplesUntilNextAmpSegment2 = samplesUntilNextAmpSegment;
  ampSegmentIsExponential2 = ampSegmentIsExponential;  
  
  pitchegGain2 = pitchegGain;
  pitchegSlope2 = pitchegSlope;
  samplesUntilNextAmpSegmentpitch2 = samplesUntilNextAmpSegmentpitch;
  ampSegmentIsExponentialpitch2 = ampSegmentIsExponentialpitch;
  
  filteregGain2 = filteregGain;
  filteregSlope2 = filteregSlope;
  samplesUntilNextAmpSegmentfilter2 = samplesUntilNextAmpSegmentfilter;
  ampSegmentIsExponentialfilter2 = ampSegmentIsExponentialfilter;
  
  ampegGain3 = ampegGain;
  ampegSlope3 = ampegSlope;
  samplesUntilNextAmpSegment3 = samplesUntilNextAmpSegment;
  ampSegmentIsExponential3 = ampSegmentIsExponential;  
  
  pitchegGain3 = pitchegGain;
  pitchegSlope3 = pitchegSlope;
  samplesUntilNextAmpSegmentpitch3 = samplesUntilNextAmpSegmentpitch;
  ampSegmentIsExponentialpitch3 = ampSegmentIsExponentialpitch;
  
  filteregGain3 = filteregGain;
  filteregSlope3 = filteregSlope;
  samplesUntilNextAmpSegmentfilter3 = samplesUntilNextAmpSegmentfilter;
  ampSegmentIsExponentialfilter3 = ampSegmentIsExponentialfilter;    

}

bool sfzero::Voice::isPlayingNoteDown() { return region_ && region_->trigger != sfzero::Region::release; }

bool sfzero::Voice::isPlayingOneShot() { return region_ && region_->loop_mode == sfzero::Region::one_shot; }

int sfzero::Voice::getGroup() { return region_ ? region_->group : 0; }

juce::uint64 sfzero::Voice::getOffBy() { return region_ ? region_->off_by : 0; }

void sfzero::Voice::setRegion(sfzero::Region *nextRegion) { region_ = nextRegion; }

juce::String sfzero::Voice::infoString()
{
  const char *egSegmentNames[] = {"delay", "attack", "hold", "decay", "sustain", "release", "done"};

  const static int numEGSegments(sizeof(egSegmentNames) / sizeof(egSegmentNames[0]));

  const char *egSegmentName = "-Invalid-";
  int egSegmentIndex = ampeg_.segmentIndex();
  if ((egSegmentIndex >= 0) && (egSegmentIndex < numEGSegments))
  {
    egSegmentName = egSegmentNames[egSegmentIndex];
  }

  juce::String info;
  info << "note: " << curMidiNote_ << ", vel: " << curVelocity_ << ", pan: " << region_->pan << ", eg: " << egSegmentName
       << ", loops: " << numLoops_;
  return info;
}

void sfzero::Voice::calcPitchRatio()
{
  if((curMidiNote_ == 0) || (region_->troff == 1))
  {
  pitchRatio_ = 1;
  return;
  }
    
  double note = curMidiNote_;

  note += region_->transpose;
  note += region_->tune / 100.0;

  double adjustedPitch = region_->pitch_keycenter + (note - region_->pitch_keycenter) * (region_->pitch_keytrack / 100.0);
  if (curPitchWheel_ != 8192)
  {
    double wheel = ((2.0 * curPitchWheel_ / 16383.0) - 1.0);
    if (wheel > 0)
    {
      adjustedPitch += wheel * region_->bend_up / 100.0;
    }
    else
    {
      adjustedPitch += wheel * region_->bend_down / -100.0;
    }
  }

    if(region_->pitch_randomon == 1)
    {
    float randomval = (float) rand() / (float) RAND_MAX;
    pitchnum = adjustedPitch * 100.0 + (randomval * region_->pitch_random);
    if(pitchnum > 9600.0)
    pitchnum = 9600.0;
    if(pitchnum < -9600.0)
    pitchnum = -9600.0;    
    }
    else
	pitchnum = adjustedPitch * 100.0;
	
    if(region_->pitchveltrackon == 1)
    {
    pitchnum = pitchnum + ((curvel * region_->pitch_veltrack) / 127.0);   
    }
	
	pitchden = region_->sample->getSampleRate() / (timecents2Secs(region_->pitch_keycenter * 100.0) * getSampleRate());
	
	if((region_->pitchlfofilteron == 0) && (region_->pitchegdepthon == 0))
  pitchRatio_ = timecents2Secs(pitchnum) * pitchden;       
}

void sfzero::Voice::killNote()
{
/*
  blocklfopitch = 0;
  blockresetpitch = 0; 
  blocklfotrem = 0;
  blockresettrem = 0; 
  blocklfofilter = 0;
  blockresetfilter = 0;
*/
  region_ = nullptr;
  clearCurrentNote();
}

inline double sfzero::Voice::fractionalMidiNoteInHz(double note, double freqOfA)
{
  // Like MidiMessage::getMidiNoteInHertz(), but with a float note.
  note -= 69;
  // Now 0 = A
  return freqOfA * pow(2.0, note / 12.0);
}

inline float sfzero::Voice::timecents2Secs(float timecents) { return static_cast<float>(pow(2.0, timecents / 1200.0)); }

/*
void sfzero::Voice::setMidiVolume(int volume)
{
  midiVolume_=volume;
}
*/

