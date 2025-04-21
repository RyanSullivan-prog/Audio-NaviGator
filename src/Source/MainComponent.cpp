#include "MainComponent.h"

using namespace juce;

// file selecter and audio player created using https://docs.juce.com/master/tutorial_playing_sound_files.html, https://www.youtube.com/watch?v=eB6S8iWvx2k, and https://www.youtube.com/watch?v=vVnu-L712ho
// Audio thumbnail created using https://juce.com/tutorials/tutorial_audio_thumbnail/
// dials created using https://www.youtube.com/watch?v=po46y8UKPOY
// combobox created using https://juce.com/tutorials/tutorial_combo_box/

//==============================================================================
MainComponent::MainComponent() : state(Stopped), openButton("Open"), playButton("Play"), stopButton("Stop"), sliderButton("Start effect"), parseButton("Parse Song"), saveButton("Apply Effects and Save"), thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize(600, 750);
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

    sliderButton.onClick = [this] {sliderButtonClicked(); };
    sliderButton.setColour(TextButton::buttonColourId, Colours::blue);
    sliderButton.setEnabled(false);
    addAndMakeVisible(&sliderButton);

    parseButton.onClick = [this] {parseButtonClicked(); };
    parseButton.setColour(TextButton::buttonColourId, Colours::blue);
    parseButton.setEnabled(false);
    addAndMakeVisible(&parseButton);

    saveButton.onClick = [this] {saveButtonClicked(); };
    saveButton.setColour(TextButton::buttonColourId, Colours::blue);
    saveButton.setEnabled(false);
    addAndMakeVisible(&saveButton);

    addAndMakeVisible(&reverbMenu);
    reverbMenu.addItem("Room Size", room_size);
    reverbMenu.addItem("Damping", damping);
    reverbMenu.addItem("Wet Level", wet_level);
    reverbMenu.addItem("Dry Level", dry_level);
    reverbMenu.addItem("Width", width);
    reverbMenu.addItem("Freeze Mode", freeze_mode);
    reverbMenu.onChange = [this] { reverbMenuChanged(); };
    reverbMenu.setSelectedId(room_size);

    addAndMakeVisible(&instrumentMenu);
    instrumentMenu.addItem("Full Song", fullSong);
    instrumentMenu.addItem("Bass", bass);
    instrumentMenu.addItem("Drums", drums);
    instrumentMenu.addItem("Vocals", vocals);
    instrumentMenu.addItem("Other", other);
    instrumentMenu.onChange = [this] { instrumentMenuChanged(); };
    instrumentMenu.setSelectedId(fullSong);
    instrumentMenu.setEnabled(false);

    addAndMakeVisible(&compressionMenu);
    compressionMenu.addItem("Threshold dB", thresholdDB);
    compressionMenu.addItem("Ratio", ratio);
    compressionMenu.addItem("Attack ms", attackMS);
    compressionMenu.addItem("Release ms", releaseMS);
    compressionMenu.onChange = [this] { compressionMenuChanged(); };
    compressionMenu.setSelectedId(thresholdDB);

    addAndMakeVisible(&phaserMenu);
    phaserMenu.addItem("Rate (hz)", rate);
    phaserMenu.addItem("Depth", depth);
    phaserMenu.addItem("Center Frequency (hz)", freq);
    phaserMenu.addItem("Feedback", feedback);
    phaserMenu.addItem("Mix", mix);
    phaserMenu.onChange = [this] { phaserMenuChanged(); };
    phaserMenu.setSelectedId(rate);

    PhaserDial.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    PhaserDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    PhaserDial.setRange(0.1, 4.0);
    PhaserDial.setValue(myRate);
    addAndMakeVisible(&PhaserDial);

    filterDial.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    filterDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    filterDial.setRange(10, 300.0);
    filterDial.setValue(50);
    addAndMakeVisible(&filterDial);

    CompressionDial.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    CompressionDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    CompressionDial.setRange(-60, 0.0);
    CompressionDial.setValue(myThresholdDB);
    addAndMakeVisible(&CompressionDial);

    myPathToInstruments = "";
    originalFilePath = "";

    originalFile = File();
    myBass = File();
    myVocals = File();
    myOther = File();
    myDrums = File();
    currentFile = File();

    //decibelSlider.setSliderStyle(Slider::SliderStyle::LinearBar);
    decibelDial.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    decibelDial.setRange(-80, 35);
    decibelDial.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    //decibelSlider.onValueChange = [this] { level = Decibels::decibelsToGain((float)decibelSlider.getValue()); };
    //decibelSlider.setValue(juce::Decibels::gainToDecibels(level));
    addAndMakeVisible(&decibelDial);

    distortionDial.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    distortionDial.setRange(-80, 35);
    distortionDial.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(&distortionDial);

    addAndMakeVisible(&scrubSlider);

    scrubSlider.setRange(0, 1);

    scrubSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, -1, -1);

    scrubSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);

    scrubSlider.addListener(this);

    scrubSlider.setAlpha(0);

    decibelLabel.setText("Gain Level (dB)", juce::dontSendNotification);
    addAndMakeVisible(&decibelLabel);

    distortionLabel.setText("Distortion Drive (dB)", juce::dontSendNotification);
    addAndMakeVisible(&distortionLabel);

    reverbLabel.setText("Reverb", juce::dontSendNotification);
    addAndMakeVisible(&reverbLabel);

    compressionLabel.setText("Compression", juce::dontSendNotification);
    addAndMakeVisible(&compressionLabel);

    phaserLabel.setText("Phaser", juce::dontSendNotification);
    addAndMakeVisible(&phaserLabel);

    filterLabel.setText("Lowpass Filter Cutoff Hz", juce::dontSendNotification);
    addAndMakeVisible(&filterLabel);


    ReverbDial.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    ReverbDial.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 100, 25);
    ReverbDial.setRange(0, 1.0);
    ReverbDial.setValue(0.5);
    addAndMakeVisible(&ReverbDial);

    formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);

    startTimer(40);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired(juce::RuntimePermissions::recordAudio)
        && !juce::RuntimePermissions::isGranted(juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request(juce::RuntimePermissions::recordAudio,
            [&](bool granted) { setAudioChannels(granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels(2, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()

    transport.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
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
    /*auto currentLevel = level;
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
    }*/
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

            currentFile = file;

            std::string originalFilePath = (fc.getResult().getFullPathName().toStdString());

            juce::File temp = temp.getCurrentWorkingDirectory();

            std::string myCurrentDir = temp.getFullPathName().toStdString();

            std::string myFileName = fc.getResult().getFileNameWithoutExtension().toStdString();

            myPathToInstruments = temp.getChildFile("separated").getChildFile("htdemucs").getChildFile(myFileName).getFullPathName().toStdString();

            myInstruments = temp.getChildFile("separated").getChildFile("htdemucs").getChildFile(myFileName);

            std::string myFull = "demucs \"" + originalFilePath + "\"";

            myPython = "python RunModel.py \"" + originalFilePath + "\"";

            myVocals = temp.getChildFile("vocals_" + fc.getResult().getFileNameWithoutExtension().toStdString() + ".wav");
            myBass = temp.getChildFile("bass_" + fc.getResult().getFileNameWithoutExtension().toStdString() + ".wav");
            myDrums = temp.getChildFile("drums_" + fc.getResult().getFileNameWithoutExtension().toStdString() + ".wav");
            myOther = temp.getChildFile("other_" + fc.getResult().getFileNameWithoutExtension().toStdString() + ".wav");

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
                    startEffect = 0.0f;
                    stopEffect = 0.0f;
                    sliderButton.setButtonText("Start effect");
                    sliderButton.setEnabled(true);
                    parseButton.setEnabled(true);
                    saveButton.setEnabled(true);
                    instrumentMenu.setEnabled(false);
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

void MainComponent::parseButtonClicked()
{
    transportStateChanged(Stopping);
    if (!(myVocals.exists() && myBass.exists() && myDrums.exists() && myOther.exists())) {
        system(myPython.c_str());
    }
    if (myVocals.exists() && myBass.exists() && myDrums.exists() && myOther.exists()) {
        instrumentMenu.setEnabled(true);
    }
    else {
        instrumentMenu.setEnabled(false);
    }
    parseButton.setEnabled(false);
    startEffect = 0.0f;
    stopEffect = 0.0f;
    sliderButton.setButtonText("Start effect");
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

void MainComponent::saveButtonClicked() {
    switch (prevReverb) {
        case room_size:
            myRoomSize = ReverbDial.getValue();
            break;
        case damping:
            myDamping = ReverbDial.getValue();
            break;
        case wet_level:
            myWetLevel = ReverbDial.getValue();
            break;
        case dry_level:
            myDryLevel = ReverbDial.getValue();
            break;
        case width:
            myWidth = ReverbDial.getValue();
            break;
        case freeze_mode:
            myFreezeMode = ReverbDial.getValue();
            break;
    }
    switch (prevCompression) {
        case thresholdDB:
            myThresholdDB = CompressionDial.getValue();
            break;
        case ratio:
            myRatio = CompressionDial.getValue();
            break;
        case attackMS:
            myAttackMS = CompressionDial.getValue();
            break;
        case releaseMS:
            myReleaseMS = CompressionDial.getValue();
            break;
    } switch (prevPhaser) {
        case rate:
            myRate = PhaserDial.getValue();
            break;
        case depth:
            myDepth = PhaserDial.getValue();
            break;
        case freq:
            myFreq = PhaserDial.getValue();;
            break;
        case feedback:
            myFeedback = PhaserDial.getValue();
            break;
        case mix:
            myMix = PhaserDial.getValue();
            break;
    }
    std::string myEffects = "python main.py \"" + currentFile.getFullPathName().toStdString() + "\" " + std::to_string(myRoomSize) + " " + std::to_string(myDamping) + " " + std::to_string(myWetLevel) + " " + std::to_string(myDryLevel) + " " + std::to_string(myWidth) + " " + std::to_string(myFreezeMode) + " " + std::to_string(decibelDial.getValue()) + " " + std::to_string(startEffect) + " " + std::to_string(stopEffect) + " " + std::to_string(distortionDial.getValue()) + " " + std::to_string(myThresholdDB) + " " + std::to_string(myRatio) + " " + std::to_string(myAttackMS) + " " + std::to_string(myReleaseMS) + " " + std::to_string(myRate) + " " + std::to_string(myDepth) + " "+ std::to_string(myFreq) + " " + std::to_string(myFeedback) + " " + std::to_string(myMix) + " " + std::to_string(filterDial.getValue());
    system(myEffects.c_str());
    thumbnailCache.clear();
}

void MainComponent::reverbMenuChanged() {
    switch (prevReverb) {
        case room_size:
            myRoomSize = ReverbDial.getValue();
            break;
        case damping:
            myDamping = ReverbDial.getValue();
            break;
        case wet_level:
            myWetLevel = ReverbDial.getValue();
            break;
        case dry_level:
            myDryLevel = ReverbDial.getValue();
            break;
        case width:
            myWidth = ReverbDial.getValue();
            break;
        case freeze_mode:
            myFreezeMode = ReverbDial.getValue();
            break;
    }
    switch (reverbMenu.getSelectedId())
    {
        case room_size:
            ReverbDial.setValue(myRoomSize);
            prevReverb = room_size;
            break;
        case damping:
            ReverbDial.setValue(myDamping);
            prevReverb = damping;
            break;
        case wet_level:
            ReverbDial.setValue(myWetLevel);
            prevReverb = wet_level;
            break;
        case dry_level:
            ReverbDial.setValue(myDryLevel);
            prevReverb = dry_level;
            break;
        case width:
            ReverbDial.setValue(myWidth);
            prevReverb = width;
            break;
        case freeze_mode:
            ReverbDial.setValue(myFreezeMode);
            prevReverb = freeze_mode;
            break;
    }
}

void MainComponent::compressionMenuChanged() {
        switch (prevCompression) {
        case thresholdDB:
            myThresholdDB = CompressionDial.getValue();
            DBG(myThresholdDB);
            break;
        case ratio:
            myRatio = CompressionDial.getValue();
            DBG(myRatio);
            break;
        case attackMS:
            myAttackMS = CompressionDial.getValue();
            DBG(myAttackMS);
            break;
        case releaseMS:
            myReleaseMS = CompressionDial.getValue();
            DBG(myReleaseMS);
            break;
        }
    switch (compressionMenu.getSelectedId())
    {
        case thresholdDB:
            CompressionDial.setRange(-60, 0.0);
            CompressionDial.setValue(myThresholdDB);
            prevCompression = thresholdDB;
            break;
        case ratio:
            CompressionDial.setRange(1, 15.0);
            CompressionDial.setValue(myRatio);
            prevCompression = ratio;
            break;
        case attackMS:
            CompressionDial.setRange(0.01, 100);
            CompressionDial.setValue(myAttackMS);
            prevCompression = attackMS;
            break;
        case releaseMS:
            CompressionDial.setRange(10, 4000);
            CompressionDial.setValue(myReleaseMS);
            prevCompression = releaseMS;
            break;
    }
}

void MainComponent::phaserMenuChanged() {
    switch (prevPhaser) {
        case rate:
            myRate = PhaserDial.getValue();
            DBG(myRate);
            break;
        case depth:
            myDepth = PhaserDial.getValue();
            DBG(myDepth);
            break;
        case freq:
            myFreq = PhaserDial.getValue();
            DBG(myFreq);
            break;
        case feedback:
            myFeedback = PhaserDial.getValue();
            DBG(myFeedback);
            break;
        case mix:
            myMix = PhaserDial.getValue();
            DBG(myMix);
            break;
    }

    switch (phaserMenu.getSelectedId())
    {
        case rate:
            PhaserDial.setRange(0.1, 4.0);
            PhaserDial.setValue(myRate);
            prevPhaser = rate;
            break;
        case depth:
            PhaserDial.setRange(0, 1.0);
            PhaserDial.setValue(myDepth);
            prevPhaser = depth;
            break;
        case freq:
            PhaserDial.setRange(300, 3000.0);
            PhaserDial.setValue(myFreq);
            prevPhaser = freq;
            break;
        case feedback:
            PhaserDial.setRange(0, 1.0);
            PhaserDial.setValue(myFeedback);
            prevPhaser = feedback;
            break;
        case mix:
            PhaserDial.setRange(0, 1.0);
            PhaserDial.setValue(myMix);
            prevPhaser = mix;
            break;
    }
}

void MainComponent::instrumentMenuChanged() {
    juce::File myFile = juce::File();
    myRoomSize = 0.5;
    myDamping = 0.5;
    myWetLevel = 0.33;
    myDryLevel = 0.4;
    myWidth = 1.0;
    myFreezeMode = 0.0;
    myThresholdDB = 0.0;
    myRatio = 1;
    myAttackMS = 1;
    myReleaseMS = 100;
    prevReverb = 1;
    prevCompression = thresholdDB;
    reverbMenu.setSelectedId(prevReverb);
    compressionMenu.setSelectedId(prevCompression);
    ReverbDial.setValue(myRoomSize);
    CompressionDial.setValue(myThresholdDB);
    CompressionDial.setRange(-60, 0.0);
    decibelDial.setValue(0);
    distortionDial.setValue(0);
    myRate = 1.0;
    myDepth = 0.5;
    myFreq = 1300.0;
    myFeedback = 0.0;
    myMix = 0.5;
    prevPhaser = rate;
    phaserMenu.setSelectedId(rate);
    PhaserDial.setValue(myRate);
    PhaserDial.setRange(0.1, 4.0);
    filterDial.setValue(50.0);
    switch (instrumentMenu.getSelectedId())
    {
        case fullSong:
            myFile = originalFile;
            break;
        case bass:
            myFile = myBass;
            break;
        case drums:
            myFile = myDrums;
            break;
        case vocals:
            myFile = myVocals;
            break;
        case other:
            myFile = myOther;
            break;
    }
    if (myFile != juce::File{})
    {
        auto* reader = formatManager.createReaderFor(myFile);

        if (reader != nullptr)
        {
            auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
            transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
            playButton.setEnabled(true);
            stopButton.setEnabled(false);
            transportStateChanged(Stopped);
            thumbnail.setSource(new juce::FileInputSource(myFile));
            playSource.reset(newSource.release());
            startEffect = 0.0f;
            stopEffect = 0.0f;
            sliderButton.setButtonText("Start effect");
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
        transportStateChanged(transport.isPlaying() ? Playing : Stopped);
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
void MainComponent::paint(juce::Graphics& g)
{
    juce::Rectangle<int> thumbnailBounds(10, 210, (getWidth() - 20) / 2, getHeight() - 300);
    if (thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
        paintIfFileLoaded(g, thumbnailBounds);
    auto duration = (float)transport.getLengthInSeconds();

    if (duration > 0.0)
    {
        auto audioPosition = (float)transport.getCurrentPosition();
        auto drawPosition = (audioPosition / duration) * (float)thumbnailBounds.getWidth() + 10;

        g.setColour(juce::Colours::blue);
        g.drawLine(drawPosition, 210, drawPosition, (float)getHeight() - 90, 2.0f);
        if (startEffect != 0.0f) {
            g.setColour(juce::Colours::green);
            drawPosition = (startEffect / duration) * (float)thumbnailBounds.getWidth() + 10;
            g.drawLine(drawPosition, 210, drawPosition, (float)getHeight() - 90, 2.0f);
        }
        if (stopEffect != 0.0f) {
            g.setColour(juce::Colours::red);
            drawPosition = (stopEffect / duration) * (float)thumbnailBounds.getWidth() + 10;
            g.drawLine(drawPosition, 210, drawPosition, (float)getHeight() - 90, 2.0f);
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
    parseButton.setBounds(10, 130, getWidth() - 20, 30);
    instrumentMenu.setBounds(10, 170, getWidth() - 20, 30);
    sliderButton.setBounds(10, getHeight() - 85, (getWidth() - 20) / 4-5, 30);
    saveButton.setBounds(10+ (getWidth() - 20) / 4, getHeight() - 85, (getWidth() - 20) / 4+2, 30);
   // sliderButton.setBounds(10, 210, getWidth() - 20, 30);
    //saveButton.setBounds(10, 250, getWidth() - 20, 30);
    //bassButton.setBounds(10, 130, getWidth() - 20, 30);
    //drumsButton.setBounds(10, 170, getWidth() - 20, 30);
    //vocalsButton.setBounds(10, 210, getWidth() - 20, 30);
    //otherButton.setBounds(10, 250, getWidth() - 20, 30);
    //songButton.setBounds(10, 290, getWidth() - 20, 30);
    decibelDial.setBounds((getWidth() - 20) / 2 + 20, 240, 75, 110);
    distortionDial.setBounds((getWidth() - 20) / 2 + 170, 240, 75, 110);
    scrubSlider.setBounds(0, 210, (getWidth()+20)/2, getHeight() - 300);
    //sliderButton.setBounds((getWidth() - 20) / 2 + 20, 430, (getWidth() - 30) / 2, 30);
    //parseButton.setBounds((getWidth() - 20) / 2 + 20, 470, (getWidth() - 30) / 2, 30);
    decibelLabel.setBounds((getWidth() - 20) / 2 + 10, 220, (getWidth() - 30) / 2, 30);
    distortionLabel.setBounds((getWidth() - 20) / 2 + 160, 220, (getWidth() - 30) / 2, 30);
    ReverbDial.setBounds((getWidth() - 20) / 2 + 20, 380, 75, 110);
    reverbLabel.setBounds((getWidth() - 20) / 2 + 20, 360, (getWidth() - 30) / 2, 30);
    reverbMenu.setBounds((getWidth() - 20) / 2 + 20, 495, 125, 30);
    CompressionDial.setBounds((getWidth() - 20) / 2 + 170, 380, 75, 110);
    compressionMenu.setBounds((getWidth() - 20) / 2 + 170, 495, 125, 30);
    compressionLabel.setBounds((getWidth() - 20) / 2 + 160, 360, (getWidth() - 30) / 2, 30);
    PhaserDial.setBounds((getWidth() - 20) / 2 + 20, 550, 75, 110);
    phaserLabel.setBounds((getWidth() - 20) / 2 + 20, 530, (getWidth() - 30) / 2, 30);
    phaserMenu.setBounds((getWidth() - 20) / 2 + 20, 665, 125, 30);
    filterDial.setBounds((getWidth() - 20) / 2 + 170, 550, 75, 110);
    filterLabel.setBounds((getWidth() - 20) / 2 + 140, 530, (getWidth() - 30) / 2, 30);
    //saveButton.setBounds((getWidth() - 20) / 2 + 20, 650, (getWidth() - 30) / 2, 30);
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}
