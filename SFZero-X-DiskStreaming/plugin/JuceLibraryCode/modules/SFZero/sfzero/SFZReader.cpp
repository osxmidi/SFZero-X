/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#include "SFZReader.h"
#include "SFZRegion.h"
#include "SFZSound.h"

std::string& trim(std::string &s)
{
    std::size_t hit = s.find_last_not_of("\t\n\v\f\r ") + 1;

    if(hit == s.size() || hit == std::string::npos){
        hit = s.find_first_not_of("\t\n\v\f\r ");
        s = s.substr(hit);
    }
    else
    s.erase(hit);

    return s;
}

sfzero::Reader::Reader(sfzero::Sound *soundIn) : sound_(soundIn), line_(1), globalon(0) 
{

}

sfzero::Reader::~Reader() {}

void sfzero::Reader::read(const juce::File &file, char *includepath)
{
  std::string cdir, cdir2;
  juce::MemoryBlock contents;
  bool ok;

  if(includepath[0] == '\0')
  {
  curread = file;
  loadstring = file.getFullPathName();
  loadstring = loadstring.upToLastOccurrenceOf("/", false, false);
  cdir = loadstring.toStdString();
  juce::File tmpFile(cdir.c_str());
  curdir = tmpFile;
  }
  else
  {
  loadstring = file.getFullPathName();
  cdir = loadstring.toStdString();
  cdir = cdir + "/";
  cdir2 = includepath;
  cdir += cdir2;
  juce::File tmpFile2(cdir.c_str());
  curread = tmpFile2;
  }

  juce::String value2;
  juce::String itfirst;
  juce::String itsecond;	
  std::size_t found;
  std::string cdir3;
 
  if(loadstring.contains("$"))
  {
  cdir3 = loadstring.toStdString();
  it = definemacro.begin();
  while(it != definemacro.end())
  {
  found = cdir3.find(it->first);
  if (found!=std::string::npos)
  {
  itfirst = it->first;
  itsecond = it->second;	  
  value2 = loadstring.replace(itfirst, itsecond, false);
  curread = value2;
  break;
  }
  it++;
  }
  }

  ok = curread.loadFileAsData(contents);
  if (!ok)
  {
    sound_->addError("Couldn't read \"" + curread.getFullPathName() + "\"");
    return;
  }
  read(static_cast<const char *>(contents.getData()), static_cast<int>(contents.getSize()));
}
void sfzero::Reader::read(const char *text, unsigned int length)
{
  const char *p = text;
  const char *end = text + length;
  char c = 0;

  sfzero::Region curGroup;
  sfzero::Region curRegion;
  sfzero::Region *buildingRegion = nullptr;
  bool inControl = false;
  juce::String defaultPath;
  char includebuf[2048];
  char *parptr = nullptr;
  juce::String value2;
  juce::String itfirst;
  juce::String itsecond;	
  std::size_t found;
  std::string cdir;
  char macroline[2048];
  int miss, idx;
  std::size_t pos, pos2; 
  std::string val;
  char *testp;
  
#ifdef SFZEROTR
  char* transposeval;  
  int transposeoff = 0;
  if((transposeval = getenv("SFZEROTR")) == NULL)
  {
  transposeoff = 0;
  }
  else
  {
  transposeoff = atoi(transposeval);
  if(transposeoff > 0)
  transposeoff = 1;
  }
#endif    
    
  while (p < end)
  {
    // We're at the start of a line; skip any whitespace.
    while (p < end)
    {
      c = *p;
      if ((c != ' ') && (c != '\t'))
      {
        break;
      }
      p += 1;
    }
    if (p >= end)
    {
      break;
    }

    // Check if it's a comment line.
    if (c == '/')
    {
      // Skip to end of line.
      while (p < end)
      {
        c = *++p;
        if ((c == '\n') || (c == '\r'))
        {
          break;
        }
      }
      p = handleLineEnd(p);
      continue;
    }

    // Check if it's a blank line.
    if ((c == '\r') || (c == '\n'))
    {
      p = handleLineEnd(p);
      continue;
    }

    // Handle elements on the line.
    while (p < end)
    {
      c = *p;

      // Tag.
      if (c == '<')
      {
        p += 1;
        const char *tagStart = p;
        while (p < end)
        {
          c = *p++;
          if ((c == '\n') || (c == '\r'))
          {
            error("Unterminated tag");
            goto fatalError;
          }
          else if (c == '>')
          {
            break;
          }
        }
        if (p >= end)
        {
          error("Unterminated tag");
          goto fatalError;
        }
        sfzero::StringSlice tag(tagStart, p - 1);
        if (tag == "region")
        {
          if (buildingRegion && (buildingRegion == &curRegion))
          {
            finishRegion(&curRegion);
          }
          curRegion = curGroup;
          buildingRegion = &curRegion;
          inControl = false;
        }
        else if (tag == "group")
        {
          if (buildingRegion && (buildingRegion == &curRegion))
          {
            finishRegion(&curRegion);
          }
          
          if(globalon == 1)
          memcpy(&curGroup, &sound_->curGlobal, sizeof(curGroup));
          else
          curGroup.clear();
          
          buildingRegion = &curGroup;
          inControl = false;
        }
        else if (tag == "global")
        { 
          sound_->curGlobal.clear();
          buildingRegion = &sound_->curGlobal;
          globalon = 1;
             
          inControl = false;
        }                             
        else if (tag == "control")
        {
          if (buildingRegion && (buildingRegion == &curRegion))
          {
            finishRegion(&curRegion);
          }
          curGroup.clear();
          buildingRegion = nullptr;
          inControl = true;
        }
        else
        {
          error("Illegal tag");
        }
      }
      // Comment.
      else if (c == '/')
      {
        // Skip to end of line.
        while (p < end)
        {
          c = *p;
          if ((c == '\n') || (c == '\r') || (c == '\t'))
          {
            break;
          }
          p += 1;
        }
      }      
      else if (c == '#')
      {
      testp = (char *) p + 1;
      idx = 0;
      miss = 0;
      memset(macroline, 0, 2048);
      pos = 0;
      pos2 = 0;
            
      while (p < end)
      {
      c = *p++;
      if ((c == '\n') || (c == '\r') || (c == '\t'))
      {
      macroline[idx] = '\0';
      break;
      }
      macroline[idx] = c;
      idx++;
      if(idx >= 2048)
      {
      miss = 1;
      break;
      } 
      }
      if(*testp == 'i')
      {  
      if(miss == 0)
      {      
      val = macroline;
      val = trim(val);      
      pos = val.find_first_of("\"");      
      pos2 = val.find_last_of("\"");
      if (pos!=std::string::npos && pos2!=std::string::npos)
      {     
      pos++;
      val = val.substr(pos, pos2 - pos);      
      sfzero::Reader::read(curdir, (char *)val.c_str());  
      } 
      }
      }
      else if(*testp == 'd')
      {
      if(miss == 0)
      {
      val = macroline;
      val = trim(val);
      pos = val.find_last_of(" ");
      if (pos!=std::string::npos)
      {         
      std::string val2 = val.substr(pos);
      val2 = trim(val2);
      val = val.substr(0, pos);
      pos = val.find_first_not_of(" ", strlen("#define"));
      if (pos!=std::string::npos)
      {           
      std::string val3 = val.substr(pos);
      val3 = trim(val3);
      definemacro.insert(std::make_pair(val3, val2));                  
      }
      }     
      }
      }
      }      
      // Parameter.
      else
      {
        // Get the parameter name.
        const char *parameterStart = p;
        while (p < end)
        {
          c = *p++;
          if ((c == '=') || (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'))
          {
            break;
          }
        }
        if ((p >= end) || (c != '='))
        {
          error("Malformed parameter");
          goto nextElement;
        }
        sfzero::StringSlice opcode(parameterStart, p - 1);
        if (inControl)
        {
          if (opcode == "default_path")
          {
            p = readPathInto(&defaultPath, p, end);
          }
          else if (!strncmp(opcode.getStart(), "set_cc", 6))
          {
          int ccnum= 0;
          int ccmacrohit = 0;
  
          juce::String opcodemacro = juce::String(opcode.getStart(), opcode.length());
   
           if(opcodemacro.contains("$"))
           {
           cdir = opcodemacro.toStdString();
           it = definemacro.begin();
           while(it != definemacro.end())
           {
           found = cdir.find(it->first);
           if (found!=std::string::npos)
           {
           itfirst = it->first;
           itsecond = it->second;		   
           value2 = opcodemacro.replace(itfirst, itsecond, false);
           opcodemacro = value2;
           ccmacrohit = 1;
           break;
           }
           it++;
           }
           }
                
           cdir = opcodemacro.toStdString();
           const char *ccnummacro= cdir.c_str(); 
           if(ccmacrohit == 1)   
           ccnum = atoi(&ccnummacro[6]);
           else 
           ccnum = atoi(opcode.getStart() + 6);
           
           if((ccnum >= 0) && (ccnum < 128))
           {
           const char *valueStart = p;
           while (p < end)
           {
            c = *p;
            if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
            {
              break;
            }
            p++;
            }
            juce::String value(valueStart, p - valueStart);
            sound_->setcc[ccnum] = value.getIntValue(); 
            }            
          }           
          else
          {
            const char *valueStart = p;
            while (p < end)
            {
              c = *p;
              if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
              {
                break;
              }
              p++;
            }
            juce::String value(valueStart, p - valueStart);
            juce::String fauxOpcode = juce::String(opcode.getStart(), opcode.length()) + " (in <control>)";
            sound_->addUnsupportedOpcode(fauxOpcode);
          }
        }
        else if (opcode == "sample")
        {
          juce::String path;
          p = readPathInto(&path, p, end);
          if (!path.isEmpty())
          {
            if (buildingRegion)
            {
              buildingRegion->sample = sound_->addSample(path, defaultPath);
            }
            else
            {
              error("Adding sample outside a group or region");
            }
          }
          else
          {
            error("Empty sample path");
          }
        }
        else
        {
          const char *valueStart = p;
          while (p < end)
          {
            c = *p;
            if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
            {
              break;
            }
            p++;
          }
          juce::String value(valueStart, p - valueStart);
          if(value.contains("$"))
          {
          cdir = value.toStdString();
          it = definemacro.begin();
          while(it != definemacro.end())
          {
          found = cdir.find(it->first);
          if (found!=std::string::npos)
          {
	  itfirst = it->first;
          itsecond = it->second;	  
          value2 = value.replace(itfirst, itsecond, false);
          value = value2;
          break;
          }
          it++;
          }
          }
          if (buildingRegion == nullptr)
          {
            error("Setting a parameter outside a region or group");
          }
          else if (opcode == "seq_length")
          {
            buildingRegion->seq_length = value.getIntValue();
            if(buildingRegion->seq_length >= 1 && buildingRegion->seq_length <= 100)
            {
            buildingRegion->seqlencount = buildingRegion->seq_length - 1;
            buildingRegion->seqcount = 0;
            }
          }                
          else if (opcode == "seq_position")
          {
            buildingRegion->seq_position = value.getIntValue();
            if(buildingRegion->seq_position >= 1 && buildingRegion->seq_position <= 100)
            {
            buildingRegion->seqposcount = buildingRegion->seq_position - 1;
            buildingRegion->seqcount = 0;
            }
          }   
          else if (opcode == "hirand")
          {
            buildingRegion->hirandomval = value.getFloatValue();
            if(buildingRegion->hirandomval < 0.0 || buildingRegion->hirandomval > 1.0)
            buildingRegion->hirandomval = 1.0;
          }
          else if (opcode == "lorand")
          {
            buildingRegion->lorandomval = value.getFloatValue();  
            if(buildingRegion->lorandomval < 0.0 || buildingRegion->lorandomval > 1.0)
            buildingRegion->lorandomval = 0.0;       
          } 
          else if (!strncmp(opcode.getStart(), "gain_cc", 7))
          {
            int ccnum = atoi(opcode.getStart() + 7);
            if((ccnum >= 0) && (ccnum < 128))
            {
            buildingRegion->ccgain = value.getFloatValue();
            if(buildingRegion->ccgain < -144.0 || buildingRegion->ccgain > 48.0)
            {
            buildingRegion->ccgain = 0.0;
            ccnum = 0;
            }
            buildingRegion->ccnum = ccnum;            
            }
          }
          else if (!strncmp(opcode.getStart(), "gain_oncc", 9))
          {
            int ccnum = atoi(opcode.getStart() + 9);
            if((ccnum >= 0) && (ccnum < 128))
            {
            buildingRegion->ccgain = value.getFloatValue();
            if(buildingRegion->ccgain < -144.0 || buildingRegion->ccgain > 48.0)
            {
            buildingRegion->ccgain = 0.0;
            ccnum = 0;
            }
            buildingRegion->ccnum = ccnum;            
            }
          }
          else if (!strncmp(opcode.getStart(), "locc", 4))
          {
            int cclo = atoi(opcode.getStart() + 4);
            if((cclo >= 0) && (cclo < 128))
            {
            buildingRegion->ccnumlo = cclo;
            buildingRegion->ccnumloval = value.getIntValue();             
            buildingRegion->ccon = 1;
            }
          }
          else if (!strncmp(opcode.getStart(), "hicc", 4))
          {
            int cchi = atoi(opcode.getStart() + 4);
            if((cchi >= 0) && (cchi < 128))
            {
            buildingRegion->ccnumhi = cchi;
            buildingRegion->ccnumhival = value.getIntValue();             
            buildingRegion->ccon = 1;
            }
          } 
          else if (!strncmp(opcode.getStart(), "on_locc", 7))
          {
            int cclotrig = atoi(opcode.getStart() + 7);
            if((cclotrig >= 0) && (cclotrig < 128))
            {
            buildingRegion->ccnumlotrig = cclotrig;
            buildingRegion->ccnumlovaltrig = value.getIntValue();           
            buildingRegion->ccontrig = 1;
            }
          }
          else if (!strncmp(opcode.getStart(), "on_hicc", 7))
          {
            int cchitrig = atoi(opcode.getStart() + 7);
            if((cchitrig >= 0) && (cchitrig < 128))
            {
            buildingRegion->ccnumhitrig = cchitrig;
            buildingRegion->ccnumhivaltrig = value.getIntValue();          
            buildingRegion->ccontrig = 1;
            }
          }           
          else if (opcode == "xfin_lovel")
          {
            buildingRegion->xfin_lovel = value.getIntValue();
            if((buildingRegion->xfin_lovel >= 0) && (buildingRegion->xfin_lovel < 128))
            buildingRegion->fadeonvel = 1;
          }          
          else if (opcode == "xfin_hivel")
          {
            buildingRegion->xfin_hivel = value.getIntValue();
            if((buildingRegion->xfin_hivel >= 0) && (buildingRegion->xfin_hivel < 128))
            buildingRegion->fadeonvel = 1;
          } 
          else if (opcode == "xfout_lovel")
          {
            buildingRegion->xfout_lovel = value.getIntValue();
            if((buildingRegion->xfout_lovel >= 0) && (buildingRegion->xfout_lovel < 128))            
            buildingRegion->fadeonvel = 1;
          }          
          else if (opcode == "xfout_hivel")
          {
            buildingRegion->xfout_hivel = value.getIntValue();
            if((buildingRegion->xfout_hivel >= 0) && (buildingRegion->xfout_hivel < 128))                        
            buildingRegion->fadeonvel = 1;
          }          
          else if(opcode == "xf_velcurve")
          {
            buildingRegion->fadetypevel = fadeValue(value);         
          }                         
          else if (opcode == "xfin_lokey")
          {
            buildingRegion->xfin_lokey = value.getIntValue();
            if((buildingRegion->xfin_lokey >= 0) && (buildingRegion->xfin_lokey < 128))                        
            buildingRegion->fadeon = 1;
          }          
          else if (opcode == "xfin_hikey")
          {
            buildingRegion->xfin_hikey = value.getIntValue();
            if((buildingRegion->xfin_hikey >= 0) && (buildingRegion->xfin_hikey < 128))                                    
            buildingRegion->fadeon = 1;
          } 
          else if (opcode == "xfout_lokey")
          {
            buildingRegion->xfout_lokey = value.getIntValue();
            if((buildingRegion->xfout_lokey >= 0) && (buildingRegion->xfout_lokey < 128))                                                
            buildingRegion->fadeon = 1;
          }          
          else if (opcode == "xfout_hikey")
          {
            buildingRegion->xfout_hikey = value.getIntValue();
            if((buildingRegion->xfout_hikey >= 0) && (buildingRegion->xfout_hikey < 128))                                                            
            buildingRegion->fadeon = 1;
          } 
          else if(!strncmp(opcode.getStart(), "xfin_locc", 9))
          {
            int fadeinlo = atoi(opcode.getStart() + 9);
            if((fadeinlo >= 0) && (fadeinlo < 128))
            {
            buildingRegion->ccnumfadein = fadeinlo;
            buildingRegion->ccnumfadeinlokey = value.getIntValue();  
            buildingRegion->ccnumfade = 1;      
            buildingRegion->fadeon = 1;
            }
          }          
          else if(!strncmp(opcode.getStart(), "xfin_hicc", 9))
          {
            int fadeinhi = atoi(opcode.getStart() + 9);
            if((fadeinhi >= 0) && (fadeinhi < 128))
            {
            buildingRegion->ccnumfadein = fadeinhi;
            buildingRegion->ccnumfadeinhikey = value.getIntValue();  
            buildingRegion->ccnumfade = 1;      
            buildingRegion->fadeon = 1;
            }
          }          
           else if(!strncmp(opcode.getStart(), "xfout_locc", 10))
          {
            int fadeoutlo = atoi(opcode.getStart() + 10);
            if((fadeoutlo >= 0) && (fadeoutlo < 128))
            {
            buildingRegion->ccnumfadeout = fadeoutlo;
            buildingRegion->ccnumfadeoutlokey = value.getIntValue();  
            buildingRegion->ccnumfade = 1;      
            buildingRegion->fadeon = 1;
            }
          }          
          else if(!strncmp(opcode.getStart(), "xfout_hicc", 10))
          {
            int fadeouthi = atoi(opcode.getStart() + 10);
            if((fadeouthi >= 0) && (fadeouthi < 128))
            {
            buildingRegion->ccnumfadeout = fadeouthi;
            buildingRegion->ccnumfadeouthikey = value.getIntValue();  
            buildingRegion->ccnumfade = 1;      
            buildingRegion->fadeon = 1;
            }
          }                                                       
          else if(opcode == "xf_keycurve")
          {
            buildingRegion->fadetype = fadeValue(value);         
          }             
          else if(opcode == "xf_cccurve")
          {
            buildingRegion->fadetypecc = fadeValue(value);         
          }                                                                                   
          else if (opcode == "lochan")
          {
            buildingRegion->lochan = value.getIntValue();
            if(buildingRegion->lochan < 1 || buildingRegion->lochan > 16)
            buildingRegion->lochan = 1;
          }
          else if (opcode == "hichan")
          {
            buildingRegion->hichan = value.getIntValue();
            if(buildingRegion->hichan < 1 || buildingRegion->hichan > 16)
            buildingRegion->hichan = 16;            
          } 
          else if (opcode == "fil_type")
          {
			      buildingRegion->filter = static_cast<sfzero::Region::Filter>(filterValue(value));
            if(buildingRegion->filter == sfzero::Region::notused)
             buildingRegion->filteron = 0; 
            else
			       buildingRegion->filteron = 1;
		      }	
		  else if (opcode == "cutoff")
		  {
			buildingRegion->cutoff = value.getFloatValue();  
			if(buildingRegion->cutoff < 0.0 || buildingRegion->cutoff > 384000.0)
			buildingRegion->cutoff = 0.0;                    
		  }	
		  else if (opcode == "resonance")
		  {
			buildingRegion->resonance = value.getFloatValue();
			if(buildingRegion->resonance < 0.0 || buildingRegion->resonance > 40.0)
			buildingRegion->resonance = 0.0;
		  }	
		  else if (opcode == "fillfo_freq")
		  {
			buildingRegion->lfofreq = value.getFloatValue();
			if((buildingRegion->lfofreq < 0.0) || (buildingRegion->lfofreq > 200.0))
			buildingRegion->lfofilteron = 0;
			else
			buildingRegion->lfofilteron = 1;
		  }	   
		  else if (opcode == "fillfo_depth")
		  {
		    buildingRegion->lfodepth = value.getFloatValue();
		    if((buildingRegion->lfodepth < -1200.0) || (buildingRegion->lfodepth > 1200.0))
		    buildingRegion->lfodepthon = 0;
		    else
		    {		    
			buildingRegion->lfodepthon = 1;
			}
		  }
         else if (!strncmp(opcode.getStart(), "fillfo_depthcc", 14))
          {
            int ccnum = atoi(opcode.getStart() + 14);
            if((ccnum >= 0) && (ccnum < 128))
            {
            buildingRegion->fillfo_depthccnum = ccnum;
            buildingRegion->fillfo_depthccval = value.getFloatValue();
            if((buildingRegion->fillfo_depthccval < -1200.0) || (buildingRegion->fillfo_depthccval > 1200.0))
            buildingRegion->fillfo_depthccon = 0;
            else
            {
            buildingRegion->fillfo_depthccon = 1;
            buildingRegion->lfodepthon = 1;
            }
            }
          }           
          else if (!strncmp(opcode.getStart(), "fillfo_freqcc", 13))
          {
            int ccnum = atoi(opcode.getStart() + 13);
            if((ccnum >= 0) && (ccnum < 128))
            {
            buildingRegion->fillfo_freqccnum = ccnum;
            buildingRegion->fillfo_freqccval = value.getFloatValue();
            if((buildingRegion->fillfo_freqccval < -200.0) || (buildingRegion->fillfo_freqccval > 200.0))
            buildingRegion->fillfo_freqccon = 0;
            else
            {
            buildingRegion->fillfo_freqccon = 1;
            buildingRegion->lfofilteron = 1;
            }
            }
          }                           	  	          	 		  
		  else if (opcode == "pitchlfo_freq")
		  {
			buildingRegion->pitchlfofreq = value.getFloatValue();
			if((buildingRegion->pitchlfofreq < 0.0) || (buildingRegion->pitchlfofreq > 200.0))
			buildingRegion->pitchlfofilteron = 0;
			else
			buildingRegion->pitchlfofilteron = 1;
		  }		  		      
		  else if (opcode == "pitchlfo_depth")
		  {
		    buildingRegion->pitchlfodepth = value.getFloatValue();
		    if((buildingRegion->pitchlfodepth < -1200.0) || (buildingRegion->pitchlfodepth > 1200.0))
		    buildingRegion->pitchlfodepthon = 0;
		    else
			buildingRegion->pitchlfodepthon = 1;
		  }	
          else if (opcode == "pitchlfo_delay")
          {
            buildingRegion->delaypitchval = value.getFloatValue();
            if((buildingRegion->delaypitchval < 0.0) || (buildingRegion->delaypitchval > 100.0))
            buildingRegion->delaylfopitchon = 0; 
            else
            buildingRegion->delaylfopitchon = 1;
          }	
          else if (opcode == "amplfo_delay")
          {
            buildingRegion->delayampval = value.getFloatValue();
            if((buildingRegion->delayampval < 0.0) || (buildingRegion->delayampval > 100.0))
            buildingRegion->delaylfoampon = 0; 
            else
            buildingRegion->delaylfoampon = 1;
          }
          else if (opcode == "fillfo_delay")
          {
            buildingRegion->delayfilval = value.getFloatValue();
            if((buildingRegion->delayfilval < 0.0) || (buildingRegion->delayfilval > 100.0))
            buildingRegion->delaylfofilon = 0; 
            else
            buildingRegion->delaylfofilon = 1;
          }	          	          
          else if (opcode == "pitchlfo_fade")
          {
            buildingRegion->fadepitchval = value.getFloatValue();
            if((buildingRegion->fadepitchval < 0.0) || (buildingRegion->fadepitchval > 100.0))
            {
            buildingRegion->fadelfopitchon = 0;
            }
            else if ((buildingRegion->fadepitchval < 0.1) && (buildingRegion->fadepitchval > 0.0))
            {
            buildingRegion->fadepitchval = 0.1;
            buildingRegion->fadelfopitchon = 1;
            }
            else
            buildingRegion->fadelfopitchon = 1;
          }	
          else if (opcode == "amplfo_fade")
          {
            buildingRegion->fadeampval = value.getFloatValue();
            if((buildingRegion->fadeampval < 0.0) || (buildingRegion->fadeampval > 100.0))
            {
            buildingRegion->fadelfoampon = 0;
            }
            else if ((buildingRegion->fadeampval < 0.1) && (buildingRegion->fadeampval > 0.0))
            {
            buildingRegion->fadeampval = 0.1;
            buildingRegion->fadelfoampon = 1;
            }
            else
            buildingRegion->fadelfoampon = 1;
          }	          
          else if (opcode == "fillfo_fade")
          {
            buildingRegion->fadefilval = value.getFloatValue();
            if((buildingRegion->fadefilval < 0.0) || (buildingRegion->fadefilval > 100.0))
            {
            buildingRegion->fadelfofilon = 0;
            }
            else if((buildingRegion->fadefilval < 0.1) && (buildingRegion->fadefilval > 0.0))
            {
            buildingRegion->fadefilval = 0.1;
            buildingRegion->fadelfofilon = 1;
            }
            else
            buildingRegion->fadelfofilon = 1;
          }	
          else if (!strncmp(opcode.getStart(), "pitchlfo_depthcc", 16))
          {
            int ccnum = atoi(opcode.getStart() + 16);
            if((ccnum >= 0) && (ccnum < 128))
            {
            buildingRegion->pitchlfo_depthccnum = ccnum;
            buildingRegion->pitchlfo_depthccval = value.getFloatValue();
            if((buildingRegion->pitchlfo_depthccval < -1200.0) || (buildingRegion->pitchlfo_depthccval > 1200.0))
            buildingRegion->pitchlfo_depthccon = 0;
            else
            {
            buildingRegion->pitchlfo_depthccon = 1;
            buildingRegion->pitchlfodepthon = 1;
            }
            }
          }           
          else if (!strncmp(opcode.getStart(), "pitchlfo_freqcc", 15))
          {
            int ccnum = atoi(opcode.getStart() + 15);
            if((ccnum >= 0) && (ccnum < 128))
            {
            buildingRegion->pitchlfo_freqccnum = ccnum;
            buildingRegion->pitchlfo_freqccval = value.getFloatValue();
            if((buildingRegion->pitchlfo_freqccval < -200.0) || (buildingRegion->pitchlfo_freqccval > 200.0))
            buildingRegion->pitchlfo_freqccon = 0;
            else
            {
            buildingRegion->pitchlfo_freqccon = 1;
            buildingRegion->pitchlfofilteron = 1;
            }
            }
          }                           	  	          	  	
		  else if (opcode == "amplfo_freq")
		  {
			buildingRegion->tremlfofreq = value.getFloatValue();
			if((buildingRegion->tremlfofreq < 0.0) || (buildingRegion->tremlfofreq > 200.0))
			buildingRegion->tremlfofilteron = 0;
			else
			buildingRegion->tremlfofilteron = 1;
		  }		  		      	   
		  else if (opcode == "amplfo_depth")
		  {
			buildingRegion->tremlfodepth = value.getFloatValue();
			if((buildingRegion->tremlfodepth < -10) || (buildingRegion->tremlfodepth > 10))
			buildingRegion->tremlfodepthon = 0;
			else
			buildingRegion->tremlfodepthon = 1;
		  }
          else if (!strncmp(opcode.getStart(), "amplfo_depthcc", 14))
          {
            int ccnum = atoi(opcode.getStart() + 14);
            if((ccnum >= 0) && (ccnum < 128))
            {
            buildingRegion->amplfo_depthccnum = ccnum;
            buildingRegion->amplfo_depthccval = value.getFloatValue();
            if((buildingRegion->amplfo_depthccval < -10.0) || (buildingRegion->amplfo_depthccval > 10.0))
            buildingRegion->amplfo_depthccon = 0;
            else
            {
            buildingRegion->amplfo_depthccon = 1;
            buildingRegion->tremlfodepthon = 1;
            }
            }
          }           
          else if (!strncmp(opcode.getStart(), "amplfo_freqcc", 13))
          {
            int ccnum = atoi(opcode.getStart() + 13);
            if((ccnum >= 0) && (ccnum < 128))
            {
            buildingRegion->amplfo_freqccnum = ccnum;
            buildingRegion->amplfo_freqccval = value.getFloatValue();
            if((buildingRegion->amplfo_freqccval < -200.0) || (buildingRegion->amplfo_freqccval > 200.0))
            buildingRegion->amplfo_freqccon = 0;
            else
            {
            buildingRegion->amplfo_freqccon = 1;
            buildingRegion->tremlfofilteron = 1;
            }
            }
          }                    		
		  else if (opcode == "lokey")
          {
            int loval = value.getIntValue();
            if(loval < 0 || loval > 127)
            buildingRegion->lokey = 0;
            else
            buildingRegion->lokey = keyValue(value);
          }
          else if (opcode == "hikey")
          {
            int hival = value.getIntValue();
            if(hival < 0 || hival > 127)
            buildingRegion->hikey = 127;
            else          
            buildingRegion->hikey = keyValue(value);
          }
          else if (opcode == "key")
          {
            int keyval = value.getIntValue();
            if(keyval < 0 || keyval > 127)
            {
            buildingRegion->lokey = 0;
            buildingRegion->hikey = 127;          
            }
            else
            buildingRegion->hikey = buildingRegion->lokey = buildingRegion->pitch_keycenter = keyValue(value);
          }
          else if (opcode == "lovel")
          {
            buildingRegion->lovel = value.getIntValue();
            if(buildingRegion->lovel < 0 || buildingRegion->lovel > 127)
            buildingRegion->lovel = 0;
          }
          else if (opcode == "hivel")
          {
            buildingRegion->hivel = value.getIntValue();
            if(buildingRegion->hivel < 0 || buildingRegion->hivel > 127)
            buildingRegion->hivel = 127;
          }
          else if (opcode == "trigger")
          {
            buildingRegion->trigger = static_cast<sfzero::Region::Trigger>(triggerValue(value));
          }
          else if (opcode == "group")
          {
            buildingRegion->group = static_cast<int>(value.getLargeIntValue());
          }
          else if (opcode == "off_by" || opcode == "offby")
          {
            buildingRegion->off_by = value.getLargeIntValue();
            if(buildingRegion->off_by > 4294967296)
            buildingRegion->off_by = 0;
          }
          else if(opcode == "off_mode")
          {
            buildingRegion->offmode = offbyValue(value);         
          }             
          else if (opcode == "offset")
          {
            buildingRegion->offset = value.getLargeIntValue();
            if((buildingRegion->offset < 0) || (buildingRegion->offset > 4294967296))
            buildingRegion->offset = 0;            
          }
          else if (opcode == "end")
          {
            juce::int64 end2 = value.getLargeIntValue();
            if (end2 < 0 || end2 > 4294967296)
            {
              buildingRegion->negative_end = true;
            }
            else
            {
              buildingRegion->end = end2;
            }
          }
          else if (opcode == "loop_mode" || opcode == "loopmode")
          {
            bool modeIsSupported = value == "no_loop" || value == "one_shot" || value == "loop_continuous" || value == "loop_sustain";
            if (modeIsSupported)
            {
              buildingRegion->loop_mode = static_cast<sfzero::Region::LoopMode>(loopModeValue(value));
            }
            else
            {
              juce::String fauxOpcode = juce::String(opcode.getStart(), opcode.length()) + "=" + value;
              sound_->addUnsupportedOpcode(fauxOpcode);
            }
          }
          else if (opcode == "loop_start" || opcode == "loopstart")
          {
            buildingRegion->loop_start = value.getLargeIntValue();
            if (buildingRegion->loop_start < 0 || buildingRegion->loop_start > 4294967296)
            buildingRegion->loop_start = 0;
          }
          else if (opcode == "loop_end" || opcode == "loopend")
          {
            buildingRegion->loop_end = value.getLargeIntValue();
            if (buildingRegion->loop_end < 0 || buildingRegion->loop_end > 4294967296)
            buildingRegion->loop_end = 0;
          }
          else if (opcode == "transpose")
          {
            buildingRegion->transpose = value.getIntValue();
            if(buildingRegion->transpose < -127 || buildingRegion->transpose > 127)
            buildingRegion->transpose = 0;
          }
          else if (opcode == "tune")
          {
            buildingRegion->tune = value.getIntValue();
            if(buildingRegion->tune < -100 || buildingRegion->tune > 100)
            buildingRegion->tune = 0;
          }
          else if (opcode == "pitch_keycenter")
          {
            buildingRegion->pitch_keycenter = keyValue(value);
            if(buildingRegion->pitch_keycenter < -127 || buildingRegion->pitch_keycenter > 127)
            buildingRegion->pitch_keycenter = 60;
          }
          else if (opcode == "pitch_keytrack")
          {
            buildingRegion->pitch_keytrack = value.getIntValue();
            if(buildingRegion->pitch_keytrack < -1200 || buildingRegion->pitch_keytrack > 1200)    
            buildingRegion->pitch_keytrack = 100;              
          }
          else if (opcode == "bend_up" || opcode == "bendup")
          {
            buildingRegion->bend_up = value.getIntValue();
            if(buildingRegion->bend_up < -9600 || buildingRegion->bend_up > 9600)  
            buildingRegion->bend_up = 200;           
          }
          else if (opcode == "bend_down" || opcode == "benddown")
          {
            buildingRegion->bend_down = value.getIntValue();
            if(buildingRegion->bend_down < -9600 || buildingRegion->bend_down > 9600)  
            buildingRegion->bend_down = -200;                
          }
          else if (opcode == "volume")
          {
            buildingRegion->volume = value.getFloatValue();
            if(buildingRegion->volume < -144.0 || buildingRegion->volume > 6.0)
            buildingRegion->volume = 0.0;
          }
          else if (opcode == "pan")
          {
            buildingRegion->pan = value.getFloatValue();
            if(buildingRegion->pan < -100.0 || buildingRegion->pan > 100.0)
            buildingRegion->pan = 0.0;            
          }          
          else if (opcode == "rt_decay")
          {
            buildingRegion->rt_decay = value.getFloatValue();
            if(buildingRegion->rt_decay < 0.0 || buildingRegion->rt_decay > 200.0)  
            {
            buildingRegion->rt_decay = 0.0;
            buildingRegion->rt_decayon = 0;               
            }
            else       
            buildingRegion->rt_decayon = 1;
          }                    
          else if (opcode == "delay")
          {
            buildingRegion->delaytime = value.getFloatValue();
            if(buildingRegion->delaytime < 0.0 || buildingRegion->delaytime > 100.0)
            buildingRegion->delaytime = 0.0;  
          }      
          else if (!strncmp(opcode.getStart(), "delaycc", 7))
          {
          int ccnum = atoi(opcode.getStart() + 7);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->delayccnum = ccnum;
          buildingRegion->delaycc = value.getFloatValue();
          if((buildingRegion->delaycc < 0.0) || (buildingRegion->ampeg_delaycc > 100.0))
          {
            buildingRegion->delaycc = 0.0;
            buildingRegion->delayccon = 0;
          }
          else  
            buildingRegion->delayccon = 1;
          }
          }
          else if (!strncmp(opcode.getStart(), "delay_cc", 8))
          {
          int ccnum = atoi(opcode.getStart() + 8);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->delayccnum = ccnum;
          buildingRegion->delaycc = value.getFloatValue();
          if((buildingRegion->delaycc < 0.0) || (buildingRegion->delaycc > 100.0))
          {
            buildingRegion->delaycc = 0.0;
            buildingRegion->delayccon = 0;
          }
          else  
            buildingRegion->delayccon = 1;
          }
          }
          else if (!strncmp(opcode.getStart(), "offset_cc", 9))
          {
          int ccnum = atoi(opcode.getStart() + 9);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->offsetccnum = ccnum;
          buildingRegion->offsetcc = value.getLargeIntValue();
          if((buildingRegion->offsetcc < 0) || (buildingRegion->offsetcc > 4294967296))
          {
            buildingRegion->offsetcc = 0;
            buildingRegion->offsetccon = 0;
          }
          else  
            buildingRegion->offsetccon = 1;
          }
          }    
          else if (opcode == "amp_random")
          {
            buildingRegion->amp_random = value.getFloatValue();
            if(buildingRegion->amp_random < 0.0 || buildingRegion->amp_random > 24.0)
            {
            buildingRegion->amp_random = 0.0;
            buildingRegion->amp_randomon = 0;
            }
            else
            buildingRegion->amp_randomon = 1;
          }                    
          else if (opcode == "delay_random")
          {
            buildingRegion->delay_random = value.getFloatValue();
            if(buildingRegion->delay_random < 0.0 || buildingRegion->delay_random > 100.0)
            {
            buildingRegion->delay_random = 0.0;
            buildingRegion->delay_randomon = 0;
            }
            else            
            buildingRegion->delay_randomon = 1;
          }           
          else if (opcode == "offset_random")
          {
            buildingRegion->offset_random = value.getLargeIntValue();
            if(buildingRegion->offset_random < 0 || buildingRegion->offset_random > 4294967296)
            {
            buildingRegion->offset_random = 0;
            buildingRegion->offset_randomon = 0;
            }
            else                       
            buildingRegion->offset_randomon = 1;
          }             
          else if (opcode == "fil_random")
          {
            buildingRegion->fil_random = value.getFloatValue();
            if(buildingRegion->fil_random < 0.0 || buildingRegion->fil_random > 9600.0)
            {
            buildingRegion->fil_random = 0.0;
            buildingRegion->fil_randomon = 0;
            }
            else
            buildingRegion->fil_randomon = 1;
          }            
          else if (opcode == "pitch_random")
          {
            buildingRegion->pitch_random = value.getFloatValue();
            if(buildingRegion->pitch_random < 0.0 || buildingRegion->pitch_random > 9600.0)
            {
            buildingRegion->pitch_random = 0.0;
            buildingRegion->pitch_randomon = 0;
            }  
            else          
            buildingRegion->pitch_randomon = 1;
          }                                                         
          else if (opcode == "amp_veltrack")
          {
            buildingRegion->amp_veltrack = value.getFloatValue();
            if((buildingRegion->amp_veltrack < -100.0) || (buildingRegion->amp_veltrack > 100.0))
            buildingRegion->amp_veltrack = 100.0; 
          }
          else if (opcode == "pitch_veltrack")
          {
            buildingRegion->pitch_veltrack = value.getFloatValue();
            if((buildingRegion->pitch_veltrack < -9600.0) || (buildingRegion->pitch_veltrack > 9600.0))
           buildingRegion->pitchveltrackon = 0;
           else 
           buildingRegion->pitchveltrackon = 1;
          }
          else if (opcode == "fil_veltrack")
          {
            buildingRegion->fil_veltrack = value.getFloatValue();
            if((buildingRegion->fil_veltrack < -9600.0) || (buildingRegion->fil_veltrack > 9600.0))
           buildingRegion->filveltrackon = 0;
           else 
           buildingRegion->filveltrackon = 1;
          }                    
          else if (opcode == "ampeg_delay")
          {
            buildingRegion->ampeg.delay = value.getFloatValue();
            if((buildingRegion->ampeg.delay < 0.0) || (buildingRegion->ampeg.delay > 100.0))
            buildingRegion->ampeg.delay = 0.0;
            buildingRegion->delayegorg = buildingRegion->ampeg.delay;            
          }
          else if (opcode == "ampeg_start")
          {
            buildingRegion->ampeg.start = value.getFloatValue();
            if((buildingRegion->ampeg.start < 0.0) || (buildingRegion->ampeg.start > 100.0))
            buildingRegion->ampeg.start = 0.0;            
            buildingRegion->startegorg = buildingRegion->ampeg.start;               
          }
          else if (opcode == "ampeg_attack")
          {
            buildingRegion->ampeg.attack = value.getFloatValue();
            if((buildingRegion->ampeg.attack < 0.0) || (buildingRegion->ampeg.attack > 100.0))
            buildingRegion->ampeg.attack = 0.0;            
            buildingRegion->attackegorg = buildingRegion->ampeg.attack;               
          }
          else if (opcode == "ampeg_hold")
          {
            buildingRegion->ampeg.hold = value.getFloatValue();
            if((buildingRegion->ampeg.hold < 0.0) || (buildingRegion->ampeg.hold > 100.0))
            buildingRegion->ampeg.hold = 0.0;            
            buildingRegion->holdegorg = buildingRegion->ampeg.hold;               
          }
          else if (opcode == "ampeg_decay")
          {
            buildingRegion->ampeg.decay = value.getFloatValue();
            if((buildingRegion->ampeg.decay < 0.0) || (buildingRegion->ampeg.decay > 100.0))
            buildingRegion->ampeg.decay = 0.0;            
            buildingRegion->decayegorg = buildingRegion->ampeg.decay;               
          }
          else if (opcode == "ampeg_sustain")
          {
            buildingRegion->ampeg.sustain = value.getFloatValue();
            if((buildingRegion->ampeg.sustain < 0.0) || (buildingRegion->ampeg.sustain > 100.0))
            buildingRegion->ampeg.sustain = 100.0;            
            buildingRegion->sustainegorg = buildingRegion->ampeg.sustain;               
          }
          else if (opcode == "ampeg_release")
          {
            buildingRegion->ampeg.release = value.getFloatValue();
            if((buildingRegion->ampeg.release < 0.0) || (buildingRegion->ampeg.release > 100.0))
            buildingRegion->ampeg.release = 0.0;            
            buildingRegion->releaseegorg = buildingRegion->ampeg.release;               
          }
          else if (opcode == "ampeg_vel2delay")
          {
            buildingRegion->ampeg_veltrack.delay = value.getFloatValue();
            if((buildingRegion->ampeg_veltrack.delay < -100.0) || (buildingRegion->ampeg_veltrack.delay > 100.0))
            buildingRegion->ampeg_veltrack.delay = 0.0;            
          }
          else if (opcode == "ampeg_vel2attack")
          {
            buildingRegion->ampeg_veltrack.attack = value.getFloatValue();
            if((buildingRegion->ampeg_veltrack.attack < -100.0) || (buildingRegion->ampeg_veltrack.attack > 100.0))
            buildingRegion->ampeg_veltrack.attack = 0.0;            
          }
          else if (opcode == "ampeg_vel2hold")
          {
            buildingRegion->ampeg_veltrack.hold = value.getFloatValue();
            if((buildingRegion->ampeg_veltrack.hold < -100.0) || (buildingRegion->ampeg_veltrack.hold > 100.0))
            buildingRegion->ampeg_veltrack.hold = 0.0;            
          }
          else if (opcode == "ampeg_vel2decay")
          {
            buildingRegion->ampeg_veltrack.decay = value.getFloatValue();
            if((buildingRegion->ampeg_veltrack.decay < -100.0) || (buildingRegion->ampeg_veltrack.decay > 100.0))
            buildingRegion->ampeg_veltrack.decay = 0.0;            
          }
          else if (opcode == "ampeg_vel2sustain")
          {
            buildingRegion->ampeg_veltrack.sustain = value.getFloatValue();
            if((buildingRegion->ampeg_veltrack.sustain < -100.0) || (buildingRegion->ampeg_veltrack.sustain > 100.0))
            buildingRegion->ampeg_veltrack.sustain = 0.0;            
          }
          else if (opcode == "ampeg_vel2release")
          {
            buildingRegion->ampeg_veltrack.release = value.getFloatValue();
            if((buildingRegion->ampeg_veltrack.release < -100.0) || (buildingRegion->ampeg_veltrack.release > 100.0))
            buildingRegion->ampeg_veltrack.release = 0.0;            
          }          
          else if (!strncmp(opcode.getStart(), "ampeg_delaycc", 13))
          {
          int ccnum = atoi(opcode.getStart() + 13);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->ampeg_delayccnum = ccnum;
          buildingRegion->ampeg_delaycc = value.getFloatValue();
          if((buildingRegion->ampeg_delaycc < 0.0) || (buildingRegion->ampeg_delaycc > 100.0))
          {
            buildingRegion->ampeg_delaycc = 0.0;
            buildingRegion->ampeg_delayccon = 0;
          }
          else  
            buildingRegion->ampeg_delayccon = 1;
          }
          }
          else if (!strncmp(opcode.getStart(), "ampeg_startcc", 13))
          {
          int ccnum = atoi(opcode.getStart() + 13);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->ampeg_startccnum = ccnum;
          buildingRegion->ampeg_startcc = value.getFloatValue();
          if((buildingRegion->ampeg_startcc < 0.0) || (buildingRegion->ampeg_startcc > 100.0))
          {
            buildingRegion->ampeg_startcc = 0.0;
            buildingRegion->ampeg_startccon = 0; 
          }
          else  
            buildingRegion->ampeg_startccon = 1;
          }
          }
          else if (!strncmp(opcode.getStart(), "ampeg_attackcc", 14))
          {
          int ccnum = atoi(opcode.getStart() + 14);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->ampeg_attackccnum = ccnum;
          buildingRegion->ampeg_attackcc = value.getFloatValue();
          if((buildingRegion->ampeg_attackcc < 0.0) || (buildingRegion->ampeg_attackcc > 100.0))
          {
            buildingRegion->ampeg_attackcc = 0.0; 
            buildingRegion->ampeg_attackccon = 0;
          }  
          else
            buildingRegion->ampeg_attackccon = 1;
          }
          }
          else if (!strncmp(opcode.getStart(), "ampeg_holdcc", 12))
          {
          int ccnum = atoi(opcode.getStart() + 12);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->ampeg_holdccnum = ccnum;
          buildingRegion->ampeg_holdcc = value.getFloatValue();
          if((buildingRegion->ampeg_holdcc < 0.0) || (buildingRegion->ampeg_holdcc > 100.0))
          {
            buildingRegion->ampeg_holdcc = 0.0;
            buildingRegion->ampeg_holdccon = 0;
          }              
          else
            buildingRegion->ampeg_holdccon = 1;
          }
          }
          else if (!strncmp(opcode.getStart(), "ampeg_decaycc", 13))
          {
          int ccnum = atoi(opcode.getStart() + 13);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->ampeg_decayccnum = ccnum;
          buildingRegion->ampeg_decaycc = value.getFloatValue();
          if((buildingRegion->ampeg_decaycc < 0.0) || (buildingRegion->ampeg_decaycc > 100.0))
          {
            buildingRegion->ampeg_decaycc = 0.0; 
            buildingRegion->ampeg_decayccon = 0; 
          }
          else  
            buildingRegion->ampeg_decayccon = 1;
          }
          }
          else if (!strncmp(opcode.getStart(), "ampeg_sustaincc", 15))
          {
          int ccnum = atoi(opcode.getStart() + 15);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->ampeg_sustainccnum = ccnum;
          buildingRegion->ampeg_sustaincc = value.getFloatValue();
          if((buildingRegion->ampeg_sustaincc < 0.0) || (buildingRegion->ampeg_sustaincc > 100.0))
          {
            buildingRegion->ampeg_sustaincc = 100.0; 
            buildingRegion->ampeg_sustainccon = 0;
          }
          else  
            buildingRegion->ampeg_sustainccon = 1;
          }
          }
          else if (!strncmp(opcode.getStart(), "ampeg_releasecc", 15))
          {
          int ccnum = atoi(opcode.getStart() + 15);
          if((ccnum >= 0) && (ccnum < 128))
          {
          buildingRegion->ampeg_releaseccnum = ccnum;
          buildingRegion->ampeg_releasecc = value.getFloatValue();
          if((buildingRegion->ampeg_releasecc < 0.0) || (buildingRegion->ampeg_releasecc > 100.0))
          {
            buildingRegion->ampeg_releasecc = 0.0;
            buildingRegion->ampeg_releaseccon = 0; 
          }
          else     
            buildingRegion->ampeg_releaseccon = 1;
          }
          }                    
          else if (opcode == "pitcheg_delay")
          {
            buildingRegion->pitcheg.delay = value.getFloatValue();
            if((buildingRegion->pitcheg.delay < 0.0) || (buildingRegion->pitcheg.delay > 100.0))
            buildingRegion->pitcheg.delay = 0.0;            
          }
          else if (opcode == "pitcheg_start")
          {
            buildingRegion->pitcheg.start = value.getFloatValue();
            if((buildingRegion->pitcheg.start < 0.0) || (buildingRegion->pitcheg.start > 100.0))
            buildingRegion->pitcheg.start = 0.0;                           
          }
          else if (opcode == "pitcheg_attack")
          {
            buildingRegion->pitcheg.attack = value.getFloatValue();
            if((buildingRegion->pitcheg.attack < 0.0) || (buildingRegion->pitcheg.attack > 100.0))
            buildingRegion->pitcheg.attack = 0.0;                       
          }
          else if (opcode == "pitcheg_hold")
          {
            buildingRegion->pitcheg.hold = value.getFloatValue();
            if((buildingRegion->pitcheg.hold < 0.0) || (buildingRegion->pitcheg.hold > 100.0))
            buildingRegion->pitcheg.hold = 0.0;                         
          }
          else if (opcode == "pitcheg_decay")
          {
            buildingRegion->pitcheg.decay = value.getFloatValue();
            if((buildingRegion->pitcheg.decay < 0.0) || (buildingRegion->pitcheg.decay > 100.0))
            buildingRegion->pitcheg.decay = 0.0;                          
          }
          else if (opcode == "pitcheg_sustain")
          {
            buildingRegion->pitcheg.sustain = value.getFloatValue();
            if((buildingRegion->pitcheg.sustain < 0.0) || (buildingRegion->pitcheg.sustain > 100.0))
            buildingRegion->pitcheg.sustain = 100.0;                       
          }
          else if (opcode == "pitcheg_release")
          {
            buildingRegion->pitcheg.release = value.getFloatValue();
            if((buildingRegion->pitcheg.release < 0.0) || (buildingRegion->pitcheg.release > 100.0))
            buildingRegion->pitcheg.release = 0.0;                          
          }
          else if (opcode == "pitcheg_vel2delay")
          {
            buildingRegion->pitcheg_veltrack.delay = value.getFloatValue();
            if((buildingRegion->pitcheg_veltrack.delay < -100.0) || (buildingRegion->pitcheg_veltrack.delay > 100.0))
            buildingRegion->pitcheg_veltrack.delay = 0.0;            
          }
          else if (opcode == "pitcheg_vel2attack")
          {
            buildingRegion->pitcheg_veltrack.attack = value.getFloatValue();
            if((buildingRegion->pitcheg_veltrack.attack < -100.0) || (buildingRegion->pitcheg_veltrack.attack > 100.0))
            buildingRegion->pitcheg_veltrack.attack = 0.0;            
          }
          else if (opcode == "pitcheg_vel2hold")
          {
            buildingRegion->pitcheg_veltrack.hold = value.getFloatValue();
            if((buildingRegion->pitcheg_veltrack.hold < -100.0) || (buildingRegion->pitcheg_veltrack.hold > 100.0))
            buildingRegion->pitcheg_veltrack.hold = 0.0;            
          }
          else if (opcode == "pitcheg_vel2decay")
          {
            buildingRegion->pitcheg_veltrack.decay = value.getFloatValue();
            if((buildingRegion->pitcheg_veltrack.decay < -100.0) || (buildingRegion->pitcheg_veltrack.decay > 100.0))
            buildingRegion->pitcheg_veltrack.decay = 0.0;            
          }
          else if (opcode == "pitcheg_vel2sustain")
          {
            buildingRegion->pitcheg_veltrack.sustain = value.getFloatValue();
            if((buildingRegion->pitcheg_veltrack.sustain < -100.0) || (buildingRegion->pitcheg_veltrack.sustain > 100.0))
            buildingRegion->pitcheg_veltrack.sustain = 0.0;            
          }
          else if (opcode == "pitcheg_vel2release")
          {
            buildingRegion->pitcheg_veltrack.release = value.getFloatValue();
            if((buildingRegion->pitcheg_veltrack.release < -100.0) || (buildingRegion->pitcheg_veltrack.release > 100.0))
            buildingRegion->pitcheg_veltrack.release = 0.0;            
          }
          else if (opcode == "pitcheg_depth")
          {
            buildingRegion->pitchegdepth = value.getFloatValue();
		    if((buildingRegion->pitchegdepth < -12000.0) || (buildingRegion->pitchegdepth > 12000.0))
		    {
		    buildingRegion->pitchegdepth = 0.0;
		    buildingRegion->pitchegdepthon = 0;
		    }
		    else
            buildingRegion->pitchegdepthon = 1;
          }                              
          else if (opcode == "fileg_delay")
          {
            buildingRegion->filtereg.delay = value.getFloatValue();
            if((buildingRegion->filtereg.delay < 0.0) || (buildingRegion->filtereg.delay > 100.0))
            buildingRegion->filtereg.delay = 0.0;            
          }
          else if (opcode == "fileg_start")
          {
            buildingRegion->filtereg.start = value.getFloatValue();
            if((buildingRegion->filtereg.start < 0.0) || (buildingRegion->filtereg.start > 100.0))
            buildingRegion->filtereg.start = 0.0;                           
          }
          else if (opcode == "fileg_attack")
          {
            buildingRegion->filtereg.attack = value.getFloatValue();
            if((buildingRegion->filtereg.attack < 0.0) || (buildingRegion->filtereg.attack > 100.0))
            buildingRegion->filtereg.attack = 0.0;                       
          }
          else if (opcode == "fileg_hold")
          {
            buildingRegion->filtereg.hold = value.getFloatValue();
            if((buildingRegion->filtereg.hold < 0.0) || (buildingRegion->filtereg.hold > 100.0))
            buildingRegion->filtereg.hold = 0.0;                         
          }
          else if (opcode == "fileg_decay")
          {
            buildingRegion->filtereg.decay = value.getFloatValue();
            if((buildingRegion->filtereg.decay < 0.0) || (buildingRegion->filtereg.decay > 100.0))
            buildingRegion->filtereg.decay = 0.0;                          
          }
          else if (opcode == "fileg_sustain")
          {
            buildingRegion->filtereg.sustain = value.getFloatValue();
            if((buildingRegion->filtereg.sustain < 0.0) || (buildingRegion->filtereg.sustain > 100.0))
            buildingRegion->filtereg.sustain = 100.0;                       
          }
          else if (opcode == "fileg_release")
          {
            buildingRegion->filtereg.release = value.getFloatValue();
            if((buildingRegion->filtereg.release < 0.0) || (buildingRegion->filtereg.release > 100.0))
            buildingRegion->filtereg.release = 0.0; 
          }
          else if (opcode == "fileg_vel2delay")
          {
            buildingRegion->filtereg_veltrack.delay = value.getFloatValue();
            if((buildingRegion->filtereg_veltrack.delay < -100.0) || (buildingRegion->filtereg_veltrack.delay > 100.0))
            buildingRegion->filtereg_veltrack.delay = 0.0;            
          }
          else if (opcode == "fileg_vel2attack")
          {
            buildingRegion->filtereg_veltrack.attack = value.getFloatValue();
            if((buildingRegion->filtereg_veltrack.attack < -100.0) || (buildingRegion->filtereg_veltrack.attack > 100.0))
            buildingRegion->filtereg_veltrack.attack = 0.0;            
          }
          else if (opcode == "fileg_vel2hold")
          {
            buildingRegion->filtereg_veltrack.hold = value.getFloatValue();
            if((buildingRegion->filtereg_veltrack.hold < -100.0) || (buildingRegion->filtereg_veltrack.hold > 100.0))
            buildingRegion->filtereg_veltrack.hold = 0.0;            
          }
          else if (opcode == "fileg_vel2decay")
          {
            buildingRegion->filtereg_veltrack.decay = value.getFloatValue();
            if((buildingRegion->filtereg_veltrack.decay < -100.0) || (buildingRegion->filtereg_veltrack.decay > 100.0))
            buildingRegion->filtereg_veltrack.decay = 0.0;            
          }
          else if (opcode == "fileg_vel2sustain")
          {
            buildingRegion->filtereg_veltrack.sustain = value.getFloatValue();
            if((buildingRegion->filtereg_veltrack.sustain < -100.0) || (buildingRegion->filtereg_veltrack.sustain > 100.0))
            buildingRegion->filtereg_veltrack.sustain = 0.0;            
          }
          else if (opcode == "fileg_vel2release")
          {
            buildingRegion->filtereg_veltrack.release = value.getFloatValue();
            if((buildingRegion->filtereg_veltrack.release < -100.0) || (buildingRegion->filtereg_veltrack.release > 100.0))
            buildingRegion->filtereg_veltrack.release = 0.0;            
          }
          else if (opcode == "fileg_depth")
          {
            buildingRegion->filteregdepth = value.getFloatValue();
            if((buildingRegion->filteregdepth < -12000.0) || (buildingRegion->filteregdepth > 12000.0))
            {
            buildingRegion->filteregdepth = 0.0;
		    buildingRegion->filteregdepthon = 0; 
		    }
            else  
            {         
            buildingRegion->filteregdepthon = 1;
            }
          }
          else if (opcode == "lopolyaft")
          {
            buildingRegion->lopolyaft = value.getIntValue();
            if(buildingRegion->lopolyaft < 0 || buildingRegion->lopolyaft > 127)
            buildingRegion->lopolyaft = 0;
            else
            buildingRegion->polyafton = 1;
          }
          else if (opcode == "hipolyaft")
          {
            buildingRegion->hipolyaft = value.getIntValue();
            if(buildingRegion->hipolyaft < 0 || buildingRegion->hipolyaft > 127)
            buildingRegion->hipolyaft = 127;  
            else
            buildingRegion->polyafton = 1;          
          } 		     	
		  else if (opcode == "cutoff_polyaft")
		  {
			buildingRegion->cutoffpoly = value.getFloatValue();  
			if(buildingRegion->cutoffpoly < -9600.0 || buildingRegion->cutoffpoly > 9600.0)
			buildingRegion->cutoffpoly = 0.0;  
            else
            buildingRegion->cutoff_polyafton = 1;                 
		  }	
          else if (opcode == "pitchlfo_depthpolyaft")
          {
            buildingRegion->pitchlfo_depthpolyval = value.getFloatValue();
            if((buildingRegion->pitchlfo_depthpolyval < -1200.0) || (buildingRegion->pitchlfo_depthpolyval > 1200.0))
            buildingRegion->pitchlfo_depthpolyon = 0;
            else
            {
            buildingRegion->pitchlfo_depthpolyon = 1;
            buildingRegion->pitchlfodepthon = 1;
            }
          }           
          else if (opcode == "pitchlfo_freqpolyaft")
          {
            buildingRegion->pitchlfo_freqpolyval = value.getFloatValue();
            if((buildingRegion->pitchlfo_freqpolyval < -200.0) || (buildingRegion->pitchlfo_freqpolyval > 200.0))
            buildingRegion->pitchlfo_freqpolyon = 0;
            else
            {
            buildingRegion->pitchlfo_freqpolyon = 1;
            buildingRegion->pitchlfofilteron = 1;
            }
          }                
         else if (opcode == "fillfo_depthpolyaft")
          {
            buildingRegion->fillfo_depthpolyval = value.getFloatValue();
            if((buildingRegion->fillfo_depthpolyval < -1200.0) || (buildingRegion->fillfo_depthpolyval > 1200.0))
            buildingRegion->fillfo_depthpolyon = 0;
            else
            {
            buildingRegion->fillfo_depthpolyon = 1;
            buildingRegion->lfodepthon = 1;
            }
          }           
          else if (opcode == "fillfo_freqpolyaft")
          {
            buildingRegion->fillfo_freqpolyval = value.getFloatValue();
            if((buildingRegion->fillfo_freqpolyval < -200.0) || (buildingRegion->fillfo_freqpolyval > 200.0))
            buildingRegion->fillfo_freqpolyon = 0;
            else
            {
            buildingRegion->fillfo_freqpolyon = 1;
            buildingRegion->lfofilteron = 1;
            }
          }    
          else if (opcode == "amplfo_depthpolyaft")
          {
            buildingRegion->amplfo_depthpolyval = value.getFloatValue();
            if((buildingRegion->amplfo_depthpolyval < -10.0) || (buildingRegion->amplfo_depthpolyval > 10.0))
            buildingRegion->amplfo_depthpolyon = 0;
            else
            {
            buildingRegion->amplfo_depthpolyon = 1;
            buildingRegion->tremlfodepthon = 1;
            }
          }       
          else if (opcode == "amplfo_freqpolyaft")
          {
            buildingRegion->amplfo_freqpolyval = value.getFloatValue();
            if((buildingRegion->amplfo_freqpolyval < -200.0) || (buildingRegion->amplfo_freqpolyval > 200.0))
            buildingRegion->amplfo_freqpolyon = 0;
            else
            {
            buildingRegion->amplfo_freqpolyon = 1;
            buildingRegion->tremlfofilteron = 1;
            }
          }                    		
          else if (opcode == "lochanaft")
          {
            buildingRegion->lochanaft = value.getIntValue();
            if(buildingRegion->lochanaft < 0 || buildingRegion->lochanaft > 127)
            buildingRegion->lochanaft = 0;
            else
            buildingRegion->chanafton = 1;
          }
          else if (opcode == "hichanaft")
          {
            buildingRegion->hichanaft = value.getIntValue();
            if(buildingRegion->hichanaft < 0 || buildingRegion->hichanaft > 127)
            buildingRegion->hichanaft = 127;   
            else
            buildingRegion->chanafton = 1;         
          } 		      	
		  else if (opcode == "cutoff_chanaft")
		  {
			buildingRegion->cutoffchan = value.getFloatValue();  
			if(buildingRegion->cutoffchan < -9600.0 || buildingRegion->cutoffchan > 9600.0)
			buildingRegion->cutoffchan = 0.0; 
            else
            buildingRegion->cutoff_chanafton = 1;                       
		  }	
          else if (opcode == "pitchlfo_depthchanaft")
          {
            buildingRegion->pitchlfo_depthchanval = value.getFloatValue();
            if((buildingRegion->pitchlfo_depthchanval < -1200.0) || (buildingRegion->pitchlfo_depthchanval > 1200.0))
            buildingRegion->pitchlfo_depthchanon = 0;
            else
            {
            buildingRegion->pitchlfo_depthchanon = 1;
            buildingRegion->pitchlfodepthon = 1;
            }
          }           
          else if (opcode == "pitchlfo_freqchanaft")
          {
            buildingRegion->pitchlfo_freqchanval = value.getFloatValue();
            if((buildingRegion->pitchlfo_freqchanval < -200.0) || (buildingRegion->pitchlfo_freqchanval > 200.0))
            buildingRegion->pitchlfo_freqchanon = 0;
            else
            {
            buildingRegion->pitchlfo_freqchanon = 1;
            buildingRegion->pitchlfofilteron = 1;
            }
          }                
         else if (opcode == "fillfo_depthchanaft")
          {
            buildingRegion->fillfo_depthchanval = value.getFloatValue();
            if((buildingRegion->fillfo_depthchanval < -1200.0) || (buildingRegion->fillfo_depthchanval > 1200.0))
            buildingRegion->fillfo_depthchanon = 0;
            else
            {
            buildingRegion->fillfo_depthchanon = 1;
            buildingRegion->lfodepthon = 1;
            }
          }           
          else if (opcode == "fillfo_freqchanaft")
          {
            buildingRegion->fillfo_freqchanval = value.getFloatValue();
            if((buildingRegion->fillfo_freqchanval < -200.0) || (buildingRegion->fillfo_freqchanval > 200.0))
            buildingRegion->fillfo_freqchanon = 0;
            else
            {
            buildingRegion->fillfo_freqchanon = 1;
            buildingRegion->lfofilteron = 1;
            }
          }    
          else if (opcode == "amplfo_depthchanaft")
          {
            buildingRegion->amplfo_depthchanval = value.getFloatValue();
            if((buildingRegion->amplfo_depthchanval < -10.0) || (buildingRegion->amplfo_depthchanval > 10.0))
            buildingRegion->amplfo_depthchanon = 0;
            else
            {
            buildingRegion->amplfo_depthchanon = 1;
            buildingRegion->tremlfodepthon = 1;
            }
          }       
          else if (opcode == "amplfo_freqchanaft")
          {
            buildingRegion->amplfo_freqchanval = value.getFloatValue();
            if((buildingRegion->amplfo_freqchanval < -200.0) || (buildingRegion->amplfo_freqchanval > 200.0))
            buildingRegion->amplfo_freqchanon = 0;
            else
            {
            buildingRegion->amplfo_freqchanon = 1;
            buildingRegion->tremlfofilteron = 1;
            }
          } 
          else if (opcode == "sw_lokey")
          {
            int swloval = value.getIntValue();
            if(swloval < 0 || swloval > 127)
            buildingRegion->sw_lokey = 0;
            else      
            {    
            buildingRegion->sw_lokey = keyValue(value);
           // printf("reader lo %d\n", buildingRegion->sw_lokey);
            }
          }  
          else if (opcode == "sw_hikey")
          {
            int swhival = value.getIntValue();
            if(swhival < 0 || swhival > 127)
            buildingRegion->sw_hikey = 127;
            else          
            {
            buildingRegion->sw_hikey = keyValue(value);
           //  printf("reader hi %d\n", buildingRegion->sw_hikey);
            }
          } 
          else if (opcode == "sw_up")
          {
            int swupval = value.getIntValue();
            if(swupval < 0 || swupval > 127)
            buildingRegion->sw_up = 0;
            else    
            {      
            buildingRegion->sw_up = keyValue(value);
            buildingRegion->swupon = 1;
            }         
          } 
          else if (opcode == "sw_down")
          {
            int swdownval = value.getIntValue();
            if(swdownval < 0 || swdownval > 127)
            buildingRegion->sw_down = 0;
            else      
            {    
            buildingRegion->sw_down = keyValue(value);
            buildingRegion->swdownon = 1;
            }
          }   
          else if (opcode == "sw_last")
          {
            int swlastval = value.getIntValue();
            if(swlastval < 0 || swlastval > 127)
            buildingRegion->sw_last = 0;
            else 
            {         
            buildingRegion->sw_last = keyValue(value);
                 //   printf("reader last %d\n", buildingRegion->sw_last);
            buildingRegion->swlaston = 1;
            }
          }  
          else if (opcode == "sw_previous")
          {
            int swprevval = value.getIntValue();
            if(swprevval < 0 || swprevval > 127)
            buildingRegion->sw_previous = 0;
            else          
            {
            buildingRegion->sw_previous = keyValue(value);
            buildingRegion->swprevon = 1;
            }
          }
          else if (opcode == "lobend")
          {
            buildingRegion->lobend = value.getIntValue();
            if((buildingRegion->lobend >= -8192) && (buildingRegion->lobend <= 8192))
            buildingRegion->pitchbendon = 1;
          }
          else if (opcode == "hibend")
          {
            buildingRegion->hibend = value.getIntValue();
            if((buildingRegion->hibend >= -8192) && (buildingRegion->hibend <= 8192))
            buildingRegion->pitchbendon = 1;
          }
          else if(opcode == "sw_vel")
          {
            buildingRegion->sw_vel = swvelValue(value); 
            buildingRegion->swvelon = 1;                    
          }                		
          else if (opcode == "default_path")
          {
            error("\"default_path\" outside of <control> tag");
          }
          else
          {
            sound_->addUnsupportedOpcode(juce::String(opcode.getStart(), opcode.length()));
          }
        }
      }

    // Skip to next element.
    nextElement:
      c = 0;
      while (p < end)
      {
        c = *p;
        if ((c != ' ') && (c != '\t'))
        {
          break;
        }
        p += 1;
      }
      if ((c == '\r') || (c == '\n'))
      {
        p = handleLineEnd(p);
        break;
      }
    }
  }

