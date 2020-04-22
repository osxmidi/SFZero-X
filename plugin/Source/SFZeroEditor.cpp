#include "SFZeroEditor.h"
#include "SFZeroAudioProcessor.h"

enum
{
  hMargin = 12,
  vMargin = 12,
  labelHeight = 25,
  progressBarHeight = 30,
  keyboardHeight = 54,
};

sfzero::SFZeroEditor::SFZeroEditor(SFZeroAudioProcessor *ownerFilter)
    : AudioProcessorEditor(ownerFilter), fileLabel(String(), "File... (click here to choose)"), pathLabel(String()),
      showingInfo(showingSoundInfo), midiKeyboard(ownerFilter->keyboardState, MidiKeyboardComponent::horizontalKeyboard),
      progressBar(nullptr)
{
  setSize(500, 300);

#ifdef JUCE_MAC
  Font fileFont("Helvetica", 22.0, Font::bold);
  Font labelFont("Helvetica", 15.0, Font::plain);
#else
  Font fileFont("Ariel", 22.0, Font::bold);
  Font labelFont("Ariel", 15.0, Font::plain);
#endif

  addAndMakeVisible(&fileLabel);
  fileLabel.setFont(fileFont);
  fileLabel.setColour(Label::textColourId, Colours::grey);
  fileLabel.addClickListener(this);

  addAndMakeVisible(&pathLabel);
  pathLabel.setFont(labelFont);
  pathLabel.setColour(Label::textColourId, Colours::grey);
  pathLabel.addClickListener(this);

  addAndMakeVisible(&viewport);
  viewport.setScrollBarsShown(true, true);
  viewport.setViewedComponent(&infoLabel, false);
  infoLabel.setFont(labelFont);
  infoLabel.setJustificationType(Justification::topLeft);
  infoLabel.addClickListener(this);
  
  addAndMakeVisible(&midiKeyboard);
  midiKeyboard.setOctaveForMiddleC(4);

  startTimer(200);

  File sfzFile = ownerFilter->getSfzFile();
  if (sfzFile != File())
  {
    updateFile(&sfzFile);
    showSoundInfo();
    auto sound = ownerFilter->getSound();
    if (sound && (sound->numSubsounds() > 1))
    {
      showSubsound();
    }
  }
  else
  {
    showVersion();
  }
}

sfzero::SFZeroEditor::~SFZeroEditor() { delete progressBar; }

void sfzero::SFZeroEditor::paint(Graphics &g) { g.fillAll(Colours::white); }

void sfzero::SFZeroEditor::resized()
{
  int marginedWidth = getWidth() - 2 * hMargin;

  fileLabel.setBounds(hMargin, vMargin, marginedWidth, labelHeight);
  pathLabel.setBounds(hMargin, vMargin + labelHeight, marginedWidth, labelHeight);
  int infoTop = vMargin + 2 * labelHeight;
  int keyboardTop = getHeight() - keyboardHeight - vMargin;
  int infoLabelHeight = keyboardTop - infoTop - 4;
  viewport.setBounds(hMargin, infoTop, marginedWidth, infoLabelHeight);
  infoLabel.setBounds(0, 0, marginedWidth, infoLabelHeight * 10);
  midiKeyboard.setBounds(hMargin, keyboardTop, marginedWidth, keyboardHeight);
}

void sfzero::SFZeroEditor::labelClicked(Label *clickedLabel)
{
  if (clickedLabel == &fileLabel)
  {
    chooseFile();
  }
  else if (clickedLabel == &pathLabel)
  {
    if (showing == showingSubsound)
    {
      auto processor = getProcessor();
      auto sound = processor->getSound();
      if (sound)
      {
        PopupMenu menu;
        int selectedSubsound = sound->selectedSubsound();
        int numSubsounds = sound->numSubsounds();
        for (int i = 0; i < numSubsounds; ++i)
        {
          menu.addItem(i + 1, sound->subsoundName(i), true, (i == selectedSubsound));
        }
        int result = menu.show();
        if (result != 0)
        {
          sound->useSubsound(result - 1);
          showSubsound();
        }
      }
    }
    else if (showing == showingVersion)
    {
      showPath();
    }
    else
    {
      showVersion();
    }
  }
  else if (clickedLabel == &infoLabel)
  {
    if (showingInfo == showingSoundInfo)
    {
      showVoiceInfo();
    }
    else
    {
      showSoundInfo();
    }
  }
}

void sfzero::SFZeroEditor::timerCallback()
{
  if (showing == showingProgress)
  {
    auto processor = getProcessor();
    if (processor->loadProgress >= 1.0)
    {
      auto sound = processor->getSound();
      if (sound && (sound->numSubsounds() > 1))
      {
        showSubsound();
      }
      else
      {
        showPath();
      }
      showSoundInfo();
    }
  }

  if (showingInfo == showingVoiceInfo)
  {
    showVoiceInfo();
  }
}

