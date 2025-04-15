#include "MainComponent.h"

using namespace juce;

// file selecter and audio player created using https://docs.juce.com/master/tutorial_playing_sound_files.html, https://www.youtube.com/watch?v=eB6S8iWvx2k, and https://www.youtube.com/watch?v=vVnu-L712ho
// Audio thumbnail created using https://juce.com/tutorials/tutorial_audio_thumbnail/

//==============================================================================
MainComponent::MainComponent() : state(Stopped), openButton("Open"), playButton("Play"),
stopButton("Stop"), bassButton("Bass"), drumsButton("Drums"), vocalsButton("Vocals"), otherButton("Other"),
songButton("Full Song"), newSongButton("Edited Song"), thumbnailCache(5), thumbnail(512, formatManager, thumbnailCache)
{
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

    newSongButton.onClick = [this] {newSongButtonClicked(); };
    newSongButton.setColour(TextButton::buttonColourId, Colours::blue);
    newSongButton.setEnabled(false);
    addAndMakeVisible(&newSongButton);

    myPathToInstruments = "";
    originalFilePath = "";

    originalFile = File();

    addAndMakeVisible(scrubSlider);
    scrubSlider.setRange(0, 1);
    scrubSlider.setTextBoxStyle(Slider::TextEntryBoxPosition::NoTextBox, true, -1, -1);
    scrubSlider.setSliderStyle(Slider::SliderStyle::LinearHorizontal);
    scrubSlider.addListener(this);

    //formatManager.registerBasicFormats();
    thumbnail.addChangeListener(this);

    mixer = std::make_unique<juce::MixerAudioSource>();
    ASP.setSource(mixer.get());
    deviceManager.addAudioCallback(&ASP);

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
    formatManager.registerBasicFormats();
    mixer = std::make_unique<juce::MixerAudioSource>();
    //if (mixer.get() != nullptr) {
    //    ASP.setSource(mixer.get());
    //    deviceManager.addAudioCallback(&ASP);
    //    ASP.prepareToPlay(samplesPerBlockExpected, sampleRate);
    //}

    for (const auto& file : files)
    {
        if (file.existsAsFile())
        {
            auto* reader = formatManager.createReaderFor(file);

            if (reader != nullptr)
            {
                auto readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                auto transport = std::make_unique<juce::AudioTransportSource>();
                transport->setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

                mixer->addInputSource(transport.get(), false);

                readers.push_back(std::move(readerSource));
                transports.push_back(std::move(transport));
            }
        }
    }

}


void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    mixer->getNextAudioBlock(bufferToFill);
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

            std::string myFull = "demucs \"" + originalFilePath + "\"";

            // if using forward slash operating system use forward slashes, else backslash
            if (originalFilePath.find("/") != std::string::npos) {
                myPathToInstruments = myCurrentDir + "/separated/htdemucs/" + myFileName;
            }
            else {
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

                    scrubSlider.setRange(0, transport.getLengthInSeconds());
                    //scrubSlider.setTextValueSuffix(" ms"); // necessary?
                    //scrubLabel.setText("Current time", juce::dontSendNotification);
                    //scrubLabel.attachToComponent(&scrubSlider, true);
                }
            }
        });
}
//void MainComponent::openButtonClicked()
//{
//    chooser = std::make_unique<juce::FileChooser>(
//        "Select a Wave or MP3 file to play...",
//        juce::File{},
//        "*.wav;*.mp3"
//    );
//
//    auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;
//
//    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
//        {
//            auto file = fc.getResult();
//            if (!file.existsAsFile()) {
//                return;
//            }
//
//            originalFile = file;
//            std::string originalFilePath = file.getFullPathName().toStdString();
//            std::string myCurrentDir = juce::File::getCurrentWorkingDirectory().getFullPathName().toStdString();
//            std::string myFileName = file.getFileNameWithoutExtension().toStdString();
//            std::string myFull = "python3 -m demucs \"" + originalFilePath + "\"";
//
//            juce::File instrumentsFolder(myPathToInstruments);
//
//            // Run Demucs only if stems don't exist yet
//            if (!instrumentsFolder.exists())
//            {
//                if (system(myFull.c_str()) == 0)
//                    std::cout << "Demucs executed successfully.\n";
//                else
//                    std::cerr << "Demucs command failed.\n";
//            }
//
//            // Try loading the stems
//            files.clear();
//            std::vector<std::string> stemNames = { "bass", "drums", "vocals", "other" };
//
//            if (instrumentsFolder.isDirectory())
//            {
//                for (const auto& stem : stemNames)
//                {
//                    juce::File stemFile = instrumentsFolder.getChildFile(stem + ".wav");
//                    if (stemFile.existsAsFile())
//                        files.emplace_back(stem);
//                    else
//                        std::cerr << "Missing stem: " << stem << "\n";
//                }
//
//                bassButton.setEnabled(true);
//                drumsButton.setEnabled(true);
//                vocalsButton.setEnabled(true);
//                otherButton.setEnabled(true);
//
//
//                for (const auto& file : files)
//                {
//                    if (file.existsAsFile())
//                    {
//                        auto* reader = formatManager.createReaderFor(file);
//
//                        if (reader != nullptr)
//                        {
//                            auto readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
//                            auto transport = std::make_unique<juce::AudioTransportSource>();
//                            transport->setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
//                            mixer->addInputSource(transport.get(), false);
//                            readers.push_back(std::move(readerSource));
//                            transports.push_back(std::move(transport));
//                        }
//                    }
//                }
//            }
//            else
//            {
//                bassButton.setEnabled(false);
//                drumsButton.setEnabled(false);
//                vocalsButton.setEnabled(false);
//                otherButton.setEnabled(false);
//            }
//        });
//
//}