fatalError:
  if (buildingRegion && (buildingRegion == &curRegion))
  {
    finishRegion(buildingRegion);
  }
}

const char *sfzero::Reader::handleLineEnd(const char *p)
{
  // Check for DOS-style line ending.
  char lineEndChar = *p++;

  if ((lineEndChar == '\r') && (*p == '\n'))
  {
    p += 1;
  }
  line_ += 1;
  return p;
}

const char *sfzero::Reader::readPathInto(juce::String *pathOut, const char *pIn, const char *endIn)
{
  // Paths are kind of funny to parse because they can contain whitespace.
  const char *p = pIn;
  const char *end = endIn;
  const char *pathStart = p;
  const char *potentialEnd = nullptr;

  juce::String value2;
  juce::String itfirst;
  juce::String itsecond;	
  std::size_t found;
  std::string cdir;

  while (p < end)
  {
    char c = *p;
    if (c == ' ')
    {
      // Is this space part of the path?  Or the start of the next opcode?  We
      // don't know yet.
      potentialEnd = p;
      p += 1;
      // Skip any more spaces.
      while (p < end && *p == ' ')
      {
        p += 1;
      }
    }
    else if ((c == '\n') || (c == '\r') || (c == '\t'))
    {
      break;
    }
    else if (c == '=')
    {
      // We've been looking at an opcode; we need to rewind to
      // potentialEnd.
      p = potentialEnd;
      break;
    }
    p += 1;
  }
  if (p > pathStart)
  {
    // Can't do this:
    //      juce::String path(CharPointer_UTF8(pathStart), CharPointer_UTF8(p));
    // It won't compile for some unfathomable reason.
    juce::CharPointer_UTF8 end2(p);
    juce::String path(juce::CharPointer_UTF8(pathStart), end2);
    if(path.contains("$"))
    {
    cdir = path.toStdString();
    it = definemacro.begin();
    while(it != definemacro.end())
    {
    found = cdir.find(it->first);
    if (found!=std::string::npos)
    {
    itfirst = it->first;
    itsecond = it->second;	    
    value2 = path.replace(itfirst, itsecond, false);
    path = value2;
    break;
    }
    it++;
    }
    }
    *pathOut = path.trim();
  }
  else
  {
    *pathOut = juce::String();
  }
  return p;
}

