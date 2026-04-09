#include "PluginEditor.h"

PluginEditor::PluginEditor (PluginProcessor& p, GlobalBypassFunc globalBypass, ConsoleMsgFunc consoleMsg, GetNumRegionsFunc getNumRegions, EnumProjectMarkersFunc enumProjectMarkers)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    
    addAndMakeVisible (inspectButton);
    addAndMakeVisible (showconmsgButton);
    addAndMakeVisible (bypassallfxButton);

    // this chunk of code instantiates and opens the melatonin inspector
    inspectButton.onClick = [&] {
        if (!inspector)
        {
            inspector = std::make_unique<melatonin::Inspector> (*this);
            inspector->onClose = [this]() { inspector.reset(); };
        }

        inspector->setVisible (true);
    };

    showconmsgButton.onClick = [consoleMsg, getNumRegions] { juce::NullCheckedInvocation::invoke (consoleMsg, std::to_string (getNumRegions(0)).c_str()); };
    bypassallfxButton.onClick = [globalBypass] { juce::NullCheckedInvocation::invoke (globalBypass, -1); };

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    auto area = getLocalBounds();
    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    auto helloWorld = juce::String ("Hello from ") + PRODUCT_NAME_WITHOUT_VERSION + " v" VERSION + " running in " + CMAKE_BUILD_TYPE;
    g.drawText (helloWorld, area.removeFromTop (150), juce::Justification::centred, false);
}

void PluginEditor::resized()
{
    // layout the positions of your child components here
    auto area = getLocalBounds();
    area.removeFromBottom(50);
    inspectButton.setBounds (area.removeFromTop (50).reduced (3));
    showconmsgButton.setBounds (area.removeFromTop (50).reduced (3));
    bypassallfxButton.setBounds (area.removeFromTop (50).reduced (3));
}
