/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KadenzeChorusFlangerAudioProcessorEditor::KadenzeChorusFlangerAudioProcessorEditor (KadenzeChorusFlangerAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (500, 200);
    
    auto& params = processor.getParameters();
    
    // Dry Wet SLider
    juce::AudioParameterFloat* dryWetParameter = (juce::AudioParameterFloat*)params.getUnchecked(0);
    mDryWetSlider.setBounds(0, 0, 100, 100);
    mDryWetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    
    mDryWetSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    
    mDryWetSlider.setRange(dryWetParameter->range.start, dryWetParameter->range.end);
    mDryWetSlider.setValue(*dryWetParameter);
    addAndMakeVisible(mDryWetSlider);
    
    mDryWetSlider.onValueChange = [this, dryWetParameter] { *dryWetParameter = mDryWetSlider.getValue(); };
    mDryWetSlider.onDragStart = [dryWetParameter] { dryWetParameter->beginChangeGesture(); };
    mDryWetSlider.onDragEnd = [dryWetParameter] { dryWetParameter->endChangeGesture(); };
    
    // Depth SLider
    juce::AudioParameterFloat* depthParameter = (juce::AudioParameterFloat*)params.getUnchecked(1);
    mDepthSlider.setBounds(100, 0, 100, 100);
    mDepthSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    
    mDepthSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    
    mDepthSlider.setRange(depthParameter->range.start, depthParameter->range.end);
    mDepthSlider.setValue(*depthParameter);
    addAndMakeVisible(mDepthSlider);
    
    mDepthSlider.onValueChange = [this, depthParameter] { *depthParameter = mDepthSlider.getValue(); };
    mDepthSlider.onDragStart = [depthParameter] { depthParameter->beginChangeGesture(); };
    mDepthSlider.onDragEnd = [depthParameter] { depthParameter->endChangeGesture(); };
    
    // Rate Slider
    juce::AudioParameterFloat* rateParameter = (juce::AudioParameterFloat*)params.getUnchecked(2);
    mRateSlider.setBounds(200, 0, 100, 100);
    mRateSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    
    mRateSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    
    mRateSlider.setRange(rateParameter->range.start, rateParameter->range.end);
    mRateSlider.setValue(*rateParameter);
    addAndMakeVisible(mRateSlider);
    
    mRateSlider.onValueChange = [this, rateParameter] { *rateParameter = mRateSlider.getValue(); };
    mRateSlider.onDragStart = [rateParameter] { rateParameter->beginChangeGesture(); };
    mRateSlider.onDragEnd = [rateParameter] { rateParameter->endChangeGesture(); };
    
    // Phase Offset Slider
    juce::AudioParameterFloat* phaseParameter = (juce::AudioParameterFloat*)params.getUnchecked(3);
    mPhaseOffsetSlider.setBounds(300, 0, 100, 100);
    mPhaseOffsetSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    
    mPhaseOffsetSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    
    mPhaseOffsetSlider.setRange(phaseParameter->range.start, phaseParameter->range.end);
    mPhaseOffsetSlider.setValue(*phaseParameter);
    addAndMakeVisible(mPhaseOffsetSlider);
    
    mPhaseOffsetSlider.onValueChange = [this, phaseParameter] { *phaseParameter = mPhaseOffsetSlider.getValue(); };
    mPhaseOffsetSlider.onDragStart = [phaseParameter] { phaseParameter->beginChangeGesture(); };
    mPhaseOffsetSlider.onDragEnd = [phaseParameter] { phaseParameter->endChangeGesture(); };
    
    //Feedback Slider
    juce::AudioParameterFloat* feedbackParameter = (juce::AudioParameterFloat*)params.getUnchecked(4);
    mFeedbackSlider.setBounds(400, 0, 100, 100);
    mFeedbackSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    mFeedbackSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    mFeedbackSlider.setRange(feedbackParameter->range.start, feedbackParameter->range.end);
    mFeedbackSlider.setValue(*feedbackParameter);
    addAndMakeVisible(mFeedbackSlider);
    
    mFeedbackSlider.onValueChange = [this, feedbackParameter] { *feedbackParameter = mFeedbackSlider.getValue(); };
    mFeedbackSlider.onDragStart = [feedbackParameter] { feedbackParameter->beginChangeGesture(); };
    mFeedbackSlider.onDragEnd = [feedbackParameter] { feedbackParameter->endChangeGesture(); };
    
    //Type
    juce::AudioParameterInt* typeParameter = (juce::AudioParameterInt*)params.getUnchecked(5);
    mType.setBounds(0, 100, 100, 30);
    
    mType.addItem("Chorus", 1);
    mType.addItem("Flanger", 2);
    addAndMakeVisible(mType);
    
    mType.onChange = [this, typeParameter] {
        typeParameter->beginChangeGesture();
        *typeParameter = mType.getSelectedItemIndex();
        typeParameter->endChangeGesture();
    };
    
    mType.setSelectedItemIndex(*typeParameter);
}

KadenzeChorusFlangerAudioProcessorEditor::~KadenzeChorusFlangerAudioProcessorEditor()
{
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Kadenze Chorus Flanger", getLocalBounds(), juce::Justification::centred, 1);
}

void KadenzeChorusFlangerAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}