int sfzero::Reader::keyValue(const juce::String &str)
{
  auto chars = str.toRawUTF8();

  char c = chars[0];

  if ((c >= '0') && (c <= '9'))
  {
    return str.getIntValue();
  }

  int note = 0;
  static const int notes[] = {
      12 + 0, 12 + 2, 3, 5, 7, 8, 10,
  };
  if ((c >= 'A') && (c <= 'G'))
  {
    note = notes[c - 'A'];
  }
  else if ((c >= 'a') && (c <= 'g'))
  {
    note = notes[c - 'a'];
  }
  int octaveStart = 1;

  c = chars[1];
  if ((c == 'b') || (c == '#'))
  {
    octaveStart += 1;
    if (c == 'b')
    {
      note -= 1;
    }
    else
    {
      note += 1;
    }
  }

  int octave = str.substring(octaveStart).getIntValue();
  // A3 == 57.
  int result = octave * 12 + note + (57 - 4 * 12);
  return result;
}

int sfzero::Reader::offbyValue(const juce::String &str)
{
  if (str == "normal")
  {
  return 1;
  }
  return 0;
}

int sfzero::Reader::fadeValue(const juce::String &str)
{
  if (str == "power")
  {
  return 1;
  }
  return 0;
}

int sfzero::Reader::swvelValue(const juce::String &str)
{
  if (str == "previous")
  {
  return 1;
  }
  return 0;
}

