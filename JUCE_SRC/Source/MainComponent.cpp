#include "MainComponent.h"

using namespace juce;

// file selecter and audio player created using https://docs.juce.com/master/tutorial_playing_sound_files.html, https://www.youtube.com/watch?v=eB6S8iWvx2k, and https://www.youtube.com/watch?v=vVnu-L712ho
// Audio thumbnail created using https://juce.com/tutorials/tutorial_audio_thumbnail/
// dials created using https://www.youtube.com/watch?v=po46y8UKPOY

//==============================================================================
MainComponent::MainComponent() : state(Stopped), openButton("Open"), playButton("Play"), stopButton("Stop"), bassButton("Bass"), drumsButton("Drums"), vocalsButton("Vocals"), otherButton("Other"), songButton("Full Song"), sliderButton("Start effect"), thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
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

    sliderButton.onClick = [this] {sliderButtonClicked(); };
    sliderButton.setColour(TextButton::buttonColourId, Colours::blue);
    sliderButton.setEnabled(false);
    addAndMakeVisible(&sliderButton);

    myPathToInstruments = "";
    originalFilePath = "";

    originalFile = File();
    myBass = File();
    myVocals = File();
    myOther = File();
    myDrums = File();

    //decibelSlider.setSliderStyle(Slider::SliderStyle::LinearBar);
    decibelSlider.setRange(-80, 35);
    decibelSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    decibelSlider.onValueChange = [this] { level = Decibels::decibelsToGain((float)decibelSlider.getValue()); };
    decibelSlider.setValue(juce::Decibels::gainToDecibels(level));
    addAndMakeVisible(&decibelSlider);

    startTimeSlider.setRange(0.0, 1.0);
    startTimeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    addAndMakeVisible(&startTimeSlider);

    stopTimeSlider.setRange(0.0, 1.0);
    stopTimeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    addAndMakeVisible(&stopTimeSlider);

    addAndMakeVisible(&scrubSlider);

    scrubSlider.setRange(0, 1);

    scrubSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, -1, -1);

    scrubSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);

    scrubSlider.addListener(this);



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
    // guide on changing volume from https://forum.juce.com/t/volume-control-in-audio-player/44551/7
    // decibel volume control guide from https://juce.com/tutorials/tutorial_synth_db_level_control/
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();

    transport.getNextAudioBlock(bufferToFill);

    /*WavAudioFormat format;
    std::unique_ptr<AudioFormatWriter> writer;
    juce::File file = juce::File(file.getCurrentWorkingDirectory());
    file = file.getChild
    writer.reset(format.createWriterFor(new FileOutputStream(file),
        48000.0,
        bufferToFill.buffer->getNumChannels(),
        24,
        {},
        0));
    if (writer != nullptr)
        writer->writeFromAudioSampleBuffer(*bufferToFill.buffer, 0, bufferToFill.buffer->getNumSamples());
        */
    auto currentLevel = level;
    auto levelScale = currentLevel * 2.0f;
    auto audioPosition = 0.0;
    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel) {
        auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample) {
            audioPosition = (float)transport.getCurrentPosition();
            if (audioPosition >= startEffect && audioPosition <= stopEffect) {
                buffer[sample] = buffer[sample] * levelScale - currentLevel;
            }
        }
    }
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

        originalFile = file;

        std::string originalFilePath = (fc.getResult().getFullPathName().toStdString());

        juce::File temp = temp.getCurrentWorkingDirectory();

        std::string myCurrentDir = temp.getFullPathName().toStdString();

        std::string myFileName = fc.getResult().getFileNameWithoutExtension().toStdString();

        myPathToInstruments = temp.getChildFile("separated").getChildFile("htdemucs").getChildFile(myFileName).getFullPathName().toStdString();

        myInstruments = temp.getChildFile("separated").getChildFile("htdemucs").getChildFile(myFileName);
       
        std::string myFull = "demucs \"" + originalFilePath + "\"";

        std::string myPython = "python RunModel.py \"" + originalFilePath + "\"";

        myVocals = temp.getChildFile("vocals_" + fc.getResult().getFileNameWithoutExtension().toStdString() + ".wav");
        DBG(myVocals.getFileName());
        myBass = temp.getChildFile("bass_" + fc.getResult().getFileNameWithoutExtension().toStdString() + ".wav");
        myDrums = temp.getChildFile("drums_" + fc.getResult().getFileNameWithoutExtension().toStdString() + ".wav");
        myOther = temp.getChildFile("other_" + fc.getResult().getFileNameWithoutExtension().toStdString() + ".wav");

        sliderButton.setEnabled(true);

        if (!(myVocals.exists() && myBass.exists() && myDrums.exists() && myOther.exists())) {
            system(myPython.c_str());
        }
        if (myVocals.exists() && myBass.exists() && myDrums.exists() && myOther.exists()) {
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

        // if using forward slash operating system use forward slashes, else backslash
       /* if (originalFilePath.find("/") != std::string::npos) {
            myPathToInstruments = myCurrentDir + "/separated/htdemucs/" + myFileName;
        }
        else {
            myPathToInstruments = myCurrentDir + "\\separated\\htdemucs\\" + myFileName;
        }*/



        /*if (!myInstruments.exists()) {
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
        }*/

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
                scrubSlider.setRange(0, transport.getLengthInSeconds());
                startTimeSlider.setRange(0.0, transport.getLengthInSeconds());
                stopTimeSlider.setRange(0.0, transport.getLengthInSeconds());
                startEffect = 0.0f;
                stopEffect = 0.0f;
                sliderButton.setButtonText("Start effect");
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
    juce::File temp = juce::File(myBass.getFullPathName().toStdString());
    DBG(temp.getFullPathName().toStdString());
    if (temp != juce::File{})                                               
    {
        auto* reader = formatManager.createReaderFor(temp);    

        if (reader != nullptr)
        {
            DBG("Past reader nullptr");
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
            thumbnail.setSource(new juce::FileInputSource(temp));
            playSource.reset(newSource.release());          
            startTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            stopTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            startEffect = 0.0f;
            stopEffect = 0.0f;
            sliderButton.setButtonText("Start effect");
        }
    }
}

