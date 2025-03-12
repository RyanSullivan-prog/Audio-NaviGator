#pragma once

#include <JuceHeader.h>

using namespace juce;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    enum TransportState
    {
        Stopped,
        Starting,
        Stopping
    };

    TransportState state;
    
    void openButtonClicked();

    void playButtonClicked();

    void stopButtonClicked();

    void bassButtonClicked();

    void drumsButtonClicked();

    void vocalsButtonClicked();

    void otherButtonClicked();

    void transportStateChanged(TransportState newState);

    AudioFormatManager formatManager;

    std::unique_ptr<juce::FileChooser> chooser;

    std::unique_ptr<AudioFormatReaderSource> playSource;

    AudioTransportSource transport;

    std::string myPathToInstruments;

    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    TextButton bassButton;
    TextButton drumsButton;
    TextButton vocalsButton;
    TextButton otherButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
