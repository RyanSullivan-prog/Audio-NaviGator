#include "MainComponent.h"

using namespace juce;

// file selecter and audio player created using https://docs.juce.com/master/tutorial_playing_sound_files.html, https://www.youtube.com/watch?v=eB6S8iWvx2k, and https://www.youtube.com/watch?v=vVnu-L712ho

//==============================================================================
MainComponent::MainComponent() : state(Stopped), openButton("Open"), playButton("Play"), stopButton("Stop"), bassButton("Bass"), drumsButton("Drums"), vocalsButton("Vocals"), otherButton("Other")
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (200, 400);
    openButton.onClick = [this] { openButtonClicked(); };
    addAndMakeVisible(&openButton);

    playButton.onClick = [this] {playButtonClicked(); };
    playButton.setColour(TextButton::buttonColourId, Colours::green);
    playButton.setEnabled(false);
    addAndMakeVisible(&playButton);

    stopButton.onClick = [this] {stopButtonClicked(); };
    stopButton.setColour(TextButton::buttonColourId, Colours::red);
    stopButton.setEnabled(false);
    addAndMakeVisible(&stopButton);

    bassButton.onClick = [this] {bassButtonClicked(); };
    bassButton.setColour(TextButton::buttonColourId, Colours::blue);
    bassButton.setEnabled(false);
    addAndMakeVisible(&bassButton);

    drumsButton.onClick = [this] {drumsButtonClicked(); };
    drumsButton.setColour(TextButton::buttonColourId, Colours::blue);
    drumsButton.setEnabled(false);
    addAndMakeVisible(&drumsButton);

    vocalsButton.onClick = [this] {vocalsButtonClicked(); };
    vocalsButton.setColour(TextButton::buttonColourId, Colours::blue);
    vocalsButton.setEnabled(false);
    addAndMakeVisible(&vocalsButton);

    otherButton.onClick = [this] {otherButtonClicked(); };
    otherButton.setColour(TextButton::buttonColourId, Colours::blue);
    otherButton.setEnabled(false);
    addAndMakeVisible(&otherButton);

    myPathToInstruments = "";


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

    transport.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();

    transport.getNextAudioBlock(bufferToFill);
}

void MainComponent::openButtonClicked()
{
    chooser = std::make_unique<juce::FileChooser>("Select a Wave or mp3 file to play...",
        juce::File{},
        "*.wav; *.mp3");                     // [7]
    auto chooserFlags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)     // [8]
    {
        auto file = fc.getResult();

        DBG(fc.getResult().getFullPathName());

        std::string myPath = (fc.getResult().getFullPathName().toStdString());
       
        std::string myFull = "demucs \"" + myPath + "\"";

        juce::File temp = temp.getCurrentWorkingDirectory();

        std::string myCurrentDir = temp.getFullPathName().toStdString();

        std::string myFileName = fc.getResult().getFileNameWithoutExtension().toStdString();

        myPathToInstruments = myCurrentDir + "\\separated\\htdemucs\\" + myFileName;

        juce::File myInstruments = File(myPathToInstruments);

        DBG(myPathToInstruments);

        // saved to directory of solution file
       // int result = system(myFull.c_str());
        int result = system("dir");
        if (result == 0) {
            std::cout << "Command executed successfully." << std::endl;
        }
        else {
            std::cerr << "Command execution failed." << std::endl;
        }

        if (myInstruments.isDirectory()) {
            bassButton.setEnabled(true);
            drumsButton.setEnabled(true);
            vocalsButton.setEnabled(true);
            otherButton.setEnabled(true);
        }
        else {
            bassButton.setEnabled(false);
            drumsButton.setEnabled(false);
            vocalsButton.setEnabled(false);
            otherButton.setEnabled(false);
        }

        if (file != juce::File{})                                                // [9]
        {
            auto* reader = formatManager.createReaderFor(file);                 // [10]

            if (reader != nullptr)
            {
                auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);   // [11]
                transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       // [12]
                playButton.setEnabled(true);     
                stopButton.setEnabled(false);
                playSource.reset(newSource.release());                                          // [14]
            }
        }
    });
}

void MainComponent::playButtonClicked()
{
    transportStateChanged(Starting);
}

void MainComponent::stopButtonClicked()
{
    transportStateChanged(Stopping);
}

void MainComponent::bassButtonClicked()
{
    std::string bassPath = myPathToInstruments + "\\bass.wav";
    juce::File bassFile = juce::File(bassPath);

    if (bassFile != juce::File{})                                               
    {
        auto* reader = formatManager.createReaderFor(bassFile);                 

        if (reader != nullptr)
        {
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);  
            transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);       
            playButton.setEnabled(true);
            stopButton.setEnabled(false);
            playSource.reset(newSource.release());                                          
        }
    }
}

void MainComponent::drumsButtonClicked()
{
    std::string drumsPath = myPathToInstruments + "\\drums.wav";
    juce::File drumsFile = juce::File(drumsPath);

    if (drumsFile != juce::File{})
    {
        auto* reader = formatManager.createReaderFor(drumsFile);

        if (reader != nullptr)
        {
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            stopButton.setEnabled(false);
            playSource.reset(newSource.release());
        }
    }
}

void MainComponent::vocalsButtonClicked()
{
    DBG("Vocals clicked");
    std::string vocalsPath = myPathToInstruments + "\\vocals.wav";
    juce::File vocalsFile = juce::File(vocalsPath);

    if (vocalsFile != juce::File{})
    {
        DBG("Past vocalsFile if");
        DBG(vocalsFile.getFullPathName().toStdString());
        auto* reader = formatManager.createReaderFor(vocalsFile);

        if (reader != nullptr)
        {
            DBG("Past reader nullptr");
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            stopButton.setEnabled(false);
            playSource.reset(newSource.release());
        }
    }
}

void MainComponent::otherButtonClicked()
{
    std::string otherPath = myPathToInstruments + "\\other.wav";
    juce::File otherFile = juce::File(otherPath);

    if (otherFile != juce::File{})
    {
        auto* reader = formatManager.createReaderFor(otherFile);

        if (reader != nullptr)
        {
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            stopButton.setEnabled(false);
            playSource.reset(newSource.release());
        }
    }
}

void MainComponent::transportStateChanged(TransportState newState) 
{
    if (newState != state)
    {
        state = newState;
    }

    switch (state) {
        case Stopped:
            playButton.setEnabled(true);
            transport.setPosition(0.0);
            break;
        case Starting:
            stopButton.setEnabled(true);
            playButton.setEnabled(false);
            transport.start();
            break;
        case Stopping:
            playButton.setEnabled(true);
            stopButton.setEnabled(false);
            transport.stop();
            break;
    }
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
    bassButton.setBounds(10, 130, getWidth() - 20, 30);
    drumsButton.setBounds(10, 170, getWidth() - 20, 30);
    vocalsButton.setBounds(10, 210, getWidth() - 20, 30);
    otherButton.setBounds(10, 250, getWidth() - 20, 30);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
