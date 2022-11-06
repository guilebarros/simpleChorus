/*
  ==============================================================================
                              Guilherme Barros - 2022
  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleChorusAudioProcessor::SimpleChorusAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
treeState(*this, nullptr, "Parameters", createParameterLayout())

#endif
{
    // chorus parameters listeners
    treeState.addParameterListener("rate", this);
    treeState.addParameterListener("depth", this);
    treeState.addParameterListener("centreDelay", this);
    treeState.addParameterListener("feedback", this);
    treeState.addParameterListener("mix", this);
}

SimpleChorusAudioProcessor::~SimpleChorusAudioProcessor()
{
    treeState.removeParameterListener("rate", this);
    treeState.removeParameterListener("depth", this);
    treeState.removeParameterListener("centreDelay", this);
    treeState.removeParameterListener("feedback", this);
    treeState.removeParameterListener("mix", this);
}

//==============================================================================


juce::AudioProcessorValueTreeState::ParameterLayout SimpleChorusAudioProcessor::createParameterLayout()
{
    // parameters vector (params)
    
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
        
    // chorus parameters
        
    auto pRate = std::make_unique<juce::AudioParameterFloat>("rate", "Rate", 0.1f, 100.0f, 1.0f);
    auto pDepth = std::make_unique<juce::AudioParameterFloat>("depth", "Depth", 0.0f, 1.0f, 0.0f);
    auto pCentreDelay = std::make_unique<juce::AudioParameterFloat>("centreDelay", "Center Delay", 1.0f, 100.0f, 1.0f);
    auto pFeedback = std::make_unique<juce::AudioParameterFloat>("feedback", "Feedback", -1.0f, 1.0f, 1.0f);
    auto pMix = std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 0.5f);

    params.push_back(std::move(pRate));
    params.push_back(std::move(pDepth));
    params.push_back(std::move(pCentreDelay));
    params.push_back(std::move(pFeedback));
    params.push_back(std::move(pMix));

    return { params.begin(), params.end() };
}

void SimpleChorusAudioProcessor::updateParameters()
{
    chorusModule.setRate(treeState.getRawParameterValue("rate")->load());
    chorusModule.setDepth(treeState.getRawParameterValue("depth")->load());
    chorusModule.setCentreDelay(treeState.getRawParameterValue("centreDelay")->load());
    chorusModule.setFeedback(treeState.getRawParameterValue("feedback")->load());
    chorusModule.setMix(treeState.getRawParameterValue("mix")->load());
}

void SimpleChorusAudioProcessor::parameterChanged (const juce::String& parameterID, float newValue)
{
    updateParameters();
}
//=======================================================
const juce::String SimpleChorusAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleChorusAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleChorusAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleChorusAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleChorusAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleChorusAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleChorusAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleChorusAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleChorusAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleChorusAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleChorusAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.sampleRate = sampleRate;
    spec.numChannels = getTotalNumOutputChannels();
    
    chorusModule.prepare(spec);
    
    updateParameters();

}

void SimpleChorusAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleChorusAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SimpleChorusAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    juce::dsp::AudioBlock<float> block { buffer };
    chorusModule.process(juce::dsp::ProcessContextReplacing<float>(block));
}

//==============================================================================
bool SimpleChorusAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleChorusAudioProcessor::createEditor()
{
    //return new SimpleChorusAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleChorusAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream(stream);
}

void SimpleChorusAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree  = juce::ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid())
    {
        treeState.state = tree;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleChorusAudioProcessor();
}
