# SFZero-X

The disk streaming version is at https://github.com/osxmidi/SFZero-X-DiskStreaming

Binaries are at https://github.com/osxmidi/SFZero-X/releases

The vst3 currently has gui issues with Bitwig, Reaper is ok.

Linux make instructions

For Juce 5 copy the files in SFZero-X/plugin/JuceLibraryCode/modules/SFZero/sfzero/Juce5 to 
SFZero-X/plugin/JuceLibraryCode/modules/SFZero/sfzero

Place the unzipped SFZero-X master folder (this download/clone) into the ~/JUCE6 folder 

There are basically 2 config options.

1: Copy the contents of the JUCE6 modules folder to the plugin/JuceLibraryCode/modules folder (for making the default Vst3 version)

or

2: Reconfigure using Projucer (the SFZero module is in the plugin/JuceLibraryCode/modules folder and the SFZero source files are in the plugin/Source folder).


Some files may then need to be edited
 
For Vst3, edit plugin/JuceLibraryCode/AppConfig.h 
 
and check that the below is entered
 
```
#ifndef    JUCE_VST3_CAN_REPLACE_VST2
#define JUCE_VST3_CAN_REPLACE_VST2 0
#endif 

```
 
Some libraries need to be installed

sudo apt-get -y install webkit2gtk-4.0 git pkg-config libfreetype6-dev libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev mesa-common-dev libasound2-dev freeglut3-dev libxcomposite-dev libcurl4-gnutls-dev

To make the default Vst3 version, cd into the ~/JUCE6/unzipped SFZero-X master folder/plugin/Builds/Linux folder

make CONFIG=Release

vst3 is installed into ~/.vst3

------------

# SFZero, the Juce module version

This is a fork of the [original SFZero by Steve Folta](https://github.com/stevefolta/SFZero), with the following changes:

* has been converted to a Juce module, so you can easily consume it from your own projects (you still get the sample player plugin, but it now includes that module)
* requires Juce 4.1 or higher
* supports Juce 4.1 module format
* now also supports new Juce 4.2 module format (thanks to Loki Davison)
* conveniently sits within its own `sfzero::` namespace
* has a tidied-up code base, so it now builds with as few warnings as possible on all platforms and on both 32/64 bit architectures. I also simplified logging, added support for synchronous sample loading, and fixed a few bugs.
* the SFZero Juce module and sample plugin have been separated and the Juce module is now available as a [git submodule](https://github.com/altalogix/SFZeroModule) for easy inclusion in other repositories

For more information, please see also this [blog article](http://www.mucoder.net/blog/2016/03/24/sfzero.html)

Please note that, in order to build, SFZero requires [Juce](http://www.juce.com).

Before building the sample plugin, it's necessary to

* get the sample plugin source code from [https://github.com/altalogix/SFZero](https://github.com/altalogix/SFZero)
* get the module source code from [https://github.com/altalogix/SFZeroModule](https://github.com/altalogix/SFZeroModule)
* copy the SFZeroModule folder as a childfolder to your Juce modules folder.
* load `plugin/SFZero.jucer` into your IntroJucer tool and save the project again. This should regenerate the project build definitions with the proper links to your Juce module location.

If you just want to use the Juce module and not the sample plugin, it suffices to include the contents of [https://github.com/altalogix/SFZeroModule](https://github.com/altalogix/SFZeroModule) within a SFZero child folder of your Juce modules folder.


