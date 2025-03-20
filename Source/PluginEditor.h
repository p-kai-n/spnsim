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
    juce::File& sys_file;
    juce::String& sys_edited;
    juce::String& sys_code;
    juce::String& ui_font;
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
            if (chooser.getResult().getFileName() != "") {
                juce::File temp = chooser.getResult();
                if (temp.getFileName().matchesWildcard("*.*", true) != true) {
                    temp = temp.getFullPathName() + ".spn";
                }
                temp.replaceWithText(sys_code);
                sys_file = temp;
                sys_edited = "";
                parameterChanged("sys_loadfile", 0.0f);
            }
        });
    }

    void closefile() {
        sys_file = "";
        sys_edited = "";
        sys_code = "";
        parameterChanged("sys_loadfile", 0.0f);
    }

    virtual void parameterChanged(const juce::String& parameterID, float newValue) override;

    AudioPluginAudioProcessor& processorRef;
    juce::AudioProcessorValueTreeState& valueTreeState;
    std::unique_ptr<choc::ui::WebView> chocWebView;
    std::unique_ptr<juce::Component> juceWebViewHolder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};