# SFZero-X

Binaries are at https://github.com/osxmidi/SFZero-X/releases

There is also the juicysfplugin sf2/sf3 soundfont player at https://github.com/osxmidi/juicysfplugin/tree/master

----------

# Linux make instructions

Rename the unzipped JUCE folder to JUCE and move it to the home folder

Unzip this repository/clone inside the JUCE folder

To make the Projucer, change into ~/JUCE/extras/Projucer/Builds/LinuxMakefile.
Edit the Makefile and add "-DJUCER_ENABLE_GPL_MODE=1" to both the JUCE_CPPFLAGS lines.
Then enter into the Terminal, 
make CONFIG=Release

Move the Projucer binary to the main JUCE folder.

Unzip SFZero-X-master.zip and rename to SFZero-X and place in the main JUCE folder.

Run the Projucer and load the SFZero-X jucer file in the unzipped SFZero-X/plugin folder and save the project (disable JUCE_VST3_CAN_REPLACE_VST2 in the juce_audio_plugin_client module options before saving).

---------
 
Some libraries need to be installed

sudo apt-get -y install git pkg-config libfreetype6-dev libx11-dev libxinerama-dev libxrandr-dev libxcursor-dev mesa-common-dev libasound2-dev freeglut3-dev libxcomposite-dev libcurl4-gnutls-dev

(also webkit2gtk-4.0 if using webkit)

For Manjaro/EndeavourOS/Arch:
sudo pacman -Sy libx11 gcc-multilib cmake freetype2 sqlite libxcb xcb-util gtkmm3 xcb-util-cursor libx11 pkgconfig xcb-util-keysyms

To make the default Vst3 version, cd into the ~/JUCE/SFZero-X/plugin/Builds/Linux folder

make CONFIG=Release

vst3 is installed into ~/.vst3

The same basic procedure applies to the disk streaming version as well at https://github.com/osxmidi/SFZero-X/tree/master/SFZero-X-DiskStreaming

------------

For older versions of JUCE

The lv2 version needs JUCE lv2 from the lv2 branch at https://github.com/lv2-porting-project/JUCE

Install the lv2 development files.
sudo apt-get install lv2-dev or sudo pacman -Sy lv2 for Manjaro

Make the Projucer as above and save the project as a vst2 (change from vst3 to vst2 in the Projucer settings).

Unzip the SFZero-X-lv2-make.zip file in the SFZero-X/plugin/Builds/Linux/lv2 folder and copy the Makefile to the SFZero-X/plugin/Builds/Linux folder

cd into the SFZero-X/plugin/Builds/Linux folder

make CONFIG=Release

then

cd build

copy lv2_ttl_generator and lvmake and makelv2 (might need a chmod +x to make them executable) from the unzipped SFZero-X-lv2-make.zip to the build folder

./makelv2

lvmake is a renamed lv2_ttl_generator binary.

Using g++ lv2_ttl_generator.c -o lvmake should make lvmake

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