void MainComponent::loadStems()
{
    transports.clear();
    readers.clear();
    mixer->removeAllInputs();

    std::vector<std::string> stemNames = { "bass", "drums", "vocals", "other" };

    for (const auto& name : stemNames)
    {
        juce::File file(myPathToInstruments + "\\" + name + ".wav");

        if (file.existsAsFile())
        {
            if (auto* reader = formatManager.createReaderFor(file))
            {
                auto readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                auto transport = std::make_unique<juce::AudioTransportSource>();

                transport->setSource(readerSource.get(), 0, nullptr, reader->sampleRate);
                mixer->addInputSource(transport.get(), false);

                readers.push_back(std::move(readerSource));
                transports.push_back(std::move(transport));
            }
        }
    }

    transportStateChanged(Stopped);
    playButton.setEnabled(true);
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
    if (transports.size() > 0)
    {
        for (auto& transport : transports)
        {
            if (transport != nullptr && transport->isPlaying())
            {
                mixer->removeInputSource(transports[0].get());
            }
            else {
                mixer->addInputSource(transports[0].get(), false);
            }
        }
    }
}

void MainComponent::drumsButtonClicked()
{
    if (transports.size() > 0)
    {
        for (auto& transport : transports)
        {
            if (transport != nullptr && transport->isPlaying())
            {
                mixer->removeInputSource(transports[1].get());
            }
            else {
                mixer->addInputSource(transports[1].get(), false);
            }
        }
    }
}

void MainComponent::vocalsButtonClicked()
{
    if (transports.size() > 0)
    {
        for (auto& transport : transports)
        {
            if (transport != nullptr && transport->isPlaying())
            {
                mixer->removeInputSource(transports[2].get());
            }
            else {
                mixer->addInputSource(transports[2].get(), false);
            }
        }
    }
}

void MainComponent::otherButtonClicked()
{
    if (transports.size() > 0)
    {
        for (auto& transport : transports)
        {
            if (transport != nullptr && transport->isPlaying())
            {
                mixer->removeInputSource(transports[3].get());
            }
            else {
                mixer->addInputSource(transports[3].get(), false);
            }
        }
    }
}

void MainComponent::songButtonClicked()
{
    ////juce::File songFile = juce::File(originalFilePath);
    //DBG("song clicked");
    ////DBG(originalFilePath);
    //if (originalFile != juce::File{})
    //{
    //    DBG("Past songfile check");
    //    auto* reader = formatManager.createReaderFor(originalFile);

    //    if (reader != nullptr)
    //    {
    //        DBG("Past reader check");
    //        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    //        transport.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
    //        playButton.setEnabled(true);
    //        stopButton.setEnabled(false);
    //        songButton.setEnabled(false);
    //        bassButton.setEnabled(true);
    //        drumsButton.setEnabled(true);
    //        vocalsButton.setEnabled(true);
    //        otherButton.setEnabled(true);
    //        newSongButton.setEnabled(true);
    //        transportStateChanged(Stopped);
    //        thumbnail.setSource(new juce::FileInputSource(originalFile));
    //        //playSource.reset(newSource.release());
    //    }
    //}
}