void MainComponent::drumsButtonClicked()
{
    if (myDrums != juce::File{})
    {
        auto* reader = formatManager.createReaderFor(myDrums);

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
            thumbnail.setSource(new juce::FileInputSource(myDrums));
            playSource.reset(newSource.release());
            startTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            stopTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            startEffect = 0.0f;
            stopEffect = 0.0f;
            sliderButton.setButtonText("Start effect");
        }
    }
}

void MainComponent::vocalsButtonClicked()
{

    if (myVocals != juce::File{})
    {
        auto* reader = formatManager.createReaderFor(myVocals);

        if (reader != nullptr)
        {
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
            thumbnail.setSource(new juce::FileInputSource(myVocals));
            playSource.reset(newSource.release());
            startTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            stopTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            startEffect = 0.0f;
            stopEffect = 0.0f;
            sliderButton.setButtonText("Start effect");
        }
    }
}

void MainComponent::otherButtonClicked()
{
    if (myOther != juce::File{})
    {
        auto* reader = formatManager.createReaderFor(myOther);

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
            thumbnail.setSource(new juce::FileInputSource(myOther));
            playSource.reset(newSource.release());
            startTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            stopTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            startEffect = 0.0f;
            stopEffect = 0.0f;
            sliderButton.setButtonText("Start effect");
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
            DBG("Past reader check");
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
            startTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            stopTimeSlider.setRange(0.0, transport.getLengthInSeconds());
            startEffect = 0.0f;
            stopEffect = 0.0f;
            sliderButton.setButtonText("Start effect");
        }
    }
}

void MainComponent::sliderButtonClicked() {
    if (sliderButton.getButtonText().equalsIgnoreCase("Start effect")) {
        sliderButton.setButtonText("Stop effect");
        startEffect = scrubSlider.getValue();
    }
    else {
        sliderButton.setButtonText("Start effect");
        stopEffect = scrubSlider.getValue();
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

void MainComponent::sliderValueChanged(juce::Slider* slider) {
    if (slider->isMouseButtonDown()) { transport.setPosition(scrubSlider.getValue()); }
}

void MainComponent::timerCallback() {
    repaint();
    scrubSlider.setValue(transport.getCurrentPosition());
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
    juce::Rectangle<int> thumbnailBounds(10, 350, (getWidth() - 20)/2, getHeight() - 380);
    if (thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
        paintIfFileLoaded(g, thumbnailBounds);
    auto duration = (float)transport.getLengthInSeconds();

    if (duration > 0.0)
    {
        auto audioPosition = (float)transport.getCurrentPosition();
        auto drawPosition = (audioPosition / duration) * (float)thumbnailBounds.getWidth() + 10;

        g.setColour(juce::Colours::green);
        g.drawLine(drawPosition, 350.0f, drawPosition, (float)getHeight()-30, 2.0f);
        if (startEffect != 0.0f) {
            g.setColour(juce::Colours::blue);
            drawPosition = (startEffect / duration) * (float)thumbnailBounds.getWidth() + 10;
            g.drawLine(drawPosition, 350.0f, drawPosition, (float)getHeight() - 30, 2.0f);
        }
        if (stopEffect != 0.0f) {
            g.setColour(juce::Colours::red);
            drawPosition = (stopEffect / duration) * (float)thumbnailBounds.getWidth() + 10;
            g.drawLine(drawPosition, 350.0f, drawPosition, (float)getHeight() - 30, 2.0f);
        }
        if ((float)transport.getCurrentPosition() >= duration) {
            transportStateChanged(Stopped);
        }
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
    decibelSlider.setBounds((getWidth() - 20) / 2 + 10, 320, (getWidth() - 20) / 2, 20);
    //startTimeSlider.setBounds((getWidth() - 20) / 2 + 10, 350, (getWidth() - 20) / 2, getHeight() - 380);
    auto sliderLeft = 0;
    scrubSlider.setBounds(sliderLeft, 330, getWidth(), 20);
    sliderButton.setBounds((getWidth() - 20) / 2 + 10, 350, (getWidth() - 20) / 2, 30);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
