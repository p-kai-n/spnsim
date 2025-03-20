#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    juce::AudioProcessorValueTreeState& getAPVTS() { return *parameters.get(); }

    void resetRegisters() {
        reg.fill(0);
        dram.fill(0);

        sys_wf0_max = 0.0f;
        sys_wf0_min = 0.0f;
        sys_wf1_max = 0.0f;
        sys_wf1_min = 0.0f;
        sys_wf2_max = 0.0f;
        sys_wf2_min = 0.0f;
        sys_wf3_max = 0.0f;
        sys_wf3_min = 0.0f;

        sin0lfo = 0.0f;
        cos0lfo = 0.0f;
        sin1lfo = 0.0f;
        cos1lfo = 0.0f;
        rmp0lfo = 0.0f;
        rmp0sublfo = 0.0f;
        rmp0tplfo = 1.0f;
        rmp1lfo = 0.0f;
        rmp1sublfo = 0.0f;
        rmp1tplfo = 1.0f;

        sin0deg = 0.0f;
        sin1deg = 0.0f;
        rmp0inc = 0.0f;
        rmp1inc = 0.0f;
        rmp0subinc = 0.5f;
        rmp1subinc = 0.5f;

        sin0use = false;
        sin1use = false;
        rmp0use = false;
        rmp1use = false;

        acc = 0;
        pacc = 0;

        sin0_rate = 0;
        sin0_rate_ = 0;
        sin0_range = 0;
        sin0_range_ = 0;
        sin1_rate = 0;
        sin1_rate_ = 0;
        sin1_range = 0;
        sin1_range_ = 0;

        rmp0_rate = 0;
        rmp0_rate_ = 0;
        rmp0_range = 0;
        rmp0_range_ = 0;
        rmp1_rate = 0;
        rmp1_rate_ = 0;
        rmp1_range = 0;
        rmp1_range_ = 0;

        pot0 = 0;
        pot0_ = 0;
        pot1 = 0;
        pot1_ = 0;
        pot2 = 0;
        pot2_ = 0;

        adcl = 0;
        adcl_ = 0;
        adcr = 0;
        adcr_ = 0;
        dacl = 0;
        dacl_ = 0;
        dacr = 0;
        dacr_ = 0;

        addr_ptr = 0;
        addr_ptr_ = 0;

        last_rda = 0;
        cho_reg = 0;
        dram_base = 0;
    };

    void clampRegisters() {
        for (int i = 0; i < reg.size(); i++) {
            reg[i] = std::clamp(getRegister(i), max * -2, max);
        }

        acc = std::clamp(acc, max * -2, max);
        pacc = std::clamp(pacc, max * -2, max);

        sin0_rate_ = std::clamp(sin0_rate_, max * -2, max);
        sin0_range_ = std::clamp(sin0_range_, max * -2, max);
        sin1_rate_ = std::clamp(sin1_rate_, max * -2, max);
        sin1_range_ = std::clamp(sin1_range_, max * -2, max);

        rmp0_rate_ = std::clamp(rmp0_rate_, max * -2, max);
        rmp0_range_ = std::clamp(rmp0_range_, max * -2, max);
        rmp1_rate_ = std::clamp(rmp1_rate_, max * -2, max);
        rmp1_range_ = std::clamp(rmp1_range_, max * -2, max);

        pot0_ = std::clamp(pot0_, max * -2, max);
        pot1_ = std::clamp(pot1_, max * -2, max);
        pot2_ = std::clamp(pot2_, max * -2, max);

        adcl_ = std::clamp(adcl_, max * -2, max);
        adcr_ = std::clamp(adcr_, max * -2, max);
        dacl_ = std::clamp(dacl_, max * -2, max);
        dacr_ = std::clamp(dacr_, max * -2, max);

        addr_ptr_ = std::clamp(addr_ptr_, max * -2, max);

        last_rda = std::clamp(last_rda, max * -2, max);
    };

    int& getRegister(int num) {
        if (num < 32) {
            return reg[num];
        }
        switch (num) {
        case 32: {
            return adcl_;
        }
        case 33: {
            return adcr_;
        }
        case 34: {
            return dacl_;
        }
        case 35: {
            return dacr_;
        }
        case 36: {
            return pot0_;
        }
        case 37: {
            return pot1_;
        }
        case 38: {
            return pot2_;
        }
        case 39: {
            return sin0_rate_;
        }
        case 40: {
            return sin0_range_;
        }
        case 41: {
            return sin1_rate_;
        }
        case 42: {
            return sin1_range_;
        }
        case 43: {
            return rmp0_rate_;
        }
        case 44: {
            return rmp0_range_;
        }
        case 45: {
            return rmp1_rate_;
        }
        case 46: {
            return rmp1_range_;
        }
        case 47: {
            return addr_ptr_;
        }
        }
        return acc;
    };

    int& setRegister(int num, int val) {
        if (num < 32) {
            reg[num] = val;
        }
        switch (num) {
        case 32: {
            adcl_ = val;
            return adcl_;
        }
        case 33: {
            adcr_ = val;
            return adcr_;
        }
        case 34: {
            dacl_ = val;
            return dacl_;
        }
        case 35: {
            dacr_ = val;
            return dacr_;
        }
        case 36: {
            pot0_ = val;
            return pot0_;
        }
        case 37: {
            pot1_ = val;
            return pot1_;
        }
        case 38: {
            pot2_ = val;
            return pot2_;
        }
        case 39: {
            sin0_rate_ = val;
            return sin0_rate_;
        }
        case 40: {
            sin0_range_ = val;
            return sin0_range_;
        }
        case 41: {
            sin1_rate_ = val;
            return sin1_rate_;
        }
        case 42: {
            sin1_range_ = val;
            return sin1_range_;
        }
        case 43: {
            rmp0_rate_ = val;
            return rmp0_rate_;
        }
        case 44: {
            rmp0_range_ = val;
            return rmp0_range_;
        }
        case 45: {
            rmp1_rate_ = val;
            return rmp1_rate_;
        }
        case 46: {
            rmp1_range_ = val;
            return rmp1_range_;
        }
        case 47: {
            addr_ptr_ = val;
            return addr_ptr_;
        }
        }
        return acc;
    };

    int& getChorus(unsigned __int16 addr0, unsigned __int16 addr1, float lfo) {
        int intaddr0 = int(floor((32767.0f * 0.5f * lfo) + float(addr0)));
        int intaddr1 = int(floor((32767.0f * 0.5f * lfo) + float(addr1)));
        float decaddr = (float(32767.0f * 0.5f * lfo) + float(addr0)) - float(intaddr0);

        int dram_pre0 = dram_base - intaddr0;
        if (dram_pre0 < 0) {
            dram_pre0 = dram_pre0 + 32768;
        }
        if (dram_pre0 > 32767) {
            dram_pre0 = dram_pre0 - 32768;
        }

        int dram_pre1 = dram_base - intaddr1;
        if (dram_pre1 < 0) {
            dram_pre1 = dram_pre1 + 32768;
        }
        if (dram_pre1 > 32767) {
            dram_pre1 = dram_pre1 - 32768;
        }
        /*
        juce::String text = "addr = ";
        text += intaddr0;
        text += " . ";
        text += int(floor(decaddr * 1000.0f));
        DBG(juce::String(text));*/

        acc = int(float(dram[dram_pre0]) * float(1.0f - decaddr)) + int(float(dram[dram_pre1]) * float(decaddr));
        return acc;
    };

    void resetSin0() {
        sin0use = 1;
        sin0deg = 0.0f;
    };
    void calcSin0() {
        sin0lfo = float(std::sin(sin0deg) * float(sin0_range * min));
        cos0lfo = float(std::cos(sin0deg) * float(sin0_range * min));
        float cyclesPerSecond = float((sin0_rate * min) * 20.0);
        float cyclesPerSample = float(cyclesPerSecond / 32768.0);
        sin0deg = sin0deg + float(cyclesPerSample * 2.0 * juce::MathConstants<double>::pi);
    };

    void resetSin1() {
        sin1use = 1;
        sin1deg = 0.0f;
    };
    void calcSin1() {
        sin1lfo = float(std::sin(sin1deg) * float(sin1_range * min));
        cos1lfo = float(std::cos(sin1deg) * float(sin1_range * min));
        float cyclesPerSecond = float((sin1_rate * min) * 20.0);
        float cyclesPerSample = float(cyclesPerSecond / 32768.0);
        sin1deg = sin1deg + float(cyclesPerSample * 2.0 * juce::MathConstants<double>::pi);
    };

    int& getPitch(unsigned __int16 addr0, unsigned __int16 addr1, float lfo) {
        int intaddr0 = int(floor((4096.0f * 2.0f * lfo) + float(addr0)));
        int intaddr1 = int(floor((4096.0f * 2.0f * lfo) + float(addr1)));
        float decaddr = (float(4096.0f * 2.0f * lfo) + float(addr0)) - float(intaddr0);

        int dram_pre0 = dram_base - intaddr0;
        if (dram_pre0 < 0) {
            dram_pre0 = dram_pre0 + 32768;
        }
        if (dram_pre0 > 32767) {
            dram_pre0 = dram_pre0 - 32768;
        }

        int dram_pre1 = dram_base - intaddr1;
        if (dram_pre1 < 0) {
            dram_pre1 = dram_pre1 + 32768;
        }
        if (dram_pre1 > 32767) {
            dram_pre1 = dram_pre1 - 32768;
        }

        acc = int(float(dram[dram_pre0]) * float(1.0f - decaddr)) + int(float(dram[dram_pre1]) * float(decaddr));
        return acc;
    };

    void resetRmp0() {
        rmp0use = 1;
        rmp0inc = 0.0f;
        rmp0subinc = 0.5f;
        rmp0tplfo = 1.0f;
    };
    void calcRmp0() {
        rmp0lfo = float(rmp0inc * (rmp0_range * 0.0001220703125));
        rmp0sublfo = float(rmp0subinc * (rmp0_range * 0.0001220703125));

        float cyclesPerSecond = float((rmp0_rate * min) * 16.0);
        float cyclesPerSample = float(cyclesPerSecond / 32768.0);

        switch (rmp0_range) {
        case  512: {
            cyclesPerSample = cyclesPerSample * 8.0f;
            break;
        }
        case 1024: {
            cyclesPerSample = cyclesPerSample * 4.0f;
            break;
        }
        case 2048: {
            cyclesPerSample = cyclesPerSample * 2.0f;
            break;
        }
//      case 4096: {
//          break;
//      }
        }

        rmp0inc = rmp0inc - cyclesPerSample;
        if (rmp0inc > 1.0f) {
            rmp0inc = 0.0f - cyclesPerSample;
        } else if (rmp0inc < 0.0f) {
            rmp0inc = 1.0f - cyclesPerSample;
        }

        rmp0subinc = float(rmp0subinc - cyclesPerSample);
        if (rmp0subinc > 1.0f) {
            rmp0subinc = float(0.0f - cyclesPerSample);
        } else if (rmp0subinc < 0.0f) {
            rmp0subinc = float(1.0f - cyclesPerSample);
        }

        if (rmp0inc < 0.375f) {
            if (rmp0tplfo >= 1.0f) {
                rmp0tplfo = 1.0f;
            } else {
                rmp0tplfo = rmp0tplfo + (cyclesPerSample * 4.0f);
            }
        } else if (rmp0inc < 0.875f) {
            if (rmp0tplfo <= 0.0f) {
                rmp0tplfo = 0.0f;
            } else {
                rmp0tplfo = rmp0tplfo - (cyclesPerSample * 4.0f);
            }
        }
    };

    void resetRmp1() {
        rmp1use = 1;
        rmp1inc = 0.0f;
        rmp1subinc = 0.5f;
        rmp1tplfo = 1.0f;
    };
    void calcRmp1() {
        rmp1lfo = rmp1inc * float(rmp1_range * 0.0001220703125);
        rmp1sublfo = rmp1subinc * float(rmp1_range * 0.0001220703125);

        float cyclesPerSecond = float((rmp1_rate * min) * 16.0);
        float cyclesPerSample = float(cyclesPerSecond / 32768.0);

        switch (rmp0_range) {
        case  512: {
            cyclesPerSample = cyclesPerSample * 8.0f;
            break;
        }
        case 1024: {
            cyclesPerSample = cyclesPerSample * 4.0f;
            break;
        }
        case 2048: {
            cyclesPerSample = cyclesPerSample * 2.0f;
            break;
        }
//      case 4096: {
//          break;
//      }
        }

        rmp1inc = rmp1inc - cyclesPerSample;
        if (rmp1inc  > 1.0f) {
            rmp1inc = 0.0f;
        } else if (rmp1inc < 0.0f) {
            rmp1inc = 1.0f;
        }

        rmp1subinc = float(rmp1subinc - cyclesPerSample);
        if (rmp1subinc > 1.0f) {
            rmp1subinc = float(0.0f - cyclesPerSample);
        } else if (rmp1subinc < 0.0f) {
            rmp1subinc = float(1.0f - cyclesPerSample);
        }

        if (rmp1inc < 0.375f) {
            if (rmp1tplfo >= 1.0f) {
                rmp1tplfo = 1.0f;
            } else {
                rmp1tplfo = rmp1tplfo + (cyclesPerSample * 4.0f);
            }
        } else if (rmp1inc < 0.875f) {
            if (rmp1tplfo <= 0.0f) {
                rmp1tplfo = 0.0f;
            } else {
                rmp1tplfo = rmp1tplfo - (cyclesPerSample * 4.0f);
            }
        }
    };

