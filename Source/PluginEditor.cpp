#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "../JuceLibraryCode/JuceHeader.h"


#ifndef WEB_VIEW_FROM_SERVER
    #define WEB_VIEW_FROM_SERVER 0
#endif

//==============================================================================
namespace
{
    std::string getMimeType(std::string const& ext)
    {
        static std::unordered_map<std::string, std::string> mimeTypes{
          { ".html", "text/html" },
          { ".js", "application/javascript" },
          { ".css", "text/css" },
          { ".json", "application/json"},
          { ".svg", "image/svg+xml"},
          { ".svgz", "image/svg+xml"},
        };

        if (mimeTypes.count(ext) > 0)
        {
            return mimeTypes.at(ext);
        }

        return "application/octet-stream";
    }

    enum
    {
        paramControlHeight = 40,
        paramLabelWidth    = 80,
        paramSliderWidth   = 300
    };
}

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p, float& w0a, float& w0i, float& w1a, float& w1i, float& w2a, float& w2i, float& w3a, float& w3i,
    juce::String& font, juce::File& f, juce::String& e, unsigned __int8& r, juce::String& c, std::array<int, 384>& op)

    : AudioProcessorEditor(&p), processorRef(p), valueTreeState(p.getAPVTS()),
    sys_wf0_max(w0a), sys_wf0_min(w0i), sys_wf1_max(w1a), sys_wf1_min(w1i), sys_wf2_max(w2a), sys_wf2_min(w2i), sys_wf3_max(w3a), sys_wf3_min(w3i),
    ui_font(font), sys_file(f), sys_edited(e), sys_running(r), sys_code(c), sim_op(op)
{
    juce::ignoreUnused (processorRef);
    choc::ui::WebView::Options options;

#if JUCE_DEBUG
    options.enableDebugMode = true;
#else
    options.enableDebugMode = false;
#endif

    chocWebView = std::make_unique<choc::ui::WebView> (options);

    auto web_view_callback_on_toggle_changed =
        [safe_this = juce::Component::SafePointer(this)](const choc::value::ValueView& args) -> choc::value::Value {
        if (safe_this.getComponent() == nullptr) {return choc::value::Value(-1);}
        const auto choc_json_string = choc::json::toString(args);
        const auto juce_json = juce::JSON::parse(choc_json_string);
        const float juce_value = juce_json[0]["toggleValue"];

        if (juce_json[0]["toggleName"] == "sys_bypass") {
            safe_this->valueTreeState.getParameter("sys_bypass")->setValueNotifyingHost(juce_value);
        }
        else if (juce_json[0]["toggleName"] == "sys_mute") {
            safe_this->valueTreeState.getParameter("sys_mute")->setValueNotifyingHost(juce_value);
        }

        return choc::value::Value(0);
    };

    auto web_view_callback_on_sliider_changed =
        [safe_this = juce::Component::SafePointer(this)](const choc::value::ValueView& args) -> choc::value::Value {
        if (safe_this.getComponent() == nullptr) {return choc::value::Value(-1);}
        const auto choc_json_string = choc::json::toString(args);
        const auto juce_json = juce::JSON::parse(choc_json_string);

        if (juce_json[0]["sliderName"] == "sys_ingain") {
            safe_this->valueTreeState.getParameter("sys_ingain")->setValueNotifyingHost((float)juce_json[0]["sliderValue"] * 0.01f);
        }
        else if (juce_json[0]["sliderName"] == "sys_outgain"){
            safe_this->valueTreeState.getParameter("sys_outgain")->setValueNotifyingHost((float)juce_json[0]["sliderValue"] * 0.01f);
        }
        else if (juce_json[0]["sliderName"] == "pot0_ctrl") {
            safe_this->valueTreeState.getParameter("pot0_ctrl")->setValueNotifyingHost((float)juce_json[0]["sliderValue"]);
        }
        else if (juce_json[0]["sliderName"] == "pot1_ctrl") {
            safe_this->valueTreeState.getParameter("pot1_ctrl")->setValueNotifyingHost((float)juce_json[0]["sliderValue"]);
        }
        else if (juce_json[0]["sliderName"] == "pot2_ctrl") {
            safe_this->valueTreeState.getParameter("pot2_ctrl")->setValueNotifyingHost((float)juce_json[0]["sliderValue"]);
        }
        else if (juce_json[0]["sliderName"] == "pot3_ctrl") {
            safe_this->valueTreeState.getParameter("pot3_ctrl")->setValueNotifyingHost((float)juce_json[0]["sliderValue"]);
        }

        return choc::value::Value(0);
    };

    auto web_view_callback_on_select_changed =
        [safe_this = juce::Component::SafePointer(this)](const choc::value::ValueView& args) -> choc::value::Value {
        if (safe_this.getComponent() == nullptr) { return choc::value::Value(-1); }
        const auto choc_json_string = choc::json::toString(args);
        const auto juce_json = juce::JSON::parse(choc_json_string);

        if (juce_json[0]["selectName"] == "reg_sel0") {
            safe_this->valueTreeState.getParameter("sys_wf0_sel")->setValueNotifyingHost((float)juce_json[0]["selectValue"] * 0.01f);
        }
        else if (juce_json[0]["selectName"] == "reg_sel1") {
            safe_this->valueTreeState.getParameter("sys_wf1_sel")->setValueNotifyingHost((float)juce_json[0]["selectValue"] * 0.01f);
        }
        else if (juce_json[0]["selectName"] == "reg_sel2") {
            safe_this->valueTreeState.getParameter("sys_wf2_sel")->setValueNotifyingHost((float)juce_json[0]["selectValue"] * 0.01f);
        }
        else if (juce_json[0]["selectName"] == "reg_sel3") {
            safe_this->valueTreeState.getParameter("sys_wf3_sel")->setValueNotifyingHost((float)juce_json[0]["selectValue"] * 0.01f);
        }
        else if (juce_json[0]["selectName"] == "knob_name") {
            safe_this->valueTreeState.getParameter("knob_name")->setValueNotifyingHost((float)juce_json[0]["selectValue"] * 0.01f);
        }

        return choc::value::Value(0);
        };
    
    auto web_view_callback_on_text_changed =
        [safe_this = juce::Component::SafePointer(this)](const choc::value::ValueView& args) -> choc::value::Value {
        if (safe_this.getComponent() == nullptr) {return choc::value::Value(-1);}
        const auto choc_json_string = choc::json::toString(args);
        const auto juce_json = juce::JSON::parse(choc_json_string);

        const juce::String text = juce_json[0]["textContent"];

        if (juce_json[0]["textName"] == "edited") {
            safe_this->sys_edited = text;
        }
        else if (juce_json[0]["textName"] == "code") {
            safe_this->sys_code = text;
        }
        else if (juce_json[0]["textName"] == "ui_font") {
            safe_this->ui_font = text;
        }

        return choc::value::Value(0);
    };
    
    auto web_view_callback_on_called =
        [safe_this = juce::Component::SafePointer(this)](const choc::value::ValueView& args) -> choc::value::Value {
        if (safe_this.getComponent() == nullptr) {return choc::value::Value(-1);}
        const auto choc_json_string = choc::json::toString(args);
        const auto juce_json = juce::JSON::parse(choc_json_string);

        const float a = juce_json[0]["callArg"];

        if (juce_json[0]["callName"] == "open") {
            safe_this->openfile();
        }
        else if (juce_json[0]["callName"] == "save") {
            safe_this->savefile();
        }
        else if (juce_json[0]["callName"] == "saveas") {
            safe_this->saveasfile();
        }
        else if (juce_json[0]["callName"] == "close") {
            safe_this->closefile();
        }
        else if (juce_json[0]["callName"] == "ui_size") {
            if (a == 3.0f) {
                safe_this->valueTreeState.getParameter("ui_size")->setValueNotifyingHost(3.0f * 0.1f);
                safe_this->setSize(640, 360);
            } else if (a == 7.0f) {
                safe_this->valueTreeState.getParameter("ui_size")->setValueNotifyingHost(7.0f * 0.1f);
                safe_this->setSize(1280, 720);
            } else {
                safe_this->valueTreeState.getParameter("ui_size")->setValueNotifyingHost(1.0f * 0.1f);
                safe_this->setSize(1920, 1080);
            }
        }
        else if (juce_json[0]["callName"] == "ui_dark") {
            if (a == 1.0f) {
                safe_this->valueTreeState.getParameter("ui_dark")->setValueNotifyingHost(1.0f);
            } else {
                safe_this->valueTreeState.getParameter("ui_dark")->setValueNotifyingHost(0.0f);
            }
        }
        else if (juce_json[0]["callName"] == "ui_lh") {
            safe_this->valueTreeState.getParameter("ui_lh")->setValueNotifyingHost(a * 0.01f);
        }
        else if (juce_json[0]["callName"] == "ui_fs") {
            safe_this->valueTreeState.getParameter("ui_fs")->setValueNotifyingHost(a * 0.01f);
        }
        else if (juce_json[0]["callName"] == "sys_rs") {
            if (a == 1.0f) {
                safe_this->valueTreeState.getParameter("sys_rs")->setValueNotifyingHost(1.0f);
            } else {
                safe_this->valueTreeState.getParameter("sys_rs")->setValueNotifyingHost(0.0f);
            }
        }
        else if (juce_json[0]["callName"] == "running") {
            if (a == 2.0f) {
                safe_this->sys_running = 2;
            } else {
                safe_this->sys_running = 0;
            }
        }

        return choc::value::Value(0);
    };

    auto web_view_callback_on_op_changed =
        [safe_this = juce::Component::SafePointer(this)](const choc::value::ValueView& args) -> choc::value::Value {
        if (safe_this.getComponent() == nullptr) { return choc::value::Value(-1); }
        const auto choc_json_string = choc::json::toString(args);
        const auto juce_json = juce::JSON::parse(choc_json_string);

        safe_this->sys_running = 2;
        safe_this->sim_op =
        {
            int(juce_json[0]["op000_0"]), int(juce_json[0]["op000_1"]), int(juce_json[0]["op000_2"]),
            int(juce_json[0]["op001_0"]), int(juce_json[0]["op001_1"]), int(juce_json[0]["op001_2"]),
            int(juce_json[0]["op002_0"]), int(juce_json[0]["op002_1"]), int(juce_json[0]["op002_2"]),
            int(juce_json[0]["op003_0"]), int(juce_json[0]["op003_1"]), int(juce_json[0]["op003_2"]),
            int(juce_json[0]["op004_0"]), int(juce_json[0]["op004_1"]), int(juce_json[0]["op004_2"]),
            int(juce_json[0]["op005_0"]), int(juce_json[0]["op005_1"]), int(juce_json[0]["op005_2"]),
            int(juce_json[0]["op006_0"]), int(juce_json[0]["op006_1"]), int(juce_json[0]["op006_2"]),
            int(juce_json[0]["op007_0"]), int(juce_json[0]["op007_1"]), int(juce_json[0]["op007_2"]),
            int(juce_json[0]["op008_0"]), int(juce_json[0]["op008_1"]), int(juce_json[0]["op008_2"]),
            int(juce_json[0]["op009_0"]), int(juce_json[0]["op009_1"]), int(juce_json[0]["op009_2"]),
            int(juce_json[0]["op010_0"]), int(juce_json[0]["op010_1"]), int(juce_json[0]["op010_2"]),
            int(juce_json[0]["op011_0"]), int(juce_json[0]["op011_1"]), int(juce_json[0]["op011_2"]),
            int(juce_json[0]["op012_0"]), int(juce_json[0]["op012_1"]), int(juce_json[0]["op012_2"]),
            int(juce_json[0]["op013_0"]), int(juce_json[0]["op013_1"]), int(juce_json[0]["op013_2"]),
            int(juce_json[0]["op014_0"]), int(juce_json[0]["op014_1"]), int(juce_json[0]["op014_2"]),
            int(juce_json[0]["op015_0"]), int(juce_json[0]["op015_1"]), int(juce_json[0]["op015_2"]),

            int(juce_json[0]["op016_0"]), int(juce_json[0]["op016_1"]), int(juce_json[0]["op016_2"]),
            int(juce_json[0]["op017_0"]), int(juce_json[0]["op017_1"]), int(juce_json[0]["op017_2"]),
            int(juce_json[0]["op018_0"]), int(juce_json[0]["op018_1"]), int(juce_json[0]["op018_2"]),
            int(juce_json[0]["op019_0"]), int(juce_json[0]["op019_1"]), int(juce_json[0]["op019_2"]),
            int(juce_json[0]["op020_0"]), int(juce_json[0]["op020_1"]), int(juce_json[0]["op020_2"]),
            int(juce_json[0]["op021_0"]), int(juce_json[0]["op021_1"]), int(juce_json[0]["op021_2"]),
            int(juce_json[0]["op022_0"]), int(juce_json[0]["op022_1"]), int(juce_json[0]["op022_2"]),
            int(juce_json[0]["op023_0"]), int(juce_json[0]["op023_1"]), int(juce_json[0]["op023_2"]),
            int(juce_json[0]["op024_0"]), int(juce_json[0]["op024_1"]), int(juce_json[0]["op024_2"]),
            int(juce_json[0]["op025_0"]), int(juce_json[0]["op025_1"]), int(juce_json[0]["op025_2"]),
            int(juce_json[0]["op026_0"]), int(juce_json[0]["op026_1"]), int(juce_json[0]["op026_2"]),
            int(juce_json[0]["op027_0"]), int(juce_json[0]["op027_1"]), int(juce_json[0]["op027_2"]),
            int(juce_json[0]["op028_0"]), int(juce_json[0]["op028_1"]), int(juce_json[0]["op028_2"]),
            int(juce_json[0]["op029_0"]), int(juce_json[0]["op029_1"]), int(juce_json[0]["op029_2"]),
            int(juce_json[0]["op030_0"]), int(juce_json[0]["op030_1"]), int(juce_json[0]["op030_2"]),
            int(juce_json[0]["op031_0"]), int(juce_json[0]["op031_1"]), int(juce_json[0]["op031_2"]),

            int(juce_json[0]["op032_0"]), int(juce_json[0]["op032_1"]), int(juce_json[0]["op032_2"]),
            int(juce_json[0]["op033_0"]), int(juce_json[0]["op033_1"]), int(juce_json[0]["op033_2"]),
            int(juce_json[0]["op034_0"]), int(juce_json[0]["op034_1"]), int(juce_json[0]["op034_2"]),
            int(juce_json[0]["op035_0"]), int(juce_json[0]["op035_1"]), int(juce_json[0]["op035_2"]),
            int(juce_json[0]["op036_0"]), int(juce_json[0]["op036_1"]), int(juce_json[0]["op036_2"]),
            int(juce_json[0]["op037_0"]), int(juce_json[0]["op037_1"]), int(juce_json[0]["op037_2"]),
            int(juce_json[0]["op038_0"]), int(juce_json[0]["op038_1"]), int(juce_json[0]["op038_2"]),
            int(juce_json[0]["op039_0"]), int(juce_json[0]["op039_1"]), int(juce_json[0]["op039_2"]),
            int(juce_json[0]["op040_0"]), int(juce_json[0]["op040_1"]), int(juce_json[0]["op040_2"]),
            int(juce_json[0]["op041_0"]), int(juce_json[0]["op041_1"]), int(juce_json[0]["op041_2"]),
            int(juce_json[0]["op042_0"]), int(juce_json[0]["op042_1"]), int(juce_json[0]["op042_2"]),
            int(juce_json[0]["op043_0"]), int(juce_json[0]["op043_1"]), int(juce_json[0]["op043_2"]),
            int(juce_json[0]["op044_0"]), int(juce_json[0]["op044_1"]), int(juce_json[0]["op044_2"]),
            int(juce_json[0]["op045_0"]), int(juce_json[0]["op045_1"]), int(juce_json[0]["op045_2"]),
            int(juce_json[0]["op046_0"]), int(juce_json[0]["op046_1"]), int(juce_json[0]["op046_2"]),
            int(juce_json[0]["op047_0"]), int(juce_json[0]["op047_1"]), int(juce_json[0]["op047_2"]),

            int(juce_json[0]["op048_0"]), int(juce_json[0]["op048_1"]), int(juce_json[0]["op048_2"]),
            int(juce_json[0]["op049_0"]), int(juce_json[0]["op049_1"]), int(juce_json[0]["op049_2"]),
            int(juce_json[0]["op050_0"]), int(juce_json[0]["op050_1"]), int(juce_json[0]["op050_2"]),
            int(juce_json[0]["op051_0"]), int(juce_json[0]["op051_1"]), int(juce_json[0]["op051_2"]),
            int(juce_json[0]["op052_0"]), int(juce_json[0]["op052_1"]), int(juce_json[0]["op052_2"]),
            int(juce_json[0]["op053_0"]), int(juce_json[0]["op053_1"]), int(juce_json[0]["op053_2"]),
            int(juce_json[0]["op054_0"]), int(juce_json[0]["op054_1"]), int(juce_json[0]["op054_2"]),
            int(juce_json[0]["op055_0"]), int(juce_json[0]["op055_1"]), int(juce_json[0]["op055_2"]),
            int(juce_json[0]["op056_0"]), int(juce_json[0]["op056_1"]), int(juce_json[0]["op056_2"]),
            int(juce_json[0]["op057_0"]), int(juce_json[0]["op057_1"]), int(juce_json[0]["op057_2"]),
            int(juce_json[0]["op058_0"]), int(juce_json[0]["op058_1"]), int(juce_json[0]["op058_2"]),
            int(juce_json[0]["op059_0"]), int(juce_json[0]["op059_1"]), int(juce_json[0]["op059_2"]),
            int(juce_json[0]["op060_0"]), int(juce_json[0]["op060_1"]), int(juce_json[0]["op060_2"]),
            int(juce_json[0]["op061_0"]), int(juce_json[0]["op061_1"]), int(juce_json[0]["op061_2"]),
            int(juce_json[0]["op062_0"]), int(juce_json[0]["op062_1"]), int(juce_json[0]["op062_2"]),
            int(juce_json[0]["op063_0"]), int(juce_json[0]["op063_1"]), int(juce_json[0]["op063_2"]),

            int(juce_json[0]["op064_0"]), int(juce_json[0]["op064_1"]), int(juce_json[0]["op064_2"]),
            int(juce_json[0]["op065_0"]), int(juce_json[0]["op065_1"]), int(juce_json[0]["op065_2"]),
            int(juce_json[0]["op066_0"]), int(juce_json[0]["op066_1"]), int(juce_json[0]["op066_2"]),
            int(juce_json[0]["op067_0"]), int(juce_json[0]["op067_1"]), int(juce_json[0]["op067_2"]),
            int(juce_json[0]["op068_0"]), int(juce_json[0]["op068_1"]), int(juce_json[0]["op068_2"]),
            int(juce_json[0]["op069_0"]), int(juce_json[0]["op069_1"]), int(juce_json[0]["op069_2"]),
            int(juce_json[0]["op070_0"]), int(juce_json[0]["op070_1"]), int(juce_json[0]["op070_2"]),
            int(juce_json[0]["op071_0"]), int(juce_json[0]["op071_1"]), int(juce_json[0]["op071_2"]),
            int(juce_json[0]["op072_0"]), int(juce_json[0]["op072_1"]), int(juce_json[0]["op072_2"]),
            int(juce_json[0]["op073_0"]), int(juce_json[0]["op073_1"]), int(juce_json[0]["op073_2"]),
            int(juce_json[0]["op074_0"]), int(juce_json[0]["op074_1"]), int(juce_json[0]["op074_2"]),
            int(juce_json[0]["op075_0"]), int(juce_json[0]["op075_1"]), int(juce_json[0]["op075_2"]),
            int(juce_json[0]["op076_0"]), int(juce_json[0]["op076_1"]), int(juce_json[0]["op076_2"]),
            int(juce_json[0]["op077_0"]), int(juce_json[0]["op077_1"]), int(juce_json[0]["op077_2"]),
            int(juce_json[0]["op078_0"]), int(juce_json[0]["op078_1"]), int(juce_json[0]["op078_2"]),
            int(juce_json[0]["op079_0"]), int(juce_json[0]["op079_1"]), int(juce_json[0]["op079_2"]),

            int(juce_json[0]["op080_0"]), int(juce_json[0]["op080_1"]), int(juce_json[0]["op080_2"]),
            int(juce_json[0]["op081_0"]), int(juce_json[0]["op081_1"]), int(juce_json[0]["op081_2"]),
            int(juce_json[0]["op082_0"]), int(juce_json[0]["op082_1"]), int(juce_json[0]["op082_2"]),
            int(juce_json[0]["op083_0"]), int(juce_json[0]["op083_1"]), int(juce_json[0]["op083_2"]),
            int(juce_json[0]["op084_0"]), int(juce_json[0]["op084_1"]), int(juce_json[0]["op084_2"]),
            int(juce_json[0]["op085_0"]), int(juce_json[0]["op085_1"]), int(juce_json[0]["op085_2"]),
            int(juce_json[0]["op086_0"]), int(juce_json[0]["op086_1"]), int(juce_json[0]["op086_2"]),
            int(juce_json[0]["op087_0"]), int(juce_json[0]["op087_1"]), int(juce_json[0]["op087_2"]),
            int(juce_json[0]["op088_0"]), int(juce_json[0]["op088_1"]), int(juce_json[0]["op088_2"]),
            int(juce_json[0]["op089_0"]), int(juce_json[0]["op089_1"]), int(juce_json[0]["op089_2"]),
            int(juce_json[0]["op090_0"]), int(juce_json[0]["op090_1"]), int(juce_json[0]["op090_2"]),
            int(juce_json[0]["op091_0"]), int(juce_json[0]["op091_1"]), int(juce_json[0]["op091_2"]),
            int(juce_json[0]["op092_0"]), int(juce_json[0]["op092_1"]), int(juce_json[0]["op092_2"]),
            int(juce_json[0]["op093_0"]), int(juce_json[0]["op093_1"]), int(juce_json[0]["op093_2"]),
            int(juce_json[0]["op094_0"]), int(juce_json[0]["op094_1"]), int(juce_json[0]["op094_2"]),
            int(juce_json[0]["op095_0"]), int(juce_json[0]["op095_1"]), int(juce_json[0]["op095_2"]),

            int(juce_json[0]["op096_0"]), int(juce_json[0]["op096_1"]), int(juce_json[0]["op096_2"]),
            int(juce_json[0]["op097_0"]), int(juce_json[0]["op097_1"]), int(juce_json[0]["op097_2"]),
            int(juce_json[0]["op098_0"]), int(juce_json[0]["op098_1"]), int(juce_json[0]["op098_2"]),
            int(juce_json[0]["op099_0"]), int(juce_json[0]["op099_1"]), int(juce_json[0]["op099_2"]),
            int(juce_json[0]["op100_0"]), int(juce_json[0]["op100_1"]), int(juce_json[0]["op100_2"]),
            int(juce_json[0]["op101_0"]), int(juce_json[0]["op101_1"]), int(juce_json[0]["op101_2"]),
            int(juce_json[0]["op102_0"]), int(juce_json[0]["op102_1"]), int(juce_json[0]["op102_2"]),
            int(juce_json[0]["op103_0"]), int(juce_json[0]["op103_1"]), int(juce_json[0]["op103_2"]),
            int(juce_json[0]["op104_0"]), int(juce_json[0]["op104_1"]), int(juce_json[0]["op104_2"]),
            int(juce_json[0]["op105_0"]), int(juce_json[0]["op105_1"]), int(juce_json[0]["op105_2"]),
            int(juce_json[0]["op106_0"]), int(juce_json[0]["op106_1"]), int(juce_json[0]["op106_2"]),
            int(juce_json[0]["op107_0"]), int(juce_json[0]["op107_1"]), int(juce_json[0]["op107_2"]),
            int(juce_json[0]["op108_0"]), int(juce_json[0]["op108_1"]), int(juce_json[0]["op108_2"]),
            int(juce_json[0]["op109_0"]), int(juce_json[0]["op109_1"]), int(juce_json[0]["op109_2"]),
            int(juce_json[0]["op110_0"]), int(juce_json[0]["op110_1"]), int(juce_json[0]["op110_2"]),
            int(juce_json[0]["op111_0"]), int(juce_json[0]["op111_1"]), int(juce_json[0]["op111_2"]),

            int(juce_json[0]["op112_0"]), int(juce_json[0]["op112_1"]), int(juce_json[0]["op112_2"]),
            int(juce_json[0]["op113_0"]), int(juce_json[0]["op113_1"]), int(juce_json[0]["op113_2"]),
            int(juce_json[0]["op114_0"]), int(juce_json[0]["op114_1"]), int(juce_json[0]["op114_2"]),
            int(juce_json[0]["op115_0"]), int(juce_json[0]["op115_1"]), int(juce_json[0]["op115_2"]),
            int(juce_json[0]["op116_0"]), int(juce_json[0]["op116_1"]), int(juce_json[0]["op116_2"]),
            int(juce_json[0]["op117_0"]), int(juce_json[0]["op117_1"]), int(juce_json[0]["op117_2"]),
            int(juce_json[0]["op118_0"]), int(juce_json[0]["op118_1"]), int(juce_json[0]["op118_2"]),
            int(juce_json[0]["op119_0"]), int(juce_json[0]["op119_1"]), int(juce_json[0]["op119_2"]),
            int(juce_json[0]["op120_0"]), int(juce_json[0]["op120_1"]), int(juce_json[0]["op120_2"]),
            int(juce_json[0]["op121_0"]), int(juce_json[0]["op121_1"]), int(juce_json[0]["op121_2"]),
            int(juce_json[0]["op122_0"]), int(juce_json[0]["op122_1"]), int(juce_json[0]["op122_2"]),
            int(juce_json[0]["op123_0"]), int(juce_json[0]["op123_1"]), int(juce_json[0]["op123_2"]),
            int(juce_json[0]["op124_0"]), int(juce_json[0]["op124_1"]), int(juce_json[0]["op124_2"]),
            int(juce_json[0]["op125_0"]), int(juce_json[0]["op125_1"]), int(juce_json[0]["op125_2"]),
            int(juce_json[0]["op126_0"]), int(juce_json[0]["op126_1"]), int(juce_json[0]["op126_2"]),
            int(juce_json[0]["op127_0"]), int(juce_json[0]["op127_1"]), int(juce_json[0]["op127_2"])
        };

        return choc::value::Value(0);
    };

    auto web_view_callback_on_initial_update =
        [safe_this = juce::Component::SafePointer(this)](const choc::value::ValueView) -> choc::value::Value {
        if (safe_this.getComponent() == nullptr) {return choc::value::Value(-1);}

        safe_this->parameterChanged("sys_ingain", safe_this->valueTreeState.getRawParameterValue("sys_ingain")->load());
        safe_this->parameterChanged("sys_outgain", safe_this->valueTreeState.getRawParameterValue("sys_outgain")->load());
        safe_this->parameterChanged("pot0_ctrl", safe_this->valueTreeState.getRawParameterValue("pot0_ctrl")->load());
        safe_this->parameterChanged("pot1_ctrl", safe_this->valueTreeState.getRawParameterValue("pot1_ctrl")->load());
        safe_this->parameterChanged("pot2_ctrl", safe_this->valueTreeState.getRawParameterValue("pot2_ctrl")->load());
        safe_this->parameterChanged("pot3_ctrl", safe_this->valueTreeState.getRawParameterValue("pot3_ctrl")->load());
        safe_this->parameterChanged("knob_name", safe_this->valueTreeState.getRawParameterValue("knob_name")->load());
        safe_this->parameterChanged("sys_bypass", safe_this->valueTreeState.getRawParameterValue("sys_bypass")->load());
        safe_this->parameterChanged("sys_mute", safe_this->valueTreeState.getRawParameterValue("sys_mute")->load());
        safe_this->parameterChanged("sys_wf_send", safe_this->valueTreeState.getRawParameterValue("sys_wf_send")->load());
        safe_this->parameterChanged("sys_wf0_sel", safe_this->valueTreeState.getRawParameterValue("sys_wf0_sel")->load());
        safe_this->parameterChanged("sys_wf1_sel", safe_this->valueTreeState.getRawParameterValue("sys_wf1_sel")->load());
        safe_this->parameterChanged("sys_wf2_sel", safe_this->valueTreeState.getRawParameterValue("sys_wf2_sel")->load());
        safe_this->parameterChanged("sys_wf3_sel", safe_this->valueTreeState.getRawParameterValue("sys_wf3_sel")->load());
        safe_this->parameterChanged("ui_size", safe_this->valueTreeState.getRawParameterValue("ui_size")->load());
        safe_this->parameterChanged("ui_dark", safe_this->valueTreeState.getRawParameterValue("ui_dark")->load());
        safe_this->parameterChanged("ui_lh", safe_this->valueTreeState.getRawParameterValue("ui_lh")->load());
        safe_this->parameterChanged("ui_fs", safe_this->valueTreeState.getRawParameterValue("ui_fs")->load());
        safe_this->parameterChanged("sys_rs", safe_this->valueTreeState.getRawParameterValue("sys_rs")->load());
        safe_this->parameterChanged("ui_font", 0.0f);
        safe_this->parameterChanged("sys_loadfile", 0.0f);
        safe_this->parameterChanged("sys_running", 0.0f);

        return choc::value::Value(0);
    };

    chocWebView->bind("onToggleChanged", web_view_callback_on_toggle_changed);
    chocWebView->bind("onSliderChanged", web_view_callback_on_sliider_changed);
    chocWebView->bind("onSelectChanged", web_view_callback_on_select_changed);
    chocWebView->bind("onCalled", web_view_callback_on_called);
    chocWebView->bind("onTextChanged", web_view_callback_on_text_changed);
    chocWebView->bind("onOpChanged", web_view_callback_on_op_changed);
    chocWebView->bind("onInitialUpdate", web_view_callback_on_initial_update);

#if WEB_VIEW_FROM_SERVER
    chocWebView->navigate ("http://localhost:5173");
#else
    const auto html = juce::String::createStringFromData (BinaryData::view_html, BinaryData::view_htmlSize);
    chocWebView->setHTML (html.toStdString());
#endif

    juceWebViewHolder = createJUCEWebViewHolder (*chocWebView.get());
    addAndMakeVisible (juceWebViewHolder.get());

    valueTreeState.addParameterListener("sys_ingain", this);
    valueTreeState.addParameterListener("sys_outgain", this);
    valueTreeState.addParameterListener("pot0_ctrl", this);
    valueTreeState.addParameterListener("pot1_ctrl", this);
    valueTreeState.addParameterListener("pot2_ctrl", this);
    valueTreeState.addParameterListener("pot3_ctrl", this);
    valueTreeState.addParameterListener("knob_name", this);
    valueTreeState.addParameterListener("sys_bypass", this);
    valueTreeState.addParameterListener("sys_mute", this);
    valueTreeState.addParameterListener("sys_wf_send", this);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    if (valueTreeState.getRawParameterValue("ui_size")->load() == 3.0f) {
        setSize(640, 360);
    } else if (valueTreeState.getRawParameterValue("ui_size")->load() == 7.0f) {
        setSize(1280, 720);
    } else {
        setSize(1920, 1080);
    }
    setResizeLimits(640, 360, 1920, 1080);
    setResizable (true, true);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    valueTreeState.removeParameterListener("sys_ingain", this);
    valueTreeState.removeParameterListener("sys_outgain", this);
    valueTreeState.removeParameterListener("pot0_ctrl", this);
    valueTreeState.removeParameterListener("pot1_ctrl", this);
    valueTreeState.removeParameterListener("pot2_ctrl", this);
    valueTreeState.removeParameterListener("pot3_ctrl", this);
    valueTreeState.removeParameterListener("knob_name", this);
    valueTreeState.removeParameterListener("sys_bypass", this);
    valueTreeState.removeParameterListener("sys_mute", this);
    valueTreeState.removeParameterListener("sys_wf_send", this);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
    /*auto rect_ui = getLocalBounds();

    auto gainRect = rect_ui.removeFromTop (paramControlHeight);
    sys_ingainLabel .setBounds (gainRect.removeFromLeft (paramLabelWidth));
    sys_ingainSlider.setBounds (gainRect);

    invertButton.setBounds (rect_ui.removeFromTop (paramControlHeight));*/

    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    juceWebViewHolder->setBounds (getLocalBounds());
}

