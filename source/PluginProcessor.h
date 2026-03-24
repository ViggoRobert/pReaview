#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#if (MSVC)
#include "ipps.h"
#endif

//JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wshadow-field-in-constructor",
//    "-Wnon-virtual-dtor")

#include <pluginterfaces/base/ftypes.h>
#include <pluginterfaces/base/funknown.h>

//JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace reaper
{
    //JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wzero-as-null-pointer-constant",
    //    "-Wunused-parameter",
    //    "-Wnon-virtual-dtor")
    //JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4100)

    using namespace Steinberg;
    using INT_PTR = juce::pointer_sized_int;
    using uint32 = Steinberg::uint32;

    //#include "../JUCE/examples/Plugins/extern/reaper_plugin_fx_embed.h"
    #include "../JUCE/examples/Plugins/extern/reaper_vst3_interfaces.h"

    //JUCE_END_IGNORE_WARNINGS_MSVC
    //JUCE_END_IGNORE_WARNINGS_GCC_LIKE
}

struct ReaperAPIListener
{
    virtual ~ReaperAPIListener() = default;
    
    virtual void setGlobalBypassFunction (void (*) (int)) = 0;
    virtual void setConsoleMsgFunction (void (*) (const char*)) = 0;
    virtual void GetNumRegionsOrMarkersFunction (int (*) (int)) = 0;
};

class ReaperAPI final : public reaper::IReaperUIEmbedInterface
{
public:
    explicit ReaperAPI (ReaperAPIListener& demo) : listener (demo) {}

    Steinberg::uint32 PLUGIN_API addRef() override { return ++refCount; }
    Steinberg::uint32 PLUGIN_API release() override { return --refCount; }

    Steinberg::tresult PLUGIN_API queryInterface (const Steinberg::TUID tuid, void** obj) override
    {
        if (std::memcmp (tuid, iid, sizeof (Steinberg::TUID)) == 0)
        {
            ++refCount;
            *obj = this;
            return Steinberg::kResultOk;
        }

        *obj = nullptr;
        return Steinberg::kNoInterface;
    }

private:
    ReaperAPIListener& listener;
    std::atomic<Steinberg::uint32> refCount { 1 };
};

class VST3Extensions final : public juce::VST3ClientExtensions
{
public:
    explicit VST3Extensions (ReaperAPIListener& l)
        : listener (l) {}
    
    void setIHostApplication (Steinberg::FUnknown* ptr) override
    {
        if (ptr == nullptr)
            return;

        void* objPtr = nullptr;
        
        if (ptr->queryInterface (reaper::IReaperHostApplication::iid, &objPtr) == Steinberg::kResultOk)
        {
            if (void* fnPtr = static_cast<reaper::IReaperHostApplication*> (objPtr)->getReaperApi ("BypassFxAllTracks"))
            {
                listener.setGlobalBypassFunction (reinterpret_cast<void (*) (int)> (fnPtr));
            }       
            
            if (void* fnPtr = static_cast<reaper::IReaperHostApplication*> (objPtr)->getReaperApi ("GetNumRegionsOrMarkers"))
            {
                listener.GetNumRegionsOrMarkersFunction (reinterpret_cast<int (*) (int)> (fnPtr));
            }

            if (void* fnPtr = static_cast<reaper::IReaperHostApplication*> (objPtr)->getReaperApi ("ShowConsoleMsg"))
            {
                listener.setConsoleMsgFunction (reinterpret_cast<void (*) (const char*)> (fnPtr));
            }
        }
    }

private:
    ReaperAPIListener& listener;
};

class PluginProcessor : public juce::AudioProcessor,
                        private ReaperAPIListener
{
public:
    PluginProcessor();
    ~PluginProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    juce::VST3ClientExtensions* getVST3ClientExtensions() override { return &vst3Extensions; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginProcessor)

    void setGlobalBypassFunction (void (*fn) (int)) override { globalBypassFn = fn; }
    void setConsoleMsgFunction (void (*fn) (const char*)) override { consoleMsgFn = fn; }
    void GetNumRegionsOrMarkersFunction (int (*fn) (int)) override { GetNumRegionsOrMarkersFn = fn; }
    
    void (*globalBypassFn) (int) = nullptr;
    void (*consoleMsgFn) (const char*) = nullptr;
    int (*GetNumRegionsOrMarkersFn) (int) = nullptr;

    VST3Extensions vst3Extensions { *this };
};