int sfzero::Reader::triggerValue(const juce::String &str)
{
  if (str == "release")
  {
    return sfzero::Region::release;
  }
  if (str == "first")
  {
    return sfzero::Region::first;
  }
  if (str == "legato")
  {
    return sfzero::Region::legato;
  }
  return sfzero::Region::attack;
}

int sfzero::Reader::loopModeValue(const juce::String &str)
{
  if (str == "no_loop")
  {
    return sfzero::Region::no_loop;
  }
  if (str == "one_shot")
  {
    return sfzero::Region::one_shot;
  }
  if (str == "loop_continuous")
  {
    return sfzero::Region::loop_continuous;
  }
  if (str == "loop_sustain")
  {
    return sfzero::Region::loop_sustain;
  }
  return sfzero::Region::sample_loop;
}

int sfzero::Reader::filterValue(const juce::String &str)
{
  if (str == "lpf_1p")
  {
	return sfzero::Region::lpf_1p;
  }	
  if (str == "hpf_1p")
  {
	return sfzero::Region::hpf_1p;
  }	
  if (str == "lpf_2p")
  {
	return sfzero::Region::lpf_2p;
  }	
  if (str == "hpf_2p")
  {
	return sfzero::Region::hpf_2p;
  }
  if (str == "bpf_2p")
  {
	return sfzero::Region::bpf_2p;
  }
  if (str == "brf_2p")
  {
	return sfzero::Region::brf_2p;
  }
	return sfzero::Region::notused;
}

void sfzero::Reader::finishRegion(sfzero::Region *region)
{
  sfzero::Region *newRegion = new sfzero::Region();

  *newRegion = *region;
  sound_->addRegion(newRegion);
}

void sfzero::Reader::error(const juce::String &message)
{
  juce::String fullMessage = message;

  fullMessage += " (line " + juce::String(line_) + ").";
  sound_->addError(fullMessage);
}