void AudioPluginAudioProcessorEditor::parameterChanged(const juce::String& parameterID, float newValue)
{
    juce::String javascript = "";
    juce::DynamicObject::Ptr json = new juce::DynamicObject();

    if (parameterID == "sys_ingain") {
        json->setProperty("parameterName", "sys_ingain");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString (json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_outgain") {
        json->setProperty("parameterName", "sys_outgain");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "pot0_ctrl") {
        json->setProperty("parameterName", "pot0_ctrl");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "pot1_ctrl") {
        json->setProperty("parameterName", "pot1_ctrl");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "pot2_ctrl") {
        json->setProperty("parameterName", "pot2_ctrl");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "pot3_ctrl") {
        json->setProperty("parameterName", "pot3_ctrl");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "knob_name") {
        json->setProperty("parameterName", "knob_name");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_wf_send" && newValue != 0.0f) {
        json->setProperty("parameterName", "sys_wf_send");
        json->setProperty("wf0max", sys_wf0_max);
        json->setProperty("wf0min", sys_wf0_min);
        json->setProperty("wf1max", sys_wf1_max);
        json->setProperty("wf1min", sys_wf1_min);
        json->setProperty("wf2max", sys_wf2_max);
        json->setProperty("wf2min", sys_wf2_min);
        json->setProperty("wf3max", sys_wf3_max);
        json->setProperty("wf3min", sys_wf3_min);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_wf0_sel") {
        json->setProperty("parameterName", "sys_wf0_sel");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_wf1_sel") {
        json->setProperty("parameterName", "sys_wf1_sel");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_wf2_sel") {
        json->setProperty("parameterName", "sys_wf2_sel");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_wf3_sel") {
        json->setProperty("parameterName", "sys_wf3_sel");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "ui_font") {
        json->setProperty("parameterName", "ui_font");
        json->setProperty("parameterFont", ui_font);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "ui_dark") {
        json->setProperty("parameterName", "ui_dark");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "ui_size") {
        json->setProperty("parameterName", "ui_size");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "ui_lh") {
        json->setProperty("parameterName", "ui_lh");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "ui_fs") {
        json->setProperty("parameterName", "ui_fs");
        json->setProperty("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_rs") {
        json->setProperty("parameterName", "sys_rs");
        json->setProperty("parameterValue", newValue ? 1.0f : 0.0f);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_loadfile") {
        json->setProperty("parameterName", "sys_loadfile");
        json->setProperty("parameterFilename", sys_file.getFileName());
        json->setProperty("parameterEdited", sys_edited);
        json->setProperty("parameterCode", sys_code);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_running") {
        json->setProperty("parameterName", "sys_running");
        json->setProperty("parameterValue", sys_running);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_bypass") {
        json->setProperty("parameterName", "sys_bypass");
        json->setProperty("parameterValue", newValue ? 1.0f : 0.0f);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }
    else if (parameterID == "sys_mute") {
        json->setProperty("parameterName", "sys_mute");
        json->setProperty("parameterValue", newValue ? 1.0f : 0.0f);
        const auto js_args_json = juce::JSON::toString(json.get());
        javascript = juce::String("onParameterChanged(") + js_args_json + juce::String(")");
    }/*
    else if (parameterID == "invertPhase") {
        juce::DynamicObject::Ptr json = new juce::DynamicObject();
        json->setProperty ("parameterName", "invertPhase");
        json->setProperty ("parameterValue", newValue);
        const auto js_args_json = juce::JSON::toString (json.get());
        javascript = juce::String ("onParameterChanged(") + js_args_json + juce::String (")");
    }*/

    if (javascript.isNotEmpty())
    {
        if (juce::MessageManager::getInstance()->isThisTheMessageThread())
        {
            const bool result = chocWebView->evaluateJavascript (javascript.toStdString());
            if (! result)
            {
                juce::Logger::outputDebugString ("Failed: " + javascript);
            }
        }
        else
        {
            juce::MessageManager::callAsync (
                [safe_this = juce::Component::SafePointer (this), javascript]
                {
                    if (safe_this.getComponent() == nullptr)
                    {
                        return;
                    }

                    const bool result = safe_this->chocWebView->evaluateJavascript (javascript.toStdString());
                    if (! result)
                    {
                        juce::Logger::outputDebugString ("Failed: " + javascript);
                    }
                });
        }
    }
}
