#pragma once

#include <JuceHeader.h>

using namespace juce;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent, private juce::Slider::Listener, private juce::ChangeListener, private juce::Timer
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

    void paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);
    void paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds);

private:
    //==============================================================================
    // Your private member variables go here...
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Stopping,
        Pausing
    };

    TransportState state;
    
    void openButtonClicked();

    void playButtonClicked();

    void stopButtonClicked();

    void bassButtonClicked();

    void drumsButtonClicked();

    void vocalsButtonClicked();

    void otherButtonClicked();

    void songButtonClicked();
    
    void sliderButtonClicked();

    void process();

    void transportStateChanged(TransportState newState);

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    void sliderValueChanged(juce::Slider* slider) override;

    void timerCallback() override;

    AudioFormatManager formatManager;

    std::unique_ptr<juce::FileChooser> chooser;

    std::unique_ptr<AudioFormatReaderSource> playSource;

    AudioTransportSource transport;

    std::string myPathToInstruments;

    std::string originalFilePath;

    juce::File originalFile;

    juce::File myInstruments;

    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    TextButton bassButton;
    TextButton drumsButton;
    TextButton vocalsButton;
    TextButton otherButton;
    TextButton songButton;
    TextButton sliderButton;

    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;

    Slider dial1;

    Slider decibelSlider;

    Slider startTimeSlider;

    Slider stopTimeSlider;

    juce::File myVocals;
    juce::File myBass;
    juce::File myOther;
    juce::File myDrums;

    float level = 0.0f;

    float startEffect = 0.0f;
    float stopEffect = 0.0f;

    juce::Slider scrubSlider;
    juce::Label scrubLabel;

    dsp::ProcessSpec mySpec;
    dsp::Reverb myReverb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
