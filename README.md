# SFZero-X

The disk streaming version is at https://github.com/osxmidi/SFZero-X-DiskStreaming

Binaries are at https://github.com/osxmidi/SFZero-X/releases

For Bitwig, the vst3 needs Bitwig 3.2 or higher.

Linux make instructions

For Juce 5 copy the files in SFZero-X/plugin/JuceLibraryCode/modules/SFZero/sfzero/Juce5 to 
SFZero-X/plugin/JuceLibraryCode/modules/SFZero/sfzero

Place the unzipped SFZero-X folder (this download/clone) into the ~/JUCE6 folder 

There are basically 2 config options.

1: Copy the contents of the JUCE6 modules folder to the plugin/JuceLibraryCode/modules folder (for making the default Vst3 version)

or

2: Reconfigure using Projucer (the SFZero module is in the SFZero-X/plugin/JuceLibraryCode/modules folder and needs to be copied to the JUCE/modules folder and the SFZero source files are in the SFZero-X/plugin/Source folder).

Some files may then need to be edited
 
For Vst3, edit plugin/JuceLibraryCode/AppConfig.h 
 
and check that the below is entered
 
```
#ifndef    JUCE_VST3_CAN_REPLACE_VST2
#define JUCE_VST3_CAN_REPLACE_VST2 0
#endif 

```

To make with no Webkit, see the Webkit folder for manual config or choose the juce_gui_extra module in the Projucer and set 
 JUCE_WEB_BROWSER to Disabled.
 
Some libraries need to be installed

sudo apt-get -y install webkit2gtk-4.0 git pkg-config libfreetype6-dev libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev mesa-common-dev libasound2-dev freeglut3-dev libxcomposite-dev libcurl4-gnutls-dev

To make the default Vst3 version, cd into the ~/JUCE6/SFZero-X/plugin/Builds/Linux folder

make CONFIG=Release

vst3 is installed into ~/.vst3

------------

The lv2 version needs JUCE lv2 from the lv2 branch at https://github.com/lv2-porting-project/JUCE

sudo apt-get install lv2-dev

Copy the contents of the JUCE modules folder to the plugin/JuceLibraryCode/modules folder 

For Juce 5 (current JUCE lv2 version) copy the files in SFZero-X/plugin/JuceLibraryCode/modules/SFZero/sfzero/Juce5 to 
SFZero-X/plugin/JuceLibraryCode/modules/SFZero/sfzero

Unzip the SFZero-X-lv2-make.zip file in the SFZero-X/plugin/Builds/Linux/lv2 folder and copy the Makefile to the SFZero-X/plugin/Builds/Linux folder

cd into the SFZero-X/plugin/Builds/Linux folder

make CONFIG=Release

cd build

copy lv2_ttl_generator and lvmake and makelv2 (might need a chmod +x to make them executable) from the unzipped SFZero-X-lv2-make.zip

./makelv2

------------

Config options

Midi note pitch shifting can be turned off by setting an environmental variable ie

export SFZEROTR=1

When this is set, the samples play as they are with no midi note pitch shifting, which may be useful for drum samples etc.

------------

SFZero-X added opcodes

Added Round Robin seq_length and seq_position opcodes and Random hirand and lorand and Modwheel gain_ccx loccx hiccx and Channel selection lochan hichan opcodes and Sample Trigger on_loccx on_hiccx opcodes and Crossfade xfin_lo xfin_hi xfout_lo xfout_hi key and MIDI CC opcodes.

seq_length seq_position hirand and lorand, useful for randomizing and varying orchestral and drum samples etc.

gain_ccx, useful for orchestral and drum etc crescendos and diminuendos using MIDI CC.

loccx hiccx, useful for sample selection using MIDI CC.

lochan hichan useful for selecting and isolating MIDI channels.

xfin_lo xfin_hi xfout_lo xfout_hi useful for crossfading.

etc etc

------------

SFZero-X is based on a fork of the [original SFZero by Steve Folta] (https://github.com/stevefolta/SFZero) and an additional fork from https://github.com/altalogix/SFZero