private:
    //==============================================================================
    std::unique_ptr<juce::AudioProcessorValueTreeState> parameters;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::File sys_file;
    juce::String sys_edited;
    juce::String sys_code;
    juce::String ui_font;
    unsigned __int8 sys_running;
    
    const int bm = 8191;
    std::array<float, 8192> lInput_dup;//should set bigger array length to support long block size
    std::array<float, 8192> rInput_dup;
    std::array<float, 8192> lInput_buff2x;
    std::array<float, 8192> rInput_buff2x;
    std::array<float, 8192> lInput_buff2x3d;
    std::array<float, 8192> rInput_buff2x3d;
    std::array<float, 8192> lOutput_buff;
    std::array<float, 8192> rOutput_buff;
    std::array<float, 8192> lOutput_buff3x;
    std::array<float, 8192> rOutput_buff3x;
    std::array<float, 8192> lOutput_prev;
    std::array<float, 8192> rOutput_prev;
    unsigned __int8 buffCounter;
    unsigned __int16 inBuffLength;
    unsigned __int16 inBuffLength3d;

    juce::dsp::FIR::Filter<float> lInput_usLPF;
    juce::dsp::FIR::Filter<float> rInput_usLPF;

    juce::dsp::FIR::Filter<float> lOutput_usLPF;
    juce::dsp::FIR::Filter<float> rOutput_usLPF;

    std::array<int, 384> sim_op;
    std::array<int, 32> reg;
    std::array<int, 32768> dram;

    const int max = 8388608;
    const double min = 1.192092895507813e-7;

    std::atomic<float>* sys_ingain  = nullptr;
    std::atomic<float>* sys_outgain = nullptr;
    std::atomic<float>* sys_bypass = nullptr;
    std::atomic<float>* sys_mute = nullptr;
    std::atomic<float>* sys_wf_send = nullptr;
    std::atomic<float>* sys_wf0_sel = nullptr;
    std::atomic<float>* sys_wf1_sel = nullptr;
    std::atomic<float>* sys_wf2_sel = nullptr;
    std::atomic<float>* sys_wf3_sel = nullptr;
    std::atomic<float>* knob_name = nullptr;
    std::atomic<float>* ui_dark = nullptr;
    std::atomic<float>* ui_size_x = nullptr;
    std::atomic<float>* ui_size_y = nullptr;
    std::atomic<float>* ui_lh = nullptr;
    std::atomic<float>* ui_fs = nullptr;
    std::atomic<float>* sys_rs = nullptr;

    std::atomic<float>* sin0_rate_ctrl = nullptr;
    std::atomic<float>* sin0_range_ctrl = nullptr;
    std::atomic<float>* sin1_rate_ctrl = nullptr;
    std::atomic<float>* sin1_range_ctrl = nullptr;

    std::atomic<float>* rmp0_rate_ctrl = nullptr;
    std::atomic<float>* rmp0_range_ctrl = nullptr;
    std::atomic<float>* rmp1_rate_ctrl = nullptr;
    std::atomic<float>* rmp1_range_ctrl = nullptr;

    std::atomic<float>* pot0_ctrl = nullptr;
    std::atomic<float>* pot1_ctrl = nullptr;
    std::atomic<float>* pot2_ctrl = nullptr;
    std::atomic<float>* pot3_ctrl = nullptr;

    int sin0_rate;
    int sin0_rate_;
    int sin0_range;
    int sin0_range_;
    int sin1_rate;
    int sin1_rate_;
    int sin1_range;
    int sin1_range_;

    int rmp0_rate;
    int rmp0_rate_;
    int rmp0_range;
    int rmp0_range_;
    int rmp1_rate;
    int rmp1_rate_;
    int rmp1_range;
    int rmp1_range_;

    int pot0;
    int pot0_;
    int pot1;
    int pot1_;
    int pot2;
    int pot2_;

    int adcl;
    int adcl_;
    int adcr;
    int adcr_;
    int dacl;
    int dacl_;
    int dacr;
    int dacr_;

    int addr_ptr;
    int addr_ptr_;

    float buffMax00;
    float buffMin00;
    float buffMax10;
    float buffMin10;
    float buffMax20;
    float buffMin20;
    float buffMax30;
    float buffMin30;

    float buffMax01;
    float buffMin01;
    float buffMax11;
    float buffMin11;
    float buffMax21;
    float buffMin21;
    float buffMax31;
    float buffMin31;

    float buffMax02;
    float buffMin02;
    float buffMax12;
    float buffMin12;
    float buffMax22;
    float buffMin22;
    float buffMax32;
    float buffMin32;

    float buffMax00_prev;
    float buffMin00_prev;
    float buffMax10_prev;
    float buffMin10_prev;
    float buffMax20_prev;
    float buffMin20_prev;
    float buffMax30_prev;
    float buffMin30_prev;

    float buffMax01_prev;
    float buffMin01_prev;
    float buffMax11_prev;
    float buffMin11_prev;
    float buffMax21_prev;
    float buffMin21_prev;
    float buffMax31_prev;
    float buffMin31_prev;

    float buffMax02_prev;
    float buffMin02_prev;
    float buffMax12_prev;
    float buffMin12_prev;
    float buffMax22_prev;
    float buffMin22_prev;
    float buffMax32_prev;
    float buffMin32_prev;

    float sys_wf0_max;
    float sys_wf0_min;
    float sys_wf1_max;
    float sys_wf1_min;
    float sys_wf2_max;
    float sys_wf2_min;
    float sys_wf3_max;
    float sys_wf3_min;

    float sin0lfo;
    float cos0lfo;
    float sin1lfo;
    float cos1lfo;
    float rmp0lfo;
    float rmp0sublfo;
    float rmp0tplfo;
    float rmp1lfo;
    float rmp1sublfo;
    float rmp1tplfo;

    float sin0deg;
    float sin1deg;
    float rmp0inc;
    float rmp0subinc;
    float rmp1inc;
    float rmp1subinc;

    bool sin0use;
    bool sin1use;
    bool rmp0use;
    bool rmp1use;

    int acc;
    int pacc;
    int pacc_temp;
    int last_rda;
    double logvalue;
    int dram_pre;
    unsigned __int16 cho_reg;
    unsigned __int16 dram_base;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sys_ingainSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sys_outgainSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pot0_ctrlSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pot1_ctrlSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pot2_ctrlSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> pot3_ctrlSm;

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sys_bypassSm;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> sys_muteSm;

    void updateParameters() {
        sys_ingainSm.setTargetValue(*sys_ingain);
        sys_outgainSm.setTargetValue(*sys_outgain);

        pot0_ctrlSm.setTargetValue(*pot0_ctrl);
        pot1_ctrlSm.setTargetValue(*pot1_ctrl);
        pot2_ctrlSm.setTargetValue(*pot2_ctrl);
        pot3_ctrlSm.setTargetValue(*pot3_ctrl);

        sys_bypassSm.setTargetValue(*sys_bypass ? 0.0f : 1.0f);
        sys_muteSm.setTargetValue(*sys_mute ? 0.0f : 1.0f);
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};