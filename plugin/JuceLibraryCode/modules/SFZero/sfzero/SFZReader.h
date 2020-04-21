/*************************************************************************************
 * Original code copyright (C) 2012 Steve Folta
 * Converted to Juce module (C) 2016 Leo Olivers
 * Forked from https://github.com/stevefolta/SFZero
 * For license info please see the LICENSE file distributed with this source code
 *************************************************************************************/
#ifndef SFZREADER_H_INCLUDED
#define SFZREADER_H_INCLUDED

#include "SFZCommon.h"

#include <map>
#include <string>
#include <iterator>
#include <iostream>

namespace sfzero
{

struct Region;
class Sound;

class Reader
{
public:
  explicit Reader(Sound *sound);
  ~Reader();

  void read(const juce::File &file, char *includepath);
  void read(const char *text, unsigned int length);

private:
  const char *handleLineEnd(const char *p);
  const char *readPathInto(juce::String *pathOut, const char *p, const char *end);
  int keyValue(const juce::String &str);
  int triggerValue(const juce::String &str);
  int offbyValue(const juce::String &str);
  int fadeValue(const juce::String &str);
  int swvelValue(const juce::String &str);  
  int filterValue(const juce::String &str);  
  int loopModeValue(const juce::String &str);
  void finishRegion(Region *region);
  void error(const juce::String &message);

  Sound *sound_;
  int line_;
  
  int globalon;
  juce::String loadstring; 
  juce::File curdir;
  juce::File curread;

  std::map<std::string, std::string> definemacro;
  std::map<std::string, std::string>::iterator it;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Reader)
};

class StringSlice
{
public:
  StringSlice(const char *startIn, const char *endIn) : start_(startIn), end_(endIn) {}
  virtual ~StringSlice() {}

  unsigned int length() { return static_cast<int>(end_ - start_); }
  bool operator==(const char *other) { return (strncmp(start_, other, length()) == 0); }
  bool operator!=(const char *other) { return (strncmp(start_, other, length()) != 0); }
  const char *getStart() const { return start_; }
  const char *getEnd() const { return end_; }
private:
  const char *start_;
  const char *end_;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StringSlice)
};
}

#endif // SFZREADER_H_INCLUDED
