#pragma once

#include <JuceHeader.h>

using namespace juce;

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent, private juce::Slider::Listener, private juce::ChangeListener, private juce::Timer
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint(juce::Graphics& g) override;
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

    void sliderButtonClicked();

    void parseButtonClicked();

    void saveButtonClicked();

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
    TextButton sliderButton;
    TextButton parseButton;
    TextButton saveButton;

    Label decibelLabel;

    Label distortionLabel;

    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;

    Slider ReverbDial;

    Slider CompressionDial;

    Slider decibelSlider;

    Slider distortionSlider;

    juce::File myVocals;
    juce::File myBass;
    juce::File myOther;
    juce::File myDrums;
    juce::File currentFile;

    float level = 0.0f;

    float startEffect = 0.0f;
    float stopEffect = 0.0f;

    juce::Slider scrubSlider;
    juce::Label scrubLabel;

    std::string myPython = "";

    juce::ComboBox reverbMenu;
    void reverbMenuChanged();

    juce::ComboBox compressionMenu;
    void compressionMenuChanged();

    juce::ComboBox instrumentMenu;
    void instrumentMenuChanged();

    enum ReverbEffects {
        room_size = 1,
        damping,
        wet_level,
        dry_level,
        width,
        freeze_mode,
        numberOfReverb
    };

    enum CompressionEffects {
        thresholdDB = numberOfReverb,
        ratio,
        attackMS,
        releaseMS,
        numberOfCompression
    };

    enum Instruments {
        fullSong = numberOfCompression,
        bass,
        drums,
        vocals,
        other
    };

    float myRoomSize = 0.5;
    float myDamping = 0.5;
    float myWetLevel = 0.33;
    float myDryLevel = 0.4;
    float myWidth = 1.0;
    float myFreezeMode = 0.0;
    float myThresholdDB = 0.0;
    float myRatio = 1;
    float myAttackMS = 1;
    float myReleaseMS = 100;
    int prevReverb = 1;
    int prevCompression = thresholdDB;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
