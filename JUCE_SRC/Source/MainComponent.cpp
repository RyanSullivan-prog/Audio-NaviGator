#include "MainComponent.h"

using namespace juce;

// file selecter and audio player created using https://docs.juce.com/master/tutorial_playing_sound_files.html, https://www.youtube.com/watch?v=eB6S8iWvx2k, and https://www.youtube.com/watch?v=vVnu-L712ho
// Audio thumbnail created using https://juce.com/tutorials/tutorial_audio_thumbnail/

//==============================================================================
MainComponent::MainComponent() : state(Stopped), openButton("Open"), playButton("Play"), stopButton("Stop"), bassButton("Bass"), drumsButton("Drums"), vocalsButton("Vocals"), otherButton("Other"), songButton("Full Song"), thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (600, 750);
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

    songButton.onClick = [this] {songButtonClicked(); };
    songButton.setColour(TextButton::buttonColourId, Colours::blue);
    songButton.setEnabled(false);
    addAndMakeVisible(&songButton);

    myPathToInstruments = "";
    originalFilePath = "";

    originalFile = File();


    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);

    startTimer(40);

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
    // UPDATE THIS
    chooser = std::make_unique<juce::FileChooser>("Select a Wave or mp3 file to play...",
        juce::File{},
        "*.wav; *.mp3");                     // [7]
    auto chooserFlags = juce::FileBrowserComponent::openMode
        | juce::FileBrowserComponent::canSelectFiles;

    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)     // [8]
    {
        auto file = fc.getResult();

        originalFile = file;

        std::string originalFilePath = (fc.getResult().getFullPathName().toStdString());

        juce::File temp = temp.getCurrentWorkingDirectory();

        std::string myCurrentDir = temp.getFullPathName().toStdString();

        std::string myFileName = fc.getResult().getFileNameWithoutExtension().toStdString();
       
        // UPDATE THIS
        std::string myFull = "demucs \"" + originalFilePath + "\"";

        // if using forward slash operating system use forward slashes, else backslash
        if (originalFilePath.find("/") != std::string::npos) {
            // UPDATE THIS
            myPathToInstruments = myCurrentDir + "/separated/htdemucs/" + myFileName;
        }
        else {
            // UPDATE THIS
            myPathToInstruments = myCurrentDir + "\\separated\\htdemucs\\" + myFileName;
        }

        juce::File myInstruments = File(myPathToInstruments);

        if (!myInstruments.exists()) {
            int result = system(myFull.c_str());
            if (result == 0) {
                std::cout << "Command executed successfully." << std::endl;
                myInstruments = File(myPathToInstruments);
            }
            else {
                std::cerr << "Command execution failed." << std::endl;
            }
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
                playButton.setButtonText("Play");
                stopButton.setEnabled(false);
                thumbnail.setSource(new juce::FileInputSource(file));
                playSource.reset(newSource.release());                                          // [14]
            }
        }
    });
}

void MainComponent::playButtonClicked()
{
    if (state == Starting) {
        transportStateChanged(Pausing);
    }
    else {
        transportStateChanged(Starting);
        playButton.setButtonText("Pause");
    }
}

void MainComponent::stopButtonClicked()
{
    transportStateChanged(Stopping);
}

void MainComponent::bassButtonClicked()
{
    // UPDATE THIS
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
            songButton.setEnabled(true);
            bassButton.setEnabled(false);
            drumsButton.setEnabled(true);
            vocalsButton.setEnabled(true);
            otherButton.setEnabled(true);
            transportStateChanged(Stopped);
            thumbnail.setSource(new juce::FileInputSource(bassFile));
            playSource.reset(newSource.release());                                          
        }
    }
}

void MainComponent::drumsButtonClicked()
{
    // UPDATE THIS
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
            songButton.setEnabled(true);
            bassButton.setEnabled(true);
            drumsButton.setEnabled(false);
            vocalsButton.setEnabled(true);
            otherButton.setEnabled(true);
            transportStateChanged(Stopped);
            thumbnail.setSource(new juce::FileInputSource(drumsFile));
            playSource.reset(newSource.release());
        }
    }
}

void MainComponent::vocalsButtonClicked()
{
    // UPDATE THIS
    std::string vocalsPath = myPathToInstruments + "\\vocals.wav";
    juce::File vocalsFile = juce::File(vocalsPath);


    if (vocalsFile != juce::File{})
    {
        auto* reader = formatManager.createReaderFor(vocalsFile);

        if (reader != nullptr)
        {
            DBG("Past reader nullptr");
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            stopButton.setEnabled(false);
            songButton.setEnabled(true);
            bassButton.setEnabled(true);
            drumsButton.setEnabled(true);
            vocalsButton.setEnabled(false);
            otherButton.setEnabled(true);
            transportStateChanged(Stopped);
            thumbnail.setSource(new juce::FileInputSource(vocalsFile));
            playSource.reset(newSource.release());
        }
    }
}

void MainComponent::otherButtonClicked()
{
    // UPDATE THIS
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
            songButton.setEnabled(true);
            bassButton.setEnabled(true);
            drumsButton.setEnabled(true);
            vocalsButton.setEnabled(true);
            otherButton.setEnabled(false);
            transportStateChanged(Stopped);
            thumbnail.setSource(new juce::FileInputSource(otherFile));
            playSource.reset(newSource.release());
        }
    }
}

void MainComponent::songButtonClicked()
{
    if (originalFile != juce::File{})
    {
        auto* reader = formatManager.createReaderFor(originalFile);

        if (reader != nullptr)
        {
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            stopButton.setEnabled(false);
            songButton.setEnabled(false);
            bassButton.setEnabled(true);
            drumsButton.setEnabled(true);
            vocalsButton.setEnabled(true);
            otherButton.setEnabled(true);
            transportStateChanged(Stopped);
            thumbnail.setSource(new juce::FileInputSource(originalFile));
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
            stopButton.setEnabled(false);
            playButton.setEnabled(true);
            playButton.setButtonText("Play");
            transport.setPosition(0.0);
            break;
        case Starting:
            stopButton.setEnabled(true);
            //playButton.setEnabled(false);
            playButton.setButtonText("Pause");
            transport.start();
            break;
        case Playing:
            stopButton.setEnabled(true);
            break;
        case Stopping:
            playButton.setEnabled(true);
            stopButton.setEnabled(false);
            transport.setPosition(0.0);
            playButton.setButtonText("Play");
            transport.stop();
            break;
        case Pausing:
            playButton.setButtonText("Play");
            transport.stop();
    }
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &transport)
        transportStateChanged(transport.isPlaying()? Playing : Stopped);
    if (source == &thumbnail)
        repaint();
}

void MainComponent::timerCallback() {
    repaint();
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
    juce::Rectangle<int> thumbnailBounds(10, 350, getWidth() - 20, getHeight() - 380);
    if (thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
        paintIfFileLoaded(g, thumbnailBounds);
    auto duration = (float)transport.getLengthInSeconds();

    if (duration > 0.0)
    {
        auto audioPosition = (float)transport.getCurrentPosition();
        auto drawPosition = (audioPosition / duration) * (float)getWidth() + 10;

        g.setColour(juce::Colours::green);
        g.drawLine(drawPosition, 350.0f, drawPosition, (float)getHeight()-30, 2.0f);
    }
}

void MainComponent::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void MainComponent::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds) {
    g.setColour(juce::Colours::white);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::red);
    thumbnail.drawChannels(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 1.0f);
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
    songButton.setBounds(10, 290, getWidth() - 20, 30);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