/*
void sfzero::SFZeroEditor::chooseFile()
{
  FileChooser chooser("Select an SFZ file...", File(), "*.sfz;*.SFZ;*.sf2;*.SF2", false, false);

  if (chooser.browseForFileToOpen())
  {
    File sfzFile(chooser.getResult());
    setFile(&sfzFile);
  }
}
*/

void sfzero::SFZeroEditor::chooseFile()
{
String loadstring = "";

  File sfile = juce::File::getSpecialLocation(File::userHomeDirectory).getChildFile(".sfzero-x");

  if (!sfile.existsAsFile())
  {
  auto result = sfile.create();   
  if(!result.wasOk() )
  {
  DBG( "create file error");
  jassertfalse;
  }
  }
  else
  loadstring = sfile.loadFileAsString();

  FileChooser chooser("Select an SFZ file...", File(loadstring), "*.sfz;*.SFZ;*.sf2;*.SF2");

  if (chooser.browseForFileToOpen())
  {
  File sfzFile(chooser.getResult());

  loadstring = sfzFile.getFullPathName();
  loadstring = loadstring.upToLastOccurrenceOf("/", false, false);
 // std::string cdir = loadstring.toStdString();

  if (sfile.existsAsFile())
  sfile.replaceWithText(loadstring);

  setFile(&sfzFile);
  }
}

void sfzero::SFZeroEditor::setFile(File *newFile)
{
  auto processor = getProcessor();

  processor->setSfzFileThreaded(newFile);

  updateFile(newFile);
  showProgress();
}

void sfzero::SFZeroEditor::updateFile(File *file)
{
  fileLabel.setText(file->getFileName(), dontSendNotification);
  fileLabel.setColour(Label::textColourId, Colours::black);
  showPath();
}

void sfzero::SFZeroEditor::showSoundInfo()
{
  auto processor = getProcessor();
  auto sound = processor->getSound();

  if (sound)
  {
      String info;
      auto& errors = sound->getErrors();
      if (errors.size() > 0)
      {
          info << errors.size() << " errors: \n";
          info << errors.joinIntoString("\n");
          info << "\n";
      }
      else
      {
          info << "no errors.\n\n";
      }
      auto& warnings = sound->getWarnings();
      if (warnings.size() > 0)
      {
          info << warnings.size() << " warnings: \n";
          info << warnings.joinIntoString("\n");
      }
      else
      {
          info << "no warnings.\n";
      }
      infoLabel.setText(info, dontSendNotification);
  }
  showingInfo = showingSoundInfo;
}

void sfzero::SFZeroEditor::showVoiceInfo()
{
  auto processor = getProcessor();

  infoLabel.setText(processor->voiceInfoString(), dontSendNotification);
  showingInfo = showingVoiceInfo;
}

void sfzero::SFZeroEditor::showVersion()
{
  auto date = Time::getCompilationDate();
  auto str = String::formatted("SFZero-X beta %d.%d.%d", date.getYear(), date.getMonth(), date.getDayOfMonth());
  pathLabel.setText(str, dontSendNotification);
  pathLabel.setColour(Label::textColourId, Colours::grey);
  hideProgress();
  showing = showingVersion;
}

void sfzero::SFZeroEditor::showPath()
{
  auto processor = getProcessor();
  File file = processor->getSfzFile();

  pathLabel.setText(file.getParentDirectory().getFullPathName(), dontSendNotification);
  pathLabel.setColour(Label::textColourId, Colours::grey);
  hideProgress();
  showing = showingPath;
}

void sfzero::SFZeroEditor::showSubsound()
{
  auto processor = getProcessor();
  auto sound = processor->getSound();

  if (sound == nullptr)
  {
    return;
  }

  pathLabel.setText(sound->subsoundName(sound->selectedSubsound()), dontSendNotification);
  pathLabel.setColour(Label::textColourId, Colours::black);
  hideProgress();
  showing = showingSubsound;
}

void sfzero::SFZeroEditor::showProgress()
{
  auto processor = getProcessor();

  pathLabel.setVisible(false);
  infoLabel.setVisible(false);
  progressBar = new ProgressBar(processor->loadProgress);
  addAndMakeVisible(progressBar);
  int marginedWidth = getWidth() - 2 * hMargin;
  progressBar->setBounds(hMargin, vMargin + labelHeight, marginedWidth, progressBarHeight);
  showing = showingProgress;
}

void sfzero::SFZeroEditor::hideProgress()
{
  if (progressBar == nullptr)
  {
    return;
  }

  removeChildComponent(progressBar);
  delete progressBar;
  progressBar = nullptr;

  pathLabel.setVisible(true);
  infoLabel.setVisible(true);
}
