/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZREGION_H_INCLUDED
#define SFZREGION_H_INCLUDED

#include "SFZCommon.h"

namespace sfzero
{

class Sample;

// Region is designed to be able to be bitwise-copied.

struct EGParameters
{
  float delay, start, attack, hold, decay, sustain, release, depth;

  void clear();
  void clearMod();
};

struct Region
{
  enum Trigger
  {
    attack,
    release,
    first,
    legato
  };

  enum LoopMode
  {
    sample_loop,
    no_loop,
    one_shot,
    loop_continuous,
    loop_sustain
  };

  enum OffMode
  {
    fast,
    normal
  };
  
  enum Filter 
  {
	lpf_1p, 
	hpf_1p, 
	lpf_2p, 
	hpf_2p, 
	bpf_2p, 
	brf_2p, 
	notused
  };  

  Region();
  void clear();
  void clearForSF2();
  void clearForRelativeSF2();
  void addForSF2(Region *other);
  void sf2ToSFZ();
  juce::String dump();

  bool matches(int note, int velocity, Trigger trig)
  {
    return (note >= lokey && note <= hikey && velocity >= lovel && velocity <= hivel &&
            (trig == this->trigger || (this->trigger == attack && (trig == first || trig == legato))));
  }
  
    bool matches2(int note, int velocity, Trigger trig, float randomval, int ccvallo, int ccvalhi, int polyval, int chanval, int midich, int *lastkeyval, int *prevkeyval)
  { 
    if(note >= lokey && note <= hikey && velocity >= lovel && velocity <= hivel && (lorandomval <= randomval && hirandomval > randomval) && ((midich >= lochan) && (midich <= hichan)) && !ccontrig && ((ccnumloval == 0 && ccnumhival == 127) || ((ccvallo >= ccnumloval && ccvalhi <= ccnumhival) && ccon)) && ((lopolyaft == 0 && hipolyaft == 127) || ((polyval >= lopolyaft && polyval <= hipolyaft) && polyafton)) && ((lochanaft == 0 && hichanaft == 127) || ((chanval >= lochanaft && chanval <= hichanaft) && chanafton)) && (sw_last == 10000 || (((sw_last >= sw_lokey) && (sw_last <= sw_hikey) && swlaston) ? (sw_last == *lastkeyval) : false)) && (sw_down == 10000 || (((sw_down >= sw_lokey) && (sw_down <= sw_hikey) && swdownon) ? (swupdownarray[sw_down] > 0) : false)) && (sw_up == 10000 || (((sw_up >= sw_lokey) && (sw_up <= sw_hikey) && swupon) ? (swupdownarray[sw_up] == 0) : false)) && (sw_previous == 10000 || ((sw_previous == *prevkeyval) && swprevon)) && (trig == this->trigger || (this->trigger == attack && (trig == first || trig == legato))))
  {        
    int prevseqcount = seqcount;
    ++seqcount;
      
    if(seqcount > seqlencount)
    seqcount = 0;
                   
    return prevseqcount == seqposcount;                
  }   
    return false;                
  }
  
    bool matches3(int note, int velocity, Trigger trig, float randomval, int ccvallo, int ccvalhi, int polyval, int chanval, int midich, int *lastkeyval, int *prevkeyval)
  {
    if(note >= lokey && note <= hikey && velocity >= lovel && velocity <= hivel && (lorandomval <= randomval && hirandomval > randomval) && ((midich >= lochan) && (midich <= hichan)) && !ccontrig && ((ccnumloval == 0 && ccnumhival == 127) || ((ccvallo >= ccnumloval && ccvalhi <= ccnumhival) && ccon)) && ((lopolyaft == 0 && hipolyaft == 127) || ((polyval >= lopolyaft && polyval <= hipolyaft) && polyafton)) && ((lochanaft == 0 && hichanaft == 127) || ((chanval >= lochanaft && chanval <= hichanaft) && chanafton)) && (sw_last == 10000 || (((sw_last >= sw_lokey) && (sw_last <= sw_hikey) && swlaston) ? (sw_last == *lastkeyval) : false)) && (sw_down == 10000 || (((sw_down >= sw_lokey) && (sw_down <= sw_hikey) && swdownon) ? (swupdownarray[sw_down] > 0) : false)) && (sw_up == 10000 || (((sw_up >= sw_lokey) && (sw_up <= sw_hikey) && swupon) ? (swupdownarray[sw_up] == 0) : false)) && (sw_previous == 10000 || ((sw_previous == *prevkeyval) && swprevon)) && (trig == this->trigger))
  {      
    int prevseqcount = seqcount;
    ++seqcount;
      
    if(seqcount > seqlencount)
    seqcount = 0;
                   
    return prevseqcount == seqposcount;            
  }  
    return false;                
  }  
  
