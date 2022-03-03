/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KadenzeChorusFlangerAudioProcessor::KadenzeChorusFlangerAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    
    //Construct and Add Parameters
    
    addParameter(mDryWetParameter = new juce::AudioParameterFloat("drywet",
                                "Dry Wet",
                                0.0,
                                1.0,
                                0.5));
    
    addParameter(mDepthParameter = new juce::AudioParameterFloat("depth",
                                "Depth",
                                0.0,
                                1.0,
                                0.5));
    
    addParameter(mRateParameter = new juce::AudioParameterFloat("rate",
                                "Rate",
                                0.1f,
                                20.f,
                                10.f));
    
    addParameter(mPhaseOffsetParameter = new juce::AudioParameterFloat("phaseoffset",
                                "Phase Offset",
                                0.0f,
                                1.f,
                                0.f));
    
    addParameter(mFeedbackParameter = new juce::AudioParameterFloat("feedback",
                                "Feedback",
                                0,
                                0.98,
                                0.5));
    
    addParameter(mType = new juce::AudioParameterInt("type",
                                "Type",
                                0,
                                1,
                                0));
    
    //initialize data to default values

    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    
    mFeedBackLeft = 0;
    mFeedBackRight = 0;
    
    mLFOPhase = 0;
    
}

KadenzeChorusFlangerAudioProcessor::~KadenzeChorusFlangerAudioProcessor()
{
}

//==============================================================================
const juce::String KadenzeChorusFlangerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KadenzeChorusFlangerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KadenzeChorusFlangerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KadenzeChorusFlangerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KadenzeChorusFlangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KadenzeChorusFlangerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KadenzeChorusFlangerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KadenzeChorusFlangerAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String KadenzeChorusFlangerAudioProcessor::getProgramName (int index)
{
    return {};
}

void KadenzeChorusFlangerAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //initialize data for current sample rate, reset phase write heads
    //initialize phase
    mLFOPhase = 0;
    
    //calculate circular buffer length
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
    
    //initialize the left buffer
    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float[mCircularBufferLength];
    }
    
    //clear junk data left channel
    juce::zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));
    //initialize right buffer
    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[mCircularBufferLength];
    }
    // clear right channel junk
    juce::zeromem(mCircularBufferRight, mCircularBufferLength * sizeof(float));
    
    //init write head 0
    mCircularBufferWriteHead = 0;
    

}

void KadenzeChorusFlangerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool KadenzeChorusFlangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
#endif

void KadenzeChorusFlangerAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    DBG("DRY WET: " << *mDryWetParameter);
    DBG("DEPTH: " << *mDepthParameter);
    DBG("RATE: " << *mRateParameter);
    DBG("PHASE: " << *mPhaseOffsetParameter);
    DBG("FEEDBACK: " << *mFeedbackParameter);
    DBG("TYPE: " << (int)*mType);

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // obtain left and right audio data pointers
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    
    //iterate through all samples in buffer
    for (int i = 0; i < buffer.getNumSamples(); i++) {
        
        //write into circular buffer
        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedBackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedBackRight;
        
        //generate left LFO output
        float lfoOutLeft = sin(2*M_PI * mLFOPhase);
        
        //calculate right channel LFO phase
        float lfoPhaseRight = mLFOPhase + *mPhaseOffsetParameter;
        
        if (lfoPhaseRight > 1) {
            lfoPhaseRight -= 1;
        }
        
        //generate right LFO output
        float lfoOutRight = sin(2*M_PI * lfoPhaseRight);
        
        
        //moving lfo phase forward
        mLFOPhase += *mRateParameter / getSampleRate();
        
        if (mLFOPhase > 1) {
            mLFOPhase -= 1;
        }
        
        //control LFO depth
        lfoOutLeft *= *mDepthParameter;
        lfoOutRight *= *mDepthParameter;
        
        float lfoOutMappedLeft = 0;
        float lfoOutMappedRight = 0;
        //map lfo output to desired delay time
        //Chorus
        if (*mType == 0) {
            lfoOutMappedLeft = juce::jmap((float)lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);
            lfoOutMappedRight = juce::jmap((float)lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);
            //else block is flanger
        } else {
            lfoOutMappedLeft = juce::jmap((float)lfoOutLeft, -1.f, 1.f, 0.001f, 0.005f);
            lfoOutMappedRight = juce::jmap((float)lfoOutRight, -1.f, 1.f, 0.001f, 0.005f);
        }
        
        //calculate the delay lenght in samples
        float delayTimeSamplesLeft = getSampleRate() * lfoOutMappedLeft;
        float delayTimeSamplesRight = getSampleRate() * lfoOutMappedRight;
        
        //calculate left read head position
        float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;
        
        if (delayReadHeadLeft < 0) {
            delayReadHeadLeft += mCircularBufferLength;
        }
        
        //calculate right read head position
        float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;
        
        if (delayReadHeadRight < 0) {
            delayReadHeadRight += mCircularBufferLength;
        }
        
        //calculating lin interp points for left channel
        int readHeadLeft_x = (int)delayReadHeadLeft;
        int readHeadLeft_y = readHeadLeft_x + 1;
        
        float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;
        
        if (readHeadLeft_y >= mCircularBufferLength){
            readHeadLeft_y -= mCircularBufferLength;
        }
        
        //calculate lin interp points for right channel
        int readHeadRight_x = (int)delayReadHeadRight;
        int readHeadRight_y = readHeadRight_x + 1;
        float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;
        
        if (readHeadRight_y >= mCircularBufferLength){
            readHeadRight_y -= mCircularBufferLength;
        }
        
        //generate samples
        float delay_sample_left = KadenzeChorusFlangerAudioProcessor::lin_interp(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_y], readHeadFloatLeft);
        float delay_sample_right = KadenzeChorusFlangerAudioProcessor::lin_interp(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_y], readHeadFloatRight);
        
        
        
        mFeedBackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedBackRight = delay_sample_right * *mFeedbackParameter;


        mCircularBufferWriteHead++;
        
        if (mCircularBufferWriteHead >= mCircularBufferLength){
            mCircularBufferWriteHead = 0;
        }
        
        float dryAmount = 1 - *mDryWetParameter;
        float wetAmount = *mDryWetParameter;
        
        buffer.setSample(0, i, buffer.getSample(0, i) * dryAmount + delay_sample_left * wetAmount);
        buffer.setSample(1, i, buffer.getSample(1, i) * dryAmount + delay_sample_right * wetAmount);
    }
}

//==============================================================================
bool KadenzeChorusFlangerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* KadenzeChorusFlangerAudioProcessor::createEditor()
{
    return new KadenzeChorusFlangerAudioProcessorEditor (*this);
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    
    std::unique_ptr<juce::XmlElement> xml(new juce::XmlElement("FlangerChorus"));
    
    xml->setAttribute("DryWet", *mDryWetParameter);
    xml->setAttribute("Depth", *mDepthParameter);
    xml->setAttribute("Rate", *mRateParameter);
    xml->setAttribute("PhaseOffset", *mPhaseOffsetParameter);
    xml->setAttribute("FeedBack", *mFeedbackParameter);
    xml->setAttribute("Type", *mType);
    
    copyXmlToBinary(*xml, destData);
    
}

void KadenzeChorusFlangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    
    if (xml.get() != nullptr && xml->hasTagName("FlangerChorus")) {
        *mDryWetParameter = xml->getDoubleAttribute("DryWet");
        *mDepthParameter = xml->getDoubleAttribute("Depth");
        *mRateParameter = xml->getDoubleAttribute("Rate");
        *mPhaseOffsetParameter = xml->getDoubleAttribute("PhaseOffset");
        *mFeedbackParameter = xml->getDoubleAttribute("FeedBack");
        
        *mType = xml->getIntAttribute("Type");

        

    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KadenzeChorusFlangerAudioProcessor();
}

float KadenzeChorusFlangerAudioProcessor::lin_interp(float inSampleX, float inSampleY, float inFloatPhase)
{
    return (1 - inFloatPhase) * inSampleX + inFloatPhase * inSampleY;
}
