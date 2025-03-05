#include "MainComponent.h"

using namespace juce;

//==============================================================================
MainComponent::MainComponent() : openButton("Open"), playButton("Play"), stopButton("Stop")
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (200, 150);
    openButton.onClick = [this] { openButtonClicked(); };
    addAndMakeVisible(&openButton);

    playButton.onClick = [this] {; };
    playButton.setColour(TextButton::buttonColourId, Colours::green);
    playButton.setEnabled(true);
    addAndMakeVisible(&playButton);

    stopButton.onClick = [this] {; };
    stopButton.setColour(TextButton::buttonColourId, Colours::red);
    stopButton.setEnabled(false);
    addAndMakeVisible(&stopButton);

    formatManager.registerBasicFormats();

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
}

void MainComponent::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser>("Select a Wave file to play...",
        juce::File{},
        "*.wav");                     // [7]
    auto chooserFlags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)     // [8]
    {
        auto file = fc.getResult();

        if (file != juce::File{})                                                // [9]
        {
            auto* reader = formatManager.createReaderFor(file);                 // [10]

            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);   // [11]
                //transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
                //playButton.setEnabled(true);                                                      // [13]
                playSource.reset(newSource.release());                                          // [14]
                DBG(reader->getFormatName());
            }
        }
    });
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    openButton.setBounds(10, 10, getWidth() - 20, 30);
    playButton.setBounds(10, 50, getWidth() - 20, 30);
    stopButton.setBounds(10, 90, getWidth() - 20, 30);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