  bool matches4(int ccvallotrig, int ccvalhiltrig, float randomval, int midich)
  { 
    if((lorandomval <= randomval && hirandomval > randomval) && ((midich >= lochan) && (midich <= hichan)) && ccontrig && (ccnumlovaltrig <= ccvallotrig && ccnumhivaltrig >= ccvalhiltrig))
    {
    int prevseqcount = seqcount;
    ++seqcount;
      
    if(seqcount > seqlencount)
    seqcount = 0;
                   
    return prevseqcount == seqposcount;       
    }
    return false;
  }
   
  int seqposcount;
  int seqlencount;
  int seqcount;
  int seq_length;
  int seq_position;
  
  float lorandomval;
  float hirandomval;
  
  float ccgain;
  int ccnum;
  
  int ccnumlo;          
  int ccnumhi; 
  int ccnumloval;          
  int ccnumhival; 
  int ccon;
  
  int ccnumlotrig;          
  int ccnumhitrig; 
  int ccnumlovaltrig;          
  int ccnumhivaltrig; 
  int ccontrig;

  int lochan; 
  int hichan;
  
  int xfin_lokey;
  int xfin_hikey;  
  int xfout_lokey;
  int xfout_hikey;
  int fadeon;
  int fadetype;
  int fadeonvel;
  int fadetypevel;
  int xfin_lovel;
  int xfin_hivel;  
  int xfout_lovel;
  int xfout_hivel;    
  int ccnumfade;
  int ccnumfadein;
  int ccnumfadeinlokey;
  int ccnumfadeinhikey;
  int ccnumfadeout;
  int ccnumfadeoutlokey;
  int ccnumfadeouthikey;
  int fadetypecc;
    
  Filter filter;
  float cutoff;
  float resonance;
  int filteron;  
    
  float lfofreq;
  int lfofilteron;
  float lfodepth;
  int lfodepthon;
  float pitchlfofreq;  
  int pitchlfofilteron;
  
  float pitchlfodepth;
  int pitchlfodepthon;  
  float tremlfofreq;
  int tremlfofilteron;
  float tremlfodepth;
  int tremlfodepthon;
 
  float delaypitchval;
  int delaylfopitchon;
  float fadepitchval;
  int fadelfopitchon; 
  float delayampval;
  int delaylfoampon;  
  float delayfilval;
  int delaylfofilon;  
  float fadeampval;
  int fadelfoampon;    
  float fadefilval;
  int fadelfofilon;

  int pitchegdepthon;  
  int filteregdepthon;
  float pitchegdepth;
  float filteregdepth;
  
  float pitch_veltrack;
  float fil_veltrack;
  int pitchveltrackon;
  int filveltrackon;
  
  int pitchlfo_depthccnum;
  float pitchlfo_depthccval;
  int pitchlfo_depthccon;  
  int pitchlfo_freqccnum;
  float pitchlfo_freqccval;
  int pitchlfo_freqccon;  
  int fillfo_depthccnum;
  float fillfo_depthccval;
  int fillfo_depthccon;  
  int fillfo_freqccnum;
  float fillfo_freqccval;
  int fillfo_freqccon;  
  int amplfo_depthccnum;
  float amplfo_depthccval;
  int amplfo_depthccon; 
  int amplfo_freqccnum;
  float amplfo_freqccval;
  int amplfo_freqccon;  
  
