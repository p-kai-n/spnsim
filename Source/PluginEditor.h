#pragma once

#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_data_structures/juce_data_structures.h>

#if JUCE_LINUX
// Include it here because of needed to access the GTK API from CHOC.
#include <gtk/gtk.h>
#include <gtk/gtkx.h>
#include <glib-unix.h>
#include <webkit2/webkit2.h>
#endif

#include <gui/choc_WebView.h>

//==============================================================================
class AudioPluginAudioProcessorEditor final
    : public juce::AudioProcessorEditor
    , private juce::AudioProcessorValueTreeState::Listener
    , private juce::ValueTree::Listener
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&,
        float& w0a, float& w0i, float& w1a, float& w1i, float& w2a, float& w2i, float& w3a, float& w3i,
        juce::String&, juce::File&, juce::String&, unsigned __int8&, juce::String&, std::array<int, 384>&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


private:
    //==============================================================================
    juce::String& ui_font;
    juce::File& sys_file;
    juce::String& sys_edited;
    juce::String& sys_code;
    unsigned __int8& sys_running;
    std::array<int, 384>& sim_op;

    float& sys_wf0_max;
    float& sys_wf0_min;
    float& sys_wf1_max;
    float& sys_wf1_min;
    float& sys_wf2_max;
    float& sys_wf2_min;
    float& sys_wf3_max;
    float& sys_wf3_min;

    std::unique_ptr <juce::FileChooser> openDialog;
    void openfile() {
        openDialog = std::make_unique<juce::FileChooser>("Open", sys_file, "*.spn;*.txt");
        auto folderChooserFlags = juce::FileBrowserComponent::openMode;
        openDialog->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser) {
            if (chooser.getResult().getFileName() != "") {
                sys_file = chooser.getResult();
                sys_edited = "";
                sys_code = sys_file.loadFileAsString();
                parameterChanged("sys_loadfile", 0.0f);
            }
        });
    }

    void savefile() {
        sys_file.replaceWithText(sys_code);
        sys_edited = "";
        parameterChanged("sys_loadfile", 0.0f);
    }

    std::unique_ptr <juce::FileChooser> saveDialog;
    void saveasfile() {
        saveDialog = std::make_unique<juce::FileChooser>("Save As", sys_file, "*.spn;*.txt");
        auto folderChooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::warnAboutOverwriting;
        saveDialog->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser) {
            juce::File testfile = chooser.getResult();
            if (testfile.getFileName().matchesWildcard("*.*", true) != true) {
                testfile = testfile.getFullPathName() + ".spn";
            }
            testfile.replaceWithText(sys_code);
            sys_file = chooser.getResult();
            sys_edited = "";
            parameterChanged("sys_loadfile", 0.0f);
        });
    }

    void closefile() {
        sys_file = "";
        sys_edited = "";
        sys_code = "";
        parameterChanged("sys_loadfile", 0.0f);
    }

    virtual void parameterChanged(const juce::String& parameterID, float newValue) override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;
    /*
    typedef juce::AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef juce::AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    */
    juce::AudioProcessorValueTreeState& valueTreeState;
    /*
    juce::Slider sys_ingainSlider;
    std::unique_ptr<SliderAttachment> sys_ingainAttachment;
    juce::Slider sys_outgainSlider;
    std::unique_ptr<SliderAttachment> sys_outgainAttachment;

    juce::Slider pot0_ctrlSlider;
    std::unique_ptr<SliderAttachment> pot0_ctrlAttachment;
    juce::Slider pot1_ctrlSlider;
    std::unique_ptr<SliderAttachment> pot1_ctrlAttachment;
    juce::Slider pot2_ctrlSlider;
    std::unique_ptr<SliderAttachment> pot2_ctrlAttachment;
    juce::Slider pot3_ctrlSlider;
    std::unique_ptr<SliderAttachment> pot3_ctrlAttachment;

    juce::Slider sys_bypassSlider;
    std::unique_ptr<SliderAttachment> sys_bypassAttachment;
    juce::Slider sys_muteSlider;
    std::unique_ptr<SliderAttachment> sys_muteAttachment;

    juce::Slider sys_wf_sendSlider;
    std::unique_ptr<SliderAttachment> sys_wf_sendAttachment;
 
    //juce::ToggleButton invertButton;
    //std::unique_ptr<ButtonAttachment> invertAttachment;
    */
    std::unique_ptr<choc::ui::WebView> chocWebView;
    std::unique_ptr<juce::Component> juceWebViewHolder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
