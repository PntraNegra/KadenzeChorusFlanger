/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define MAX_DELAY_TIME 2

//==============================================================================
/**
*/
class KadenzeChorusFlangerAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    KadenzeChorusFlangerAudioProcessor();
    ~KadenzeChorusFlangerAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    
    float lin_interp(float inSampleX, float inSampleY, float inFloatPhase);

private:
    

    
    //parameter declatarions
    juce::AudioParameterFloat* mDryWetParameter;
    juce::AudioParameterFloat* mDepthParameter;
    juce::AudioParameterFloat* mRateParameter;
    juce::AudioParameterFloat* mPhaseOffsetParameter;
    juce::AudioParameterFloat* mFeedbackParameter;
    juce::AudioParameterInt* mType;
    
    //circular buffer data
    float* mCircularBufferLeft;
    float* mCircularBufferRight;
    
    int mCircularBufferWriteHead;
    int mCircularBufferLength;
    
    float mFeedBackLeft;
    float mFeedBackRight;
    
    //LFO data
    float mLFOPhase;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KadenzeChorusFlangerAudioProcessor)
};