void MainComponent::newSongButtonClicked()
{
    DBG("new song clicked");
    loadStems();

    //std::string bassPath = myPathToInstruments + "\\bass.wav";
    //juce::File bassFile = juce::File(bassPath);

    //std::string drumsPath = myPathToInstruments + "\\drums.wav";
    //juce::File drumsFile = juce::File(drumsPath);

    //std::string vocalsPath = myPathToInstruments + "\\vocals.wav";
    //juce::File vocalsFile = juce::File(vocalsPath);

    //std::string otherPath = myPathToInstruments + "\\other.wav";
    //juce::File otherFile = juce::File(otherPath);

    //if (otherFile.existsAsFile() && vocalsFile.existsAsFile() && drumsFile.existsAsFile() && bassFile.existsAsFile())
    //{
    //    auto* bassReader = formatManager.createReaderFor(bassFile);
    //    auto* drumsReader = formatManager.createReaderFor(drumsFile);
    //    auto* vocalsReader = formatManager.createReaderFor(vocalsFile);
    //    auto* otherReader = formatManager.createReaderFor(otherFile);

    //    if (otherReader != nullptr)
    //    {
    //        auto bassSource = std::make_unique<juce::AudioFormatReaderSource>(bassReader, true);
    //        auto bassTransport = std::make_unique <juce::AudioTransportSource>();
    //        bassTransport->setSource(bassSource.get(), 0, nullptr, bassReader->sampleRate);

    //        auto drumsSource = std::make_unique<juce::AudioFormatReaderSource>(drumsReader, true);
    //        auto* drumsTransportCheckThisLater = new AudioTransportSource();
    //        drumsTransportCheckThisLater->setSource(drumsSource.get(), 0, nullptr, drumsReader->sampleRate);

    //        auto vocalsSource = std::make_unique<juce::AudioFormatReaderSource>(vocalsReader, true);
    //        auto* vocalsTransportCheckThisLater = new AudioTransportSource();
    //        vocalsTransportCheckThisLater->setSource(vocalsSource.get(), 0, nullptr, vocalsReader->sampleRate);

    //        auto otherSource = std::make_unique<juce::AudioFormatReaderSource>(otherReader, true);
    //        auto* otherTransportCheckThisLater = new AudioTransportSource();
    //        otherTransportCheckThisLater->setSource(otherSource.get(), 0, nullptr, otherReader->sampleRate);


    //        playButton.setEnabled(true);
    //        stopButton.setEnabled(false);
    //        songButton.setEnabled(true);
    //        bassButton.setEnabled(true);
    //        drumsButton.setEnabled(true);
    //        vocalsButton.setEnabled(true);
    //        otherButton.setEnabled(true);
    //        newSongButton.setEnabled(false);
    //        transportStateChanged(Stopped);
    //        thumbnail.setSource(new juce::FileInputSource(otherFile));
    //    }
    //}
}

void MainComponent::transportStateChanged(TransportState newState) 
{
    if (newState != state)
    {
        state = newState;
    }

    if (state == newState)
        return;

    state = newState;

    switch (state)
    {
    case Starting:
    case Playing:
        for (auto& transport : transports)
            if (transport != nullptr)
                transport->start();
        playButton.setButtonText("Pause");
        break;

    case Pausing:
        for (auto& transport : transports)
            if (transport != nullptr)
                transport->stop();
        playButton.setButtonText("Play");
        break;

    case Stopped:
    case Stopping:
        for (auto& transport : transports)
            if (transport != nullptr)
            {
                transport->stop();
                transport->setPosition(0.0);
            }
        playButton.setButtonText("Play");
        break;
    }

}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    for (auto& transport : transports) {
        if (source == transport.get()) {
            transportStateChanged(transport->isPlaying() ? Playing : Stopped);
            break;
        }
    }

    if (source == &thumbnail)
        repaint();
}


void MainComponent::sliderValueChanged(juce::Slider* slider) {
    //if (slider->isMouseButtonDown()) { transport.setPosition(scrubSlider.getValue()); }
    if (slider->isMouseButtonDown()) {
        for (auto& transport : transports) {
            if (transport != nullptr)
                transport->setPosition(scrubSlider.getValue());
        }
    }
}

void MainComponent::timerCallback() {
    repaint();
    //scrubSlider.setValue(mixer.getCurrentPosition());

    if (!transports.empty() && transports[0] != nullptr)
    {
        double currentTime = transports[0]->getCurrentPosition();
        scrubSlider.setValue(currentTime, juce::dontSendNotification);
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.
    //mixer.releaseResources();
    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{/*
    juce::Rectangle<int> thumbnailBounds(10, 350, getWidth() - 20, getHeight() - 380);
    if (thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
        paintIfFileLoaded(g, thumbnailBounds);
    auto duration = (float)transports[0]->getLengthInSeconds();

    if (duration > 0.0)
    {
        auto audioPosition = (float)transports[0]->getCurrentPosition();
        auto drawPosition = (audioPosition / duration) * (float)thumbnailBounds.getWidth() + 10;
        g.setColour(juce::Colours::green);
        g.drawLine(drawPosition, 350.0f, drawPosition, (float)getHeight()-30, 2.0f);
        if ((float)transports[0]->getCurrentPosition() >= duration) {
            transportStateChanged(Stopped);
        }
    }*/
}

void MainComponent::paintIfNoFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds)
{
    //g.setColour(juce::Colours::darkgrey);
    //g.fillRect(thumbnailBounds);
    //g.setColour(juce::Colours::white);
    //g.drawFittedText("No File Loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void MainComponent::paintIfFileLoaded(juce::Graphics& g, const juce::Rectangle<int>& thumbnailBounds) {
    //g.setColour(juce::Colours::white);
    //g.fillRect(thumbnailBounds);
    //g.setColour(juce::Colours::red);
    //thumbnail.drawChannels(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 1.0f);
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

    auto sliderLeft = 0;
    scrubSlider.setBounds(sliderLeft, 330, getWidth(), 20);
}