  int ampeg_delayccnum;
  float ampeg_delaycc;
  int ampeg_delayccon;
  int ampeg_startccnum;
  float ampeg_startcc;
  int ampeg_startccon;
  int ampeg_attackccnum;
  float ampeg_attackcc;
  int ampeg_attackccon;
  int ampeg_holdccnum;
  float ampeg_holdcc;
  int ampeg_holdccon;
  int ampeg_decayccnum;
  float ampeg_decaycc;
  int ampeg_decayccon;
  int ampeg_sustainccnum;
  float ampeg_sustaincc;
  int ampeg_sustainccon;
  int ampeg_releaseccnum;
  float ampeg_releasecc;
  int ampeg_releaseccon;    
  
  float delayegorg;
  float startegorg;
  float attackegorg;
  float holdegorg;
  float decayegorg;
  float sustainegorg;
  float releaseegorg;  
  
  int amp_randomon;
  int delay_randomon;
  int offset_randomon;
  int fil_randomon;
  int pitch_randomon;
  float amp_random;
  float delay_random;
  float fil_random;
  float pitch_random;

  float delaycc;
  int delayccnum;
  int delayccon;

  int offsetccnum;
  int offsetccon;

  float rt_decay;
  int rt_decayon;
  int scount;
  
  int ccvalmoved[128]; 

  int lopolyaft;
  int hipolyaft;
  float cutoffpoly;
  int cutoff_polyafton;
  float pitchlfo_depthpolyval;
  int pitchlfo_depthpolyon;
  float pitchlfo_freqpolyval;
  int pitchlfo_freqpolyon;
  float fillfo_depthpolyval;
  int fillfo_depthpolyon;
  float fillfo_freqpolyval;
  int fillfo_freqpolyon;
  float amplfo_depthpolyval;
  int amplfo_depthpolyon;
  float amplfo_freqpolyval;
  int amplfo_freqpolyon;
  int polyafton;
  int polyvalmoved[2];

  int lochanaft;
  int hichanaft;
  float cutoffchan;
  int cutoff_chanafton;
  float pitchlfo_depthchanval;
  int pitchlfo_depthchanon;
  float pitchlfo_freqchanval;
  int pitchlfo_freqchanon;
  float fillfo_depthchanval;
  int fillfo_depthchanon;
  float fillfo_freqchanval;
  int fillfo_freqchanon;
  float amplfo_depthchanval;
  int amplfo_depthchanon;
  float amplfo_freqchanval;
  int amplfo_freqchanon;
  int chanafton;
  int chanvalmoved;
	
  int sw_lokey;
  int sw_hikey;
  int sw_up;
  int sw_down;
  int sw_last;
  int sw_previous;
  int swlaston;
  int swdownon;
  int swupon;
  int swprevon;
  int swupdownarray[128];
  int swlastval;
  int swlastvaltmp;
  int swprevval;
  int swprevvaltmp;
  
  int pitchbendon;
  int lobend;
  int hibend;
  
  int sw_vel; 
  int swvelon;  	

  int offmode;
     
  int troff;    
  
  Sample *sample;
  int lokey, hikey;
  int lovel, hivel;
  Trigger trigger;
  int group;
  juce::int64 off_by;
  // OffMode off_mode;

  juce::int64 offset;
  juce::int64 offset_random;
  juce::int64 offsetcc;
  juce::int64 end;
  bool negative_end;
  LoopMode loop_mode;
  juce::int64 loop_start, loop_end;
  int transpose;
  int tune;
  int pitch_keycenter, pitch_keytrack;
  int bend_up, bend_down;

  float volume, pan, delaytime;
  float amp_veltrack;

  EGParameters ampeg, ampeg_veltrack, pitcheg, pitcheg_veltrack, filtereg, filtereg_veltrack;

  static float timecents2Secs(int timecents);
};
}

#endif // SFZREGION_H_INCLUDED
