# SFZero-X

The SFZero-X Disk Streaming version is at https://github.com/osxmidi/SFZero-X/tree/SFZero-X-DiskStreaming

Linux make instructions

Some libraries need to be installed

sudo apt-get -y install webkit2gtk-4.0 git pkg-config libfreetype6-dev libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev mesa-common-dev libasound2-dev freeglut3-dev libxcomposite-dev libcurl4-gnutls-dev

Install JUCE into ~/JUCE

Enable GPL mode

Edit ~/JUCE/extras/Projucer/JuceLibraryCode/AppConfig.h

and change #define JUCER_ENABLE_GPL_MODE to #define JUCER_ENABLE_GPL_MODE 1

Place the unzipped SFZero module folder https://github.com/osxmidi/SFZero-X-Module into the ~/JUCE/modules folder and rename the folder to SFZero

Place the unzipped SFZero-master folder (this download/clone) into the ~/JUCE folder

Place the vst2.4 sdk's pluginterfaces folder into ~/JUCE/modules/juce_audio_plugin_client/VST/

cd into ~/JUCE/Unzipped SFZero-master folder/plugin/Builds/Linux

make CONFIG=Release

Binaries are produced in the ~/JUCE/Unzipped SFZero-master folder/plugin/Builds/Linux/build folder

------------

The above instructions also apply to the SFZero-X-DiskStreaming branch except the makefile folder is ~/JUCE/unzipped SFZero-X Disk Streaming master folder/plugin/Builds/LinuxMakefile.

------------

To make without webkit dependencies

use Makefile-nowebkit (rename Makefile-nowebkit to Makefile and then make CONFIG=Release)

Some edits need to be made in 2 files

--------

juce_gui_extra.h in

/JUCE/modules/juce_gui_extra/

change

#ifndef JUCE_WEB_BROWSER

#define JUCE_WEB_BROWSER 1

#endif

to

#ifndef JUCE_WEB_BROWSER

// #define JUCE_WEB_BROWSER 1

#endif

--------

juce_ApplicationBase.cpp in

/JUCE/modules/juce_events/messages/

change

#if JUCE_LINUX && JUCE_MODULE_AVAILABLE_juce_gui_extra && (! defined(JUCE_WEB_BROWSER) || JUCE_WEB_BROWSER)

to

#if JUCE_LINUX && JUCE_MODULE_AVAILABLE_juce_gui_extra && JUCE_WEB_BROWSER

(there are 2 occurrences of it that need to be changed in the juce_ApplicationBase.cpp file)

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


