#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"
#include "melatonin_inspector/melatonin_inspector.h"

//==============================================================================
class PluginEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&,
        void (*globalBypass) (int),
        void (*consoleMsg) (const char*),
        int (*GetNumRegionsOrMarkersFn) (int),
        int (*EnumProjectMarkersFn) (int, bool*, double*, double*, const char**, int*)
    );
    ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PluginProcessor& processorRef;
    std::unique_ptr<melatonin::Inspector> inspector;
    juce::TextButton inspectButton { "Inspect the UI" };
    juce::TextButton showconmsgButton { "showconmsg" };
    juce::TextButton bypassallfxButton { "bypassallfx" };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
