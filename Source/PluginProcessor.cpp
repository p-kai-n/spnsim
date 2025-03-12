#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cstdlib>

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
{
    parameters = std::make_unique<juce::AudioProcessorValueTreeState>(*this, nullptr, juce::Identifier ("APVTS"), createParameterLayout());
    
    //phase = parameters->getRawParameterValue("invertPhase");
    sys_ingain = parameters->getRawParameterValue("sys_ingain");
    sys_outgain = parameters->getRawParameterValue("sys_outgain");

    pot0_ctrl = parameters->getRawParameterValue("pot0_ctrl");
    pot1_ctrl = parameters->getRawParameterValue("pot1_ctrl");
    pot2_ctrl = parameters->getRawParameterValue("pot2_ctrl");
    pot3_ctrl = parameters->getRawParameterValue("pot3_ctrl");

    sys_bypass = parameters->getRawParameterValue("sys_bypass");
    sys_mute = parameters->getRawParameterValue("sys_mute");

    sys_wf_send = parameters->getRawParameterValue("sys_wf_send");
    sys_wf0_sel = parameters->getRawParameterValue("sys_wf0_sel");
    sys_wf1_sel = parameters->getRawParameterValue("sys_wf1_sel");
    sys_wf2_sel = parameters->getRawParameterValue("sys_wf2_sel");
    sys_wf3_sel = parameters->getRawParameterValue("sys_wf3_sel");
    knob_name = parameters->getRawParameterValue("knob_name");

    ui_dark = parameters->getRawParameterValue("ui_dark");
    ui_size = parameters->getRawParameterValue("ui_size");
    ui_lh = parameters->getRawParameterValue("ui_lh");
    ui_fs = parameters->getRawParameterValue("ui_fs");

    sys_rs = parameters->getRawParameterValue("sys_rs");

    ui_font = "";
    sys_file = "";
    sys_edited = "";
    sys_code = "";
    sys_running = 0;

    lInput_buff2x.fill(0.0f);
    rInput_buff2x.fill(0.0f);
    lInput_buff2x3d.fill(0.0f);
    rInput_buff2x3d.fill(0.0f);

    lOutput_buff.fill(0.0f);
    rOutput_buff.fill(0.0f);
    lOutput_buff3x.fill(0.0f);
    rOutput_buff3x.fill(0.0f);

    sim_op.fill(0);
    resetRegisters();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::ignoreUnused (sampleRate, samplesPerBlock);

    sys_ingainSm.reset(sampleRate, 0.01);
    sys_ingainSm.setCurrentAndTargetValue(*sys_ingain);
    sys_outgainSm.reset(sampleRate, 0.01);
    sys_outgainSm.setCurrentAndTargetValue(*sys_outgain);

    pot0_ctrlSm.reset(sampleRate, 0.01);
    pot0_ctrlSm.setCurrentAndTargetValue(*pot0_ctrl);
    pot1_ctrlSm.reset(sampleRate, 0.01);
    pot1_ctrlSm.setCurrentAndTargetValue(*pot1_ctrl);
    pot2_ctrlSm.reset(sampleRate, 0.01);
    pot2_ctrlSm.setCurrentAndTargetValue(*pot2_ctrl);
    pot3_ctrlSm.reset(sampleRate, 0.01);
    pot3_ctrlSm.setCurrentAndTargetValue(*pot3_ctrl);

    sys_bypassSm.reset(sampleRate, 0.01);
    sys_bypassSm.setCurrentAndTargetValue(*sys_bypass);
    sys_muteSm.reset(sampleRate, 0.01);
    sys_muteSm.setCurrentAndTargetValue(*sys_mute);
    //previousGain = *sys_ingain;

    lInput_usLPF = juce::dsp::FilterDesign<float>::designFIRLowpassWindowMethod
    (10000.0f, sampleRate, 512, juce::dsp::WindowingFunction<float>::blackman);
    rInput_usLPF = juce::dsp::FilterDesign<float>::designFIRLowpassWindowMethod
    (10000.0f, sampleRate, 512, juce::dsp::WindowingFunction<float>::blackman);

    lOutput_usLPF = juce::dsp::FilterDesign<float>::designFIRLowpassWindowMethod
    (10000.0f, sampleRate, 512, juce::dsp::WindowingFunction<float>::blackman);
    rOutput_usLPF = juce::dsp::FilterDesign<float>::designFIRLowpassWindowMethod
    (10000.0f, sampleRate, 512, juce::dsp::WindowingFunction<float>::blackman);

}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    /*
    auto phase = *phaseParameter < 0.5f ? 1.0f : -1.0f;
    auto currentGain = *sys_ingainParameter * phase;

    if (juce::approximatelyEqual (currentGain, previousGain))
    {
        buffer.applyGain (currentGain);
    }
    else
    {
        buffer.applyGainRamp (0, buffer.getNumSamples(), previousGain, currentGain);
        previousGain = currentGain;
    }*/

    auto* lInput = buffer.getReadPointer(0);
    auto* rInput = buffer.getReadPointer(1);
    auto* lOutput = buffer.getWritePointer(0);
    auto* rOutput = buffer.getWritePointer(1);
    auto buffLength = buffer.getNumSamples();
    unsigned __int16 inBuffLength = 0;

    float buffMax0 = -1.0f;
    float buffMin0 = 1.0f;
    float buffMax1 = -1.0f;
    float buffMin1 = 1.0f;
    float buffMax2 = -1.0f;
    float buffMin2 = 1.0f;
    float buffMax3 = -1.0f;
    float buffMin3 = 1.0f;
    int dram_pre = 0;
    int pacc_temp = 0;

    lInput_buff2x.fill(0.0f);
    rInput_buff2x.fill(0.0f);
    lInput_buff2x3d.fill(0.0f);
    rInput_buff2x3d.fill(0.0f);
    lOutput_buff.fill(0.0f);
    rOutput_buff.fill(0.0f);
    lOutput_buff3x.fill(0.0f);
    rOutput_buff3x.fill(0.0f);

    updateParameters();

    if (sys_running > 0) {
    if (sys_rs->load() != 0.0f) {
        for (int samplesIdx = 0; samplesIdx < buffLength; samplesIdx++) {
            auto ingain = juce::Decibels::decibelsToGain(sys_ingainSm.getNextValue() - 48.0f, -48.0f);
            lInput_buff2x[std::clamp((samplesIdx * 2), 0, bm)] = lInput[samplesIdx] * ingain;
            rInput_buff2x[std::clamp((samplesIdx * 2), 0, bm)] = rInput[samplesIdx] * ingain;
        }
        for (int samplesIdx = 0; samplesIdx < buffLength * 2; samplesIdx++) {
            lInput_buff2x[std::clamp((samplesIdx), 0, bm)] = lInput_usLPF.processSample(lInput_buff2x[std::clamp((samplesIdx), 0, bm)]);
            rInput_buff2x[std::clamp((samplesIdx), 0, bm)] = rInput_usLPF.processSample(rInput_buff2x[std::clamp((samplesIdx), 0, bm)]);
        }
        for (int samplesIdx = 0; samplesIdx * 3 < buffLength * 2; samplesIdx++) {
            lInput_buff2x3d[std::clamp((samplesIdx), 0, bm)] = float(lInput_buff2x[std::clamp((samplesIdx * 3), 0, bm)] * 2.0);
            rInput_buff2x3d[std::clamp((samplesIdx), 0, bm)] = float(rInput_buff2x[std::clamp((samplesIdx * 3), 0, bm)] * 2.0);
            inBuffLength++;
        }
        inBuffLength = unsigned __int16(std::clamp((int(inBuffLength)), 0, bm));
    } else {
        for (int samplesIdx = 0; samplesIdx < buffLength; samplesIdx++) {
            auto ingain = juce::Decibels::decibelsToGain(sys_ingainSm.getNextValue() - 48.0f, -48.0f);
            lInput_buff2x3d[std::clamp((samplesIdx), 0, bm)] = lInput[samplesIdx] * ingain;
            rInput_buff2x3d[std::clamp((samplesIdx), 0, bm)] = rInput[samplesIdx] * ingain;
        }
        inBuffLength = unsigned __int16(std::clamp(buffLength, 0, bm));
    }

    for (int samplesIdx = 0; samplesIdx < inBuffLength; samplesIdx++) {
        //input and limitting
        adcl = max;
        adcr = max;
        if (lInput_buff2x3d[samplesIdx] <= -1.0f) {
            adcl = -max;
        } else if (lInput_buff2x3d[samplesIdx] <= 1.0f) {
            adcl = int(adcl * lInput_buff2x3d[samplesIdx]);
        }
        if (rInput_buff2x3d[samplesIdx] <= -1.0f) {
            adcr = -max;
        } else if (rInput_buff2x3d[samplesIdx] <= 1.0f) {
            adcr = int(adcr * rInput_buff2x3d[samplesIdx]);
        }

        //update user reg from system reg
        pot0 = int(max * pot0_ctrlSm.getNextValue());
        pot0_ = pot0;
        pot1 = int(max * pot1_ctrlSm.getNextValue());
        pot1_ = pot1;
        pot2 = int(max * pot2_ctrlSm.getNextValue());
        pot2_ = pot2;
        if (int(round(knob_name->load())) >= 2) {
            setRegister(std::clamp(int(round(knob_name->load())) - 2, 0, 31), int(max * pot3_ctrlSm.getNextValue()));
        }
        adcl_ = adcl;
        adcr_ = adcr;
        dram_pre = 0;

        if (sys_running == 2) {
            resetRegisters();
        }

        last_rda = 0;
        cho_reg = 0;

        //spn process
        for (int i = 0; i < 384; i += 3) {

            if (6 == sim_op[i] || 7 == sim_op[i] || 18 == sim_op[i]) {
                pacc_temp = pacc;
            }
            if (1 <= sim_op[i] && sim_op[i] <= 17 || 19 <= sim_op[i] && sim_op[i] <= 22) {
                pacc = acc;
            }

            switch (sim_op[i]) {
//          case 0: { //-------- NOP
//              break;
//          }
            case 1: { //-------- CLR
                acc = 0;
                break;
            }


            case 2: { //-------- LDAX
                acc = getRegister(sim_op[i + 1]);
                break;
            }
            case 3: { //-------- RDAX
                acc = acc + int(getRegister(sim_op[i + 1]) * float(sim_op[i + 2] * min));
                break;
            }
            case 4: { //-------- WRAX
                setRegister(sim_op[i + 1], acc);
                acc = int(acc * (float(sim_op[i + 2]) * min));
                break;
            }


            case 5: { //-------- RDFX
                acc = getRegister(sim_op[i + 1]) + int((acc - getRegister(sim_op[i + 1])) * (float(sim_op[i + 2]) * min));
                break;
            }
            case 6: { //-------- WRHX
                setRegister(sim_op[i + 1], acc);
                acc = int(float(acc) * (float(sim_op[i + 2]) * min)) + pacc_temp;
                break;
            }
            case 7: { //-------- WRLX
                setRegister(sim_op[i + 1], acc);
                acc = int(float(pacc_temp - acc) * (float(sim_op[i + 2]) * min)) + pacc_temp;
                break;
            }


            case 8: { //-------- MAXX
                if (abs(acc) > abs(int(getRegister(sim_op[i + 1]) * float(sim_op[i + 2] * min)))) {
                    acc = abs(acc);
                } else {
                    acc = abs(int(getRegister(sim_op[i + 1]) * float(sim_op[i + 2] * min)));
                }
                break;
            }
            case 9: { //-------- MULX
                acc = int(acc * float(getRegister(sim_op[i + 1]) * min));
                break;
            }
            case 10: { //-------- SOF
                acc = int(acc * float(sim_op[i + 1]) * min) + int(sim_op[i + 2]);
                break;
            }
            case 11: { //-------- LOG
                if (acc <= 0) {
                    if (float(sim_op[i + 1]) * min > 0.0) {
                        acc = -16777215;
                    } else {
                        acc = max;
                    }
                } else {
                    logvalue = (1.0 / 16.0) * log2(float(acc * min));
                    acc = int((logvalue * (float(sim_op[i + 1]) * min)) * max) + int(sim_op[i + 2]);
                }

                if (acc >= max) {
                    acc = max;
                } else if (acc <= -16777215) {
                    acc = -16777215;
                }
                break;
            }
            case 12: { //-------- EXP
                if (float(sim_op[i + 1]) * min > 0.05 || float(sim_op[i + 1]) * min < -0.05) {
                    if (acc > 0) {
                        acc = int(max * 0.33262);
                    }
                }
                acc = int(((exp2(acc * min * 16.0)) * (float(sim_op[i + 1]) * min)) * max) + int(sim_op[i + 2]);

                if (acc >= max) {
                    acc = max;
                } else if (acc <= -16777215) {
                    acc = -16777215;
                }
                break;
            }
            case 13: { //-------- ABSA
                acc = abs(acc);
                break;
            }


            case 14: { //-------- AND
                acc = (acc & int(sim_op[i + 1]));
                break;
            }
            case 15: { //-------- OR
                acc = (acc | int(sim_op[i + 1]));
                break;
            }
            case 16: { //-------- XOR
                acc = (acc ^ int(sim_op[i + 1]));
                break;
            }
            case 17: { //-------- NOT
                acc = (~acc);
                break;
            }


            case 18: { //-------- SKP
                switch (unsigned __int8(sim_op[i + 1])) {
                case 0: { //-- neg
                    if (acc < 0) {
                        i = i + (unsigned __int8(sim_op[i + 2]) * 3);
                    }
                    break;
                }
                case 1: { //-- gez
                    if (acc >= 0) {
                        i = i + (unsigned __int8(sim_op[i + 2]) * 3);
                    }
                    break;
                }
                case 2: { //-- zro
                    if (acc == 0) {
                        i = i + (unsigned __int8(sim_op[i + 2]) * 3);
                    }
                    break;
                }
                case 3: { //-- zrc
                    if (pacc_temp < 0) {
                        if (acc >= 0) {
                            i = i + (unsigned __int8(sim_op[i + 2]) * 3);
                        }
                    } else {
                        if (acc < 0) {
                            i = i + (unsigned __int8(sim_op[i + 2]) * 3);
                        }
                    }
                    break;
                }
                case 4: { //-- run
                    if (sys_running != 2) {
                        i = i + (unsigned __int8(sim_op[i + 2]) * 3);
                    }
                    break;
                }
                }
                break;
            }


            case 19: { //-------- RDA
                dram_pre = dram_base - unsigned __int16(sim_op[i + 1]);
                if (dram_pre < 0) {
                    dram_pre = dram_pre + 32768;
                }
                last_rda = int(dram[dram_pre]);
                acc = acc + int(float(dram[dram_pre]) * (float(sim_op[i + 2]) * min));
                break;
            }
            case 20: { //-------- WRA
                dram_pre = dram_base - unsigned __int16(sim_op[i + 1]);
                if (dram_pre < 0) {
                    dram_pre = dram_pre + 32768;
                }
                dram[dram_pre] = acc;
                acc = int(acc * float(sim_op[i + 2]) * min);
                break;
            }
            case 21: { //-------- RMPA
                //1 / (8388608 / 32767) = 0.0039061307907104
                dram_pre = dram_base - unsigned __int16(addr_ptr * 0.0039061307907104);
                if (dram_pre < 0) {
                    dram_pre = dram_pre + 32768;
                }
                acc = acc + int(float(dram[dram_pre]) * (float(sim_op[i + 1]) * min));
                break;
            }
            case 22: { //-------- WRAP
                dram_pre = dram_base - unsigned __int16(sim_op[i + 1]);
                if (dram_pre < 0) {
                    dram_pre = dram_pre + 32768;
                }
                dram[dram_pre] = acc;
                acc = int(acc * float(sim_op[i + 2]) * min) + last_rda;
                break;
            }


            case 23: { //-------- WLDS_0
                //8388608 / 511 = 16416.0626223092
                sin0_rate_ = int(float(sim_op[i + 1]) * 16416.0626223092);
                //8388608 / 32767 = 256.0078127384259
                sin0_range_ = int(float(sim_op[i + 2]) * 256.0078127384259);
                resetSin0();
                calcSin0();
                break;
            }
            case 24: { //-------- WLDS_1
                //8388608 / 511 = 16416.0626223092
                sin1_rate_ = int(float(sim_op[i + 1]) * 16416.0626223092);
                //8388608 / 32767 = 256.0078127384259
                sin1_range_ = int(float(sim_op[i + 2]) * 256.0078127384259);
                resetSin1();
                calcSin1();
                break;
            }


            case 25: { //-------- WLDR_0
                //8388608 / 32767 = 256.0078127384259
                rmp0_rate_ = int(float(sim_op[i + 1]) * 256.0078127384259);
                //8388608 / 4096 = 2048
                rmp0_range_ = int(sim_op[i + 2]);
                resetRmp0();
                calcRmp0();
                break;
            }
            case 26: { //-------- WLDR_1
                //8388608 / 32767 = 256.0078127384259
                rmp1_rate_ = int(float(sim_op[i + 1]) * 256.0078127384259);
                //8388608 / 4096 = 2048
                rmp1_range_ = int(sim_op[i + 2]);
                resetRmp1();
                calcRmp1();
                break;
            }


            case 27: { //-------- JAM
                resetRmp1();
                break;
            }
            case 28: { //-------- CHO RDAL
                if (sin0use == true || sin1use == true || rmp0use == true || rmp1use == true) {
                    switch (unsigned __int8(sim_op[i + 1])) {
                    case 0: { //-- sin0
                        acc = int(max * sin0lfo);
                        break;
                    }
                    case 1: { //-- sin1
                        acc = int(max * sin1lfo);
                        break;
                    }
                    case 8: { //-- cos0
                        acc = int(max * cos0lfo);
                        break;
                    }
                    case 9: { //-- cos1
                        acc = int(max * cos1lfo);
                        break;
                    }

                    case 2: { //-- rmp0
                        acc = int(max * rmp0lfo);
                        break;
                    }
                    case 3: { //-- rmp1
                        acc = int(max * rmp1lfo);
                        break;
                    }
                    }
                }
                break;
            }
            case 29: { //-------- CHO SOF
                if (rmp0use == true || rmp1use == true) {
                    switch (unsigned __int16(sim_op[i + 1])) {
                    case 362: { //-- rmp0, na | compc
                        acc = int(acc * rmp0tplfo) + int(sim_op[i + 2]);
                        break;
                    }
                    case 363: { //-- rmp1, na | compc
                        break;
                    }
                    }
                }
                break;
            }
            case 30: { //-------- CHO RDA
                if (sin0use == true || sin1use == true || rmp0use == true || rmp1use == true) {
                    switch (unsigned __int16(sim_op[i + 1])) {
                        //-------- sin0
                    case  60: { //-- sin0, sin | reg | compc
                        cho_reg = unsigned short(std::clamp(sim_op[i + 2], 0, 32767));
                        break;
                    }
                    case   0: { //-- sin0, sin
                        getChorus(cho_reg, unsigned __int16(sim_op[i + 2]), sin0lfo);
                        cho_reg = 0;
                        break;
                    }

                    case 100: { //-- sin0, sin | reg | compa
                        cho_reg = unsigned short(std::clamp(sim_op[i + 2], 0, 32767));
                        break;
                    }
                    case 120: { //-- sin0, sin | compc | compa
                        getChorus(cho_reg, unsigned __int16(sim_op[i + 2]), -sin0lfo);
                        cho_reg = 0;
                        break;
                    }

                    case  70: { //-- sin0, cos | reg | compc
                        cho_reg = unsigned short(std::clamp(sim_op[i + 2], 0, 32767));
                        break;
                    }
                    case  10: { //-- sin0, cos
                        getChorus(cho_reg, unsigned __int16(sim_op[i + 2]), cos0lfo);
                        cho_reg = 0;
                        break;
                    }

                    case 110: { //-- sin0, cos | reg | compa
                        cho_reg = unsigned short(std::clamp(sim_op[i + 2], 0, 32767));
                        break;
                    }
                    case 130: { //-- sin0, cos | compc | compa
                        getChorus(cho_reg, unsigned __int16(sim_op[i + 2]), -cos0lfo);
                        cho_reg = 0;
                        break;
                    }

                            //-------- sin1
                    case  61: { //-- sin1, sin | reg | compc
                        cho_reg = unsigned short(std::clamp(sim_op[i + 2], 0, 32767));
                        break;
                    }
                    case   1: { //-- sin1, sin
                        getChorus(cho_reg, unsigned __int16(sim_op[i + 2]), sin1lfo);
                        cho_reg = 0;
                        break;
                    }

                    case 101: { //-- sin0, sin | reg | compa
                        cho_reg = unsigned short(std::clamp(sim_op[i + 2], 0, 32767));
                        break;
                    }
                    case 121: { //-- sin0, sin | compc | compa
                        getChorus(cho_reg, unsigned __int16(sim_op[i + 2]), -sin1lfo);
                        cho_reg = 0;
                        break;
                    }

                    case  71: { //-- sin0, cos | reg | compc
                        cho_reg = unsigned short(std::clamp(sim_op[i + 2], 0, 32767));
                        break;
                    }
                    case  11: { //-- sin0, cos
                        getChorus(cho_reg, unsigned __int16(sim_op[i + 2]), cos1lfo);
                        cho_reg = 0;
                        break;
                    }

                    case 111: { //-- sin0, cos | reg | compa
                        cho_reg = unsigned short(std::clamp(sim_op[i + 2], 0, 32767));
                        break;
                    }
                    case 131: { //-- sin0, cos | compc | compa
                        getChorus(cho_reg, unsigned __int16(sim_op[i + 2]), -cos1lfo);
                        cho_reg = 0;
                        break;
                    }

                    case  62: { //-- rmp0, reg | compc
                        cho_reg = unsigned short(std::clamp(sim_op[i + 2], 0, 32767));
                        break;
                    }
                    case   2: { //-- rmp0, 0
                        getPitch(cho_reg, unsigned __int16(sim_op[i + 2]), rmp0lfo);
                        break;
                    }
                    case 202: { //-- rmp0, compc | rptr2
                        break;
                    }
                    case 162: { //-- rmp0, rptr2
                        getPitch(cho_reg, unsigned __int16(sim_op[i + 2]), rmp0sublfo);
                        break;
                    }

                    case 322: { //-- rmp0, na
                        dram_pre = dram_base - unsigned __int16(sim_op[i + 2]);
                        if (dram_pre < 0) {
                            dram_pre = dram_pre + 32768;
                        }
                        acc = acc + int(float(dram[dram_pre]) * (1.0f - rmp0tplfo));
                        break;
                    }
                    }
                }
                break;
            }
            }
        }

        //update system reg from user reg
        clampRegisters();

        if (sys_running == 2) {
            sys_running = 1;
        }

        sin0_rate = sin0_rate_;
        sin0_range = sin0_range_;
        sin1_rate = sin1_rate_;
        sin1_range = sin1_range_;

        rmp0_rate = rmp0_rate_;
        rmp0_range = rmp0_range_;
        rmp1_rate = rmp1_rate_;
        rmp1_range = rmp1_range_;
        if (rmp0_range != 1024 && rmp0_range != 2048 && rmp0_range != 4096) {
            rmp0_range = 512;
        }
        if (rmp1_range != 1024 && rmp1_range != 2048 && rmp1_range != 4096) {
            rmp1_range = 512;
        }

        dacl = dacl_;
        dacr = dacr_;

        addr_ptr = addr_ptr_;

        if (dram_base >= 32767) {
            dram_base = 0;
        } else {
            dram_base++;
        }

        if (sin0use == true) {
            calcSin0();
        }
        if (sin1use == true) {
            calcSin1();
        }

        if (rmp0use == true) {
            calcRmp0();
        }
        if (rmp1use == true) {
            calcRmp1();
        }

        //get waveform
        if (buffMax0 < float(getRegister(std::clamp(int(round(sys_wf0_sel->load())), 0, 47)) * min)) {
            buffMax0 = float(getRegister(std::clamp(int(round(sys_wf0_sel->load())), 0, 47)) * min);
        }
        if (buffMin0 > float(getRegister(std::clamp(int(round(sys_wf0_sel->load())), 0, 47)) * min)) {
            buffMin0 = float(getRegister(std::clamp(int(round(sys_wf0_sel->load())), 0, 47)) * min);
        }
        
        if (buffMax1 < float(getRegister(std::clamp(int(round(sys_wf1_sel->load())), 0, 47)) * min)) {
            buffMax1 = float(getRegister(std::clamp(int(round(sys_wf1_sel->load())), 0, 47)) * min);
        }
        if (buffMin1 > float(getRegister(std::clamp(int(round(sys_wf1_sel->load())), 0, 47)) * min)) {
            buffMin1 = float(getRegister(std::clamp(int(round(sys_wf1_sel->load())), 0, 47)) * min);
        }

        if (buffMax2 < float(getRegister(std::clamp(int(round(sys_wf2_sel->load())), 0, 47)) * min)) {
            buffMax2 = float(getRegister(std::clamp(int(round(sys_wf2_sel->load())), 0, 47)) * min);
        }
        if (buffMin2 > float(getRegister(std::clamp(int(round(sys_wf2_sel->load())), 0, 47)) * min)) {
            buffMin2 = float(getRegister(std::clamp(int(round(sys_wf2_sel->load())), 0, 47)) * min);
        }
        
        if (buffMax3 < float(getRegister(std::clamp(int(round(sys_wf3_sel->load())), 0, 47)) * min)) {
            buffMax3 = float(getRegister(std::clamp(int(round(sys_wf3_sel->load())), 0, 47)) * min);
        }
        if (buffMin3 > float(getRegister(std::clamp(int(round(sys_wf3_sel->load())), 0, 47)) * min)) {
            buffMin3 = float(getRegister(std::clamp(int(round(sys_wf3_sel->load())), 0, 47)) * min);
        }

        //output and limitting
        lOutput_buff[samplesIdx] = float(dacl * min);
        rOutput_buff[samplesIdx] = float(dacr * min);
        if (lOutput_buff[samplesIdx] >= 1.0f) {
            lOutput_buff[samplesIdx] = 1;
        } else if (lOutput_buff[samplesIdx] <= -1.0f) {
            lOutput_buff[samplesIdx] = -1;
        }
        if (rOutput_buff[samplesIdx] >= 1.0f) {
            rOutput_buff[samplesIdx] = 1;
        } else if (rOutput_buff[samplesIdx] <= -1.0f) {
            rOutput_buff[samplesIdx] = -1;
        }
    }

    if (sys_rs->load() != 0.0f) {
        for (int samplesIdx = 0; samplesIdx < inBuffLength; samplesIdx++) {
            lOutput_buff3x[std::clamp((samplesIdx * 3), 0, bm)] = lOutput_buff[std::clamp((samplesIdx), 0, bm)];
            rOutput_buff3x[std::clamp((samplesIdx * 3), 0, bm)] = rOutput_buff[std::clamp((samplesIdx), 0, bm)];
        }
        for (int samplesIdx = 0; samplesIdx < inBuffLength * 3; samplesIdx++) {
            lOutput_buff3x[std::clamp((samplesIdx), 0, bm)] = lOutput_usLPF.processSample(lOutput_buff3x[std::clamp((samplesIdx), 0, bm)]);
            rOutput_buff3x[std::clamp((samplesIdx), 0, bm)] = rOutput_usLPF.processSample(rOutput_buff3x[std::clamp((samplesIdx), 0, bm)]);
        }
    }
    } else {
        lOutput_buff.fill(0.0f);
        rOutput_buff.fill(0.0f);
        lOutput_buff3x.fill(0.0f);
        rOutput_buff3x.fill(0.0f);
        buffMax0 = 0.0f;
        buffMin0 = 0.0f;
        buffMax1 = 0.0f;
        buffMin1 = 0.0f;
        buffMax2 = 0.0f;
        buffMin2 = 0.0f;
        buffMax3 = 0.0f;
        buffMin3 = 0.0f;
    }

    for (int samplesIdx = 0; samplesIdx < buffLength; samplesIdx++) {
        auto bypassgain = sys_bypassSm.getNextValue();
        auto mutegain = sys_muteSm.getNextValue();
        auto outgain = juce::Decibels::decibelsToGain(sys_outgainSm.getNextValue() - 48.0f, -48.0f);
        outgain = outgain * bypassgain;

        float lBypass = lInput[samplesIdx];
        float rBypass = rInput[samplesIdx];

        float lDry = 0.0f;
        float rDry = 0.0f;
        if (int(round(knob_name->load())) == 1) {
            lDry = lBypass * pot3_ctrlSm.getNextValue();
            rDry = rBypass * pot3_ctrlSm.getNextValue();
        }

        if (sys_rs->load() != 0.0f) {
            lOutput[samplesIdx] = float(((lOutput_buff3x[std::clamp((samplesIdx * 2), 0, bm)] + lDry) * outgain * 3.0) + (lBypass * (1 - bypassgain))) * mutegain;
            rOutput[samplesIdx] = float(((rOutput_buff3x[std::clamp((samplesIdx * 2), 0, bm)] + rDry) * outgain * 3.0) + (rBypass * (1 - bypassgain))) * mutegain;
        } else {
            lOutput[samplesIdx] = float(((lOutput_buff[std::clamp((samplesIdx), 0, bm)] + lDry) * outgain) + (lBypass * (1 - bypassgain))) * mutegain;
            rOutput[samplesIdx] = float(((rOutput_buff[std::clamp((samplesIdx), 0, bm)] + rDry) * outgain) + (rBypass * (1 - bypassgain))) * mutegain;
        }
    }


    if (sys_running > 0) {
    if (sys_wf_send->load() == 0.0f) {
        getAPVTS().getParameter("sys_wf_send")->setValueNotifyingHost(1.0f);
    } else {
        getAPVTS().getParameter("sys_wf_send")->setValueNotifyingHost(0.0f);
    }
    sys_wf0_max = (buffMax0 * 0.5f) + 0.5f;
    sys_wf0_min = (buffMin0 * 0.5f) + 0.5f;
    sys_wf1_max = (buffMax1 * 0.5f) + 0.5f;
    sys_wf1_min = (buffMin1 * 0.5f) + 0.5f;
    sys_wf2_max = (buffMax2 * 0.5f) + 0.5f;
    sys_wf2_min = (buffMin2 * 0.5f) + 0.5f;
    sys_wf3_max = (buffMax3 * 0.5f) + 0.5f;
    sys_wf3_min = (buffMin3 * 0.5f) + 0.5f;
    }
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this,
        sys_wf0_max, sys_wf0_min, sys_wf1_max, sys_wf1_min, sys_wf2_max, sys_wf2_min, sys_wf3_max, sys_wf3_min,
        ui_font, sys_file, sys_edited, sys_running, sys_code, sim_op);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
    
    auto state = parameters->copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    xml->setAttribute("font", ui_font);
    xml->setAttribute("file", sys_file.getFullPathName());
    xml->setAttribute("edited", sys_edited);
    xml->setAttribute("code", sys_code);
    xml->setAttribute("run", sys_running);
    copyXmlToBinary (*xml, destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
    
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters->state.getType()))
            parameters->replaceState (juce::ValueTree::fromXml (*xmlState));
            ui_font = xmlState->getStringAttribute("font");
            sys_file = xmlState->getStringAttribute("file");
            sys_edited = xmlState->getStringAttribute("edited");
            sys_code = xmlState->getStringAttribute("code");
            sys_running = unsigned __int8(xmlState->getIntAttribute("run"));
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    // It will move should not const.
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_ingain", 1 },
        "Input Gain", juce::NormalisableRange<float>(0.0f, 100.0f), 48.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_outgain", 1 },
        "Output Gain", juce::NormalisableRange<float>(0.0f, 100.0f), 48.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "pot0_ctrl", 1 },
        "Pot0", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "pot1_ctrl", 1 },
        "Pot1", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "pot2_ctrl", 1 },
        "Pot2", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "pot3_ctrl", 1 },
        "Pot3", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_bypass", 1 },
        "Bypass", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_mute", 1 },
        "Mute", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_wf_send", 1 },
        "WF Send", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_wf0_sel", 1 },
        "WF0 Select", juce::NormalisableRange<float>(0.0f, 100.0f), 32.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_wf1_sel", 1 },
        "WF1 Select", juce::NormalisableRange<float>(0.0f, 100.0f), 33.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_wf2_sel", 1 },
        "WF2 Select", juce::NormalisableRange<float>(0.0f, 100.0f), 34.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_wf3_sel", 1 },
        "WF3 Select", juce::NormalisableRange<float>(0.0f, 100.0f), 35.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "knob_name", 1 },
        "WF3 Select", juce::NormalisableRange<float>(0.0f, 100.0f), 0.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "ui_dark", 1 },
        "Dark UI", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "ui_size", 1 },
        "UI Size", juce::NormalisableRange<float>(0.0f, 10.0f), 7.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "ui_lh", 1 },
        "UI Line Height", juce::NormalisableRange<float>(0.0f, 100.0f), 1.0f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "ui_fs", 1 },
        "UI Font Size", juce::NormalisableRange<float>(0.0f, 100.0f), 12.0f));

    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "sys_rs", 1 },
        "Resampling", juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
    
/*  parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ "invertPhase", 1 },
        "Invert Phase",
        false));*/
    return juce::AudioProcessorValueTreeState::ParameterLayout{ parameters.begin(), parameters.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}