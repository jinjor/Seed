#include "Components.h"

#include <JuceHeader.h>

#include <utility>

#include "StyleConstants.h"
#include "juce_core/system/juce_PlatformDefs.h"

//==============================================================================
float calcCurrentLevel(int numSamples, float* data) {
    float maxValue = 0.0;
    for (int i = 0; i < numSamples; ++i) {
        maxValue = std::max(maxValue, std::abs(data[i]));
    }
    return juce::Decibels::gainToDecibels(maxValue);
}

//==============================================================================
HeaderComponent::HeaderComponent(std::string name, HEADER_CHECK check)
    : enabledButton("Enabled"), name(std::move(name)), check(check) {
    if (check != HEADER_CHECK::Hidden) {
        addAndMakeVisible(enabledButton);
        enabledButton.setEnabled(check == HEADER_CHECK::Enabled);
    }
}
HeaderComponent::~HeaderComponent() {}
void HeaderComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.setColour(colour::PANEL_NAME_BACKGROUND);
    g.fillRect(bounds);

    juce::GlyphArrangement ga;
    juce::Font font = juce::Font(PANEL_NAME_FONT_SIZE, juce::Font::plain).withTypefaceStyle("Bold");
    ga.addFittedText(font, name, 0, 0, bounds.getHeight(), bounds.getWidth(), juce::Justification::right, 1);
    juce::Path p;
    ga.createPath(p);
    auto pathBounds = ga.getBoundingBox(0, name.length(), true);
    p.applyTransform(
        juce::AffineTransform()
            .rotated(-juce::MathConstants<float>::halfPi, 0, 0)
            .translated(0, bounds.getHeight() + (check == HEADER_CHECK::Hidden ? 8.0 : PANEL_NAME_HEIGHT) + 1.0));
    g.setColour(colour::TEXT);
    g.fillPath(p);
}
void HeaderComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto enabledButtonArea = bounds.removeFromTop(bounds.getWidth());
    enabledButton.setBounds(enabledButtonArea.reduced(6));
}

//==============================================================================
SectionComponent::SectionComponent(std::string name, HEADER_CHECK check, std::unique_ptr<juce::Component> _body)
    : header(std::move(name), check), body(std::move(_body)) {
    header.enabledButton.setLookAndFeel(&seedLookAndFeel);
    header.enabledButton.addListener(this);
    addAndMakeVisible(header);
    addAndMakeVisible(*body);
}
SectionComponent::~SectionComponent() {}
void SectionComponent::paint(juce::Graphics& g) {}
void SectionComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto headerArea = bounds.removeFromLeft(PANEL_NAME_HEIGHT);
    header.setBounds(headerArea);
    body->setBounds(bounds);
}
void SectionComponent::addListener(Listener* newListener) { listeners.add(newListener); }
void SectionComponent::setEnabled(bool enabled) {
    header.enabledButton.setToggleState(enabled, juce::dontSendNotification);
    body->setEnabled(enabled);
}
bool SectionComponent::getEnabled() { return header.enabledButton.getToggleState(); }
void SectionComponent::buttonClicked(juce::Button* button) {
    if (button == &header.enabledButton) {
        body->setEnabled(getEnabled());
        listeners.call([this](Listener& l) { l.enabledChanged(this); });
    }
}

//==============================================================================

JustRectangle::JustRectangle(Colour colour) : colour(colour) { setInterceptsMouseClicks(false, false); }
JustRectangle::~JustRectangle() {}
void JustRectangle::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    g.setColour(colour);
    g.fillAll();
}

//==============================================================================

SliderGrip::SliderGrip(Colour colour, bool horizontal) : colour(colour), horizontal(horizontal) {
    setMouseCursor(horizontal ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::UpDownResizeCursor);
}
SliderGrip::~SliderGrip() {}
void SliderGrip::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    float width = bounds.getWidth();
    float height = bounds.getHeight();

    Path path;
    if (horizontal) {
        float arrowSize = width / 2;
        path.startNewSubPath(bounds.getX(), bounds.getY());
        path.lineTo(bounds.getRight(), bounds.getY());
        path.lineTo(bounds.getRight(), bounds.getBottom() - arrowSize);
        path.lineTo(bounds.getCentreX(), bounds.getBottom());
        path.lineTo(bounds.getX(), bounds.getBottom() - arrowSize);
        path.closeSubPath();
    } else {
        float arrowSize = height / 2;
        path.startNewSubPath(bounds.getX(), bounds.getY());
        path.lineTo(bounds.getRight() - arrowSize, bounds.getY());
        path.lineTo(bounds.getRight(), bounds.getCentreY());
        path.lineTo(bounds.getRight() - arrowSize, bounds.getBottom());
        path.lineTo(bounds.getX(), bounds.getBottom());
        path.closeSubPath();
    }
    g.setColour(colour);
    g.fillPath(path);
}

//==============================================================================
ArrowButton2::ArrowButton2(const String& name, float arrowDirectionInRadians, Colour arrowColour)
    : Button(name), colour(arrowColour) {
    path.addTriangle(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.5f);
    path.applyTransform(AffineTransform::rotation(MathConstants<float>::twoPi * arrowDirectionInRadians, 0.5f, 0.5f));
}

ArrowButton2::~ArrowButton2() {}

void ArrowButton2::paintButton(Graphics& g, bool /*shouldDrawButtonAsHighlighted*/, bool shouldDrawButtonAsDown) {
    auto bounds = getLocalBounds();
    auto enabled = this->isEnabled();

    // g.setColour(colour::PIT);
    // g.fillRect(bounds);

    Path p(path);

    const float offset = shouldDrawButtonAsDown ? 1.0f : 0.0f;
    auto width = (float)getWidth();
    auto height = (float)getHeight();
    auto x = width / 2 + offset;
    auto y = height / 2 + offset;
    p.applyTransform(path.getTransformToScaleToFit(x - 4, y - 2, 8, 4, false));

    DropShadow(Colours::black.withAlpha(0.3f), shouldDrawButtonAsDown ? 2 : 4, Point<int>()).drawForPath(g, p);
    g.setColour(colour.withAlpha(enabled ? 1.0f : 0.3f));
    g.fillPath(p);
}

//==============================================================================
IncDecButton::IncDecButton()
    : incButton("Inc Button", 0.75, colour::TEXT),
      decButton("Dec Button", 0.25, colour::TEXT),
      slider(juce::Slider::SliderStyle::RotaryVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox) {
    label.setColour(juce::Label::textColourId, colour::TEXT);
    label.setText(juce::String(value), juce::dontSendNotification);
    label.setJustificationType(Justification::centred);
    this->addAndMakeVisible(label);

    incButton.setEnabled(value < max);
    this->addAndMakeVisible(incButton);
    incButton.addListener(this);

    decButton.setEnabled(min < value);
    this->addAndMakeVisible(decButton);
    decButton.addListener(this);

    slider.setRange(min, max, 1);
    slider.setValue(value);
    this->addAndMakeVisible(slider);
    slider.addListener(this);
}
IncDecButton::~IncDecButton() {}
void IncDecButton::paint(juce::Graphics& g) {}
void IncDecButton::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto incButtonArea = bounds.removeFromTop(12);  // TODO もう少し範囲を広げたい
    incButton.setBounds(incButtonArea);
    auto decButtonArea = bounds.removeFromBottom(12);  // TODO
    decButton.setBounds(decButtonArea);
    label.setBounds(bounds);
    slider.setBounds(bounds);
}
void IncDecButton::setRange(int min, int max) {
    jassert(min < max);
    this->min = min;
    this->max = max;
    incButton.setEnabled(value < max);
    decButton.setEnabled(min < value);
    slider.setRange(min, max, 1);
}
void IncDecButton::setValue(int newValue, NotificationType notification) {
    jassert(min <= newValue);
    jassert(newValue <= max);
    value = newValue;
    label.setText(juce::String(newValue), notification);
    incButton.setEnabled(value < max);
    decButton.setEnabled(min < value);
    slider.setValue(value);
}
int IncDecButton::getValue() { return value; }
void IncDecButton::addListener(IncDecButton::Listener* l) { listeners.add(l); }
void IncDecButton::removeListener(IncDecButton::Listener* l) { listeners.remove(l); }
void IncDecButton::buttonClicked(juce::Button* button) {
    if (button == &incButton) {
        value++;
    } else if (button == &decButton) {
        value--;
    }
    incButton.setEnabled(value < max);
    decButton.setEnabled(min < value);
    slider.setValue(value);
    listeners.call([this](Listener& l) { l.incDecValueChanged(this); });
}
void IncDecButton::sliderValueChanged(juce::Slider* _slider) {
    if (_slider == &slider) {
        value = slider.getValue();
        incButton.setEnabled(value < max);
        decButton.setEnabled(min < value);
        listeners.call([this](Listener& l) { l.incDecValueChanged(this); });
    }
}

//==============================================================================
StatusComponent::StatusComponent(LatestDataProvider* latestDataProvider) : latestDataProvider(latestDataProvider) {
    latestDataProvider->addConsumer(&levelConsumer);

    initStatusValue(volumeValueLabel, "0.0dB", *this);

    initStatusKey(volumeLabel, "Peak", *this);

    startTimerHz(4.0f);
}

StatusComponent::~StatusComponent() { latestDataProvider->removeConsumer(&levelConsumer); }

void StatusComponent::paint(juce::Graphics& g) {}

void StatusComponent::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds.reduce(0, 10);
    auto boundsHeight = bounds.getHeight();
    auto boundsWidth = bounds.getWidth();
    consumeKeyValueText(bounds, boundsHeight / 3, boundsWidth * 0.4, volumeLabel, volumeValueLabel);
}
void StatusComponent::timerCallback() {
    if (overflowWarning > 0) {
        volumeValueLabel.setColour(juce::Label::textColourId, colour::ERROR);
        auto levelStr = juce::String(overflowedLevel, 1) + " dB";
        volumeValueLabel.setText(levelStr, juce::dontSendNotification);

        overflowWarning--;
    } else {
        volumeValueLabel.removeColour(juce::Label::textColourId);

        if (levelConsumer.ready) {
            float levelLdB = calcCurrentLevel(levelConsumer.numSamples, levelConsumer.destinationL);
            float levelRdB = calcCurrentLevel(levelConsumer.numSamples, levelConsumer.destinationR);
            auto leveldB = std::max(levelLdB, levelRdB);
            auto levelStr = (leveldB <= -100 ? "-Inf" : juce::String(leveldB, 1)) + " dB";
            volumeValueLabel.setText(levelStr, juce::dontSendNotification);
            levelConsumer.ready = false;
            if (leveldB > 0) {
                overflowedLevel = leveldB;
                overflowWarning = 4 * 1.2;
            }
        }
    }
}

//==============================================================================
AnalyserToggleItem::AnalyserToggleItem(std::string name) {
    initLabel(nameLabel, PARAM_LABEL_FONT_SIZE, "Regular", juce::Justification::right, std::move(name), *this);
}
AnalyserToggleItem::~AnalyserToggleItem() {}
void AnalyserToggleItem::paint(juce::Graphics& g) {
    juce::Rectangle<int> bounds = getLocalBounds().removeFromRight(3).reduced(0, 4);

    auto color = value ? colour::SELECT : colour::PIT;
    g.setColour(color);
    g.fillRect(bounds);
}
void AnalyserToggleItem::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    bounds.removeFromRight(5);
    nameLabel.setBounds(bounds);
}
void AnalyserToggleItem::addListener(Listener* l) { listeners.add(l); }
void AnalyserToggleItem::mouseUp(const juce::MouseEvent& e) {
    std::cout << "mouseup:" << nameLabel.getText() << std::endl;
    Component::BailOutChecker checker(this);
    //    if (checker.shouldBailOut()) {
    //        return;
    //    }
    if (e.mouseWasClicked()) {
        if (!value) {
            value = true;
            listeners.callChecked(checker, [this](AnalyserToggleItem::Listener& l) { l.toggleItemSelected(this); });
        }
    }
}

//==============================================================================
AnalyserToggle::AnalyserToggle(ANALYSER_MODE* analyserMode) : analyserMode(analyserMode), spectrumToggle("Spectrum") {
    spectrumToggle.addListener(this);
    addAndMakeVisible(spectrumToggle);

    spectrumToggle.setValue(*analyserMode == ANALYSER_MODE::Spectrum);
}
AnalyserToggle::~AnalyserToggle() {}
void AnalyserToggle::paint(juce::Graphics& g) {}
void AnalyserToggle::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    spectrumToggle.setBounds(bounds.removeFromTop(25));
}
void AnalyserToggle::toggleItemSelected(AnalyserToggleItem* toggleItem) {
    if (toggleItem == &spectrumToggle) {
        *analyserMode = ANALYSER_MODE::Spectrum;
    }
    spectrumToggle.setValue(*analyserMode == ANALYSER_MODE::Spectrum);
}

//==============================================================================
AnalyserWindow::AnalyserWindow(ANALYSER_MODE* analyserMode, LatestDataProvider* latestDataProvider)
    : analyserMode(analyserMode),
      latestDataProvider(latestDataProvider),
      forwardFFT(fftOrder),
      window(fftSize, juce::dsp::WindowingFunction<float>::hann) {
    latestDataProvider->addConsumer(&fftConsumer);
    latestDataProvider->addConsumer(&levelConsumer);

    startTimerHz(30.0f);
}
AnalyserWindow::~AnalyserWindow() {
    latestDataProvider->removeConsumer(&fftConsumer);
    latestDataProvider->removeConsumer(&levelConsumer);
}

void AnalyserWindow::resized() {}
void AnalyserWindow::timerCallback() {
    stopTimer();
    bool shouldRepaint = false;

    switch (*analyserMode) {
        case ANALYSER_MODE::Spectrum: {
            lastAnalyserMode = ANALYSER_MODE::Spectrum;
            if (fftConsumer.ready) {
                auto hasData = drawNextFrameOfSpectrum();
                fftConsumer.ready = false;
                readyToDrawFrame = true;
                shouldRepaint = shouldRepaint || hasData;
            }
            if (levelConsumer.ready) {
                auto hasData = drawNextFrameOfLevel();
                levelConsumer.ready = false;
                //        readyToDrawFrame = true;
                shouldRepaint = shouldRepaint || hasData;
            }
            break;
        }
    }
    startTimerHz(30.0f);
    if (shouldRepaint) {
        repaint();
    }
}

bool AnalyserWindow::drawNextFrameOfSpectrum() {
    bool hasData = false;
    for (int i = 0; i < fftSize; i++) {
        fftData[i] = (fftData[i] + fftData[i + fftSize]) * 0.5f;
        if (fftData[i] != 0.0f) {
            hasData = true;
        }
        fftData[i + fftSize] = 0;
    }
    if (!hasData) {
        return false;
    }
    window.multiplyWithWindowingTable(fftData, fftSize);
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);

    auto sampleRate = 48000;  // TODO: ?
    auto minFreq = 40.0f;
    auto maxFreq = 20000.0f;
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    for (int i = 0; i < scopeSize; ++i) {
        float hz = xToHz(minFreq, maxFreq, (float)i / scopeSize);
        float gain = getFFTDataByHz(fftData, fftSize, sampleRate, hz);
        auto level = juce::jmap(juce::Decibels::gainToDecibels(gain) - juce::Decibels::gainToDecibels((float)fftSize),
                                mindB,
                                maxdB,
                                0.0f,
                                1.0f);
        scopeData[i] = level;
    }
    return true;
}
bool AnalyserWindow::drawNextFrameOfLevel() {
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    bool hasData = false;
    for (int i = 0; i < 2; i++) {
        auto* data = i == 0 ? levelConsumer.destinationL : levelConsumer.destinationR;
        auto db = calcCurrentLevel(levelConsumer.numSamples, data);
        currentLevel[i] = juce::jmap(db, mindB, maxdB, 0.0f, 1.0f);
        if (currentLevel[i] > mindB) {
            hasData = true;
        }
        if (db > 0) {
            (i == 0 ? overflowedLevelL : overflowedLevelR) = db;
            (i == 0 ? overflowWarningL : overflowWarningR) = 30 * 1.2;
        }
    }
    return hasData;
}
void AnalyserWindow::paint(juce::Graphics& g) {
    g.fillAll(colour::ANALYSER_BACKGROUND);

    juce::Rectangle<int> bounds = getLocalBounds();

    if (readyToDrawFrame) {
        auto offsetX = 2;
        auto offsetY = 2;
        auto displayBounds = bounds.reduced(offsetX, offsetY);
        auto height = displayBounds.getHeight();

        auto levelWidth = 8;
        auto spectrumWidth = displayBounds.getWidth() - levelWidth * 2;

        paintSpectrum(g, colour::ANALYSER_LINE, offsetX, offsetY, spectrumWidth, height, scopeData);
        offsetX += spectrumWidth;
        paintLevel(g, offsetX, offsetY, levelWidth, height, currentLevel[0]);
        offsetX += levelWidth;
        paintLevel(g, offsetX, offsetY, levelWidth, height, currentLevel[1]);
    }
    g.setColour(colour::ANALYSER_BORDER);
    g.drawRect(bounds, 2.0f);
}
void AnalyserWindow::paintSpectrum(
    juce::Graphics& g, juce::Colour colour, int offsetX, int offsetY, int width, int height, float* scopeData) {
    g.setColour(colour);
    for (int i = 1; i < scopeSize; ++i) {
        g.drawLine({offsetX + (float)juce::jmap(i - 1, 0, scopeSize - 1, 0, width),
                    offsetY - 0.5f + juce::jmap(scopeData[i - 1], 0.0f, 1.0f, (float)height, 0.0f),
                    offsetX + (float)juce::jmap(i, 0, scopeSize - 1, 0, width),
                    offsetY - 0.5f + juce::jmap(scopeData[i], 0.0f, 1.0f, (float)height, 0.0f)});
    }
}
void AnalyserWindow::paintLevel(juce::Graphics& g, int offsetX, int offsetY, int width, int height, float level) {
    g.setColour(colour::ANALYSER_LINE);
    if (overflowWarningL > 0) {
        g.setColour(colour::ERROR);
        overflowWarningL--;
    }
    int barWidth = width - 1;
    int barHeight = level * height;
    g.fillRect(offsetX + 1, offsetY + height - barHeight, barWidth, barHeight);
}

//==============================================================================
AnalyserWindow2::AnalyserWindow2(Recorder& recorder, AllParams& allParams)
    : recorder(recorder),
      allParams(allParams),
      forwardFFT(FFT_ORDER),
      window(FFT_SIZE, juce::dsp::WindowingFunction<float>::hann),
      envelopeLine{colour::ENVELOPE_LINE},
      spectrumLine{colour::SPECTRUM_LINE},
      entryButtons{juce::ToggleButton{"1"}, juce::ToggleButton{"2"}, juce::ToggleButton{"3"}, juce::ToggleButton{"4"}},
      recordButton{"Record"},
      playButton{"Play"},
      stopButton{"Stop"},
      highFreqGrip{Colours::brown, false},
      lowFreqGrip(Colours::blueviolet, false),
      highFreqMask{Colour::fromRGBA(255, 255, 255, 127)},
      lowFreqMask{Colour::fromRGBA(255, 255, 255, 127)},
      playStartGrip{Colours::cornflowerblue, true},
      playingPosition{Colours::grey} {
    for (auto& entryButton : entryButtons) {
        entryButton.setLookAndFeel(&seedLookAndFeel);
        entryButton.addListener(this);
        addAndMakeVisible(entryButton);
    }
    recordButton.setLookAndFeel(&seedLookAndFeel);
    recordButton.addListener(this);
    addAndMakeVisible(recordButton);
    playButton.setLookAndFeel(&seedLookAndFeel);
    playButton.addListener(this);
    addAndMakeVisible(playButton);
    stopButton.setLookAndFeel(&seedLookAndFeel);
    stopButton.addListener(this);
    addAndMakeVisible(stopButton);
    {
        auto image = juce::Image{juce::Image::PixelFormat::RGB, TIME_SCOPE_SIZE, FREQ_SCOPE_SIZE, true};
        heatMap.setImage(image);
        heatMap.setImagePlacement(juce::RectanglePlacement::stretchToFit);
        heatMap.addMouseListener(this, false);
        addAndMakeVisible(heatMap);
    }
    addAndMakeVisible(envelopeLine);
    addAndMakeVisible(spectrumLine);
    {
        auto image = juce::Image{juce::Image::PixelFormat::RGB, TIME_SCOPE_SIZE, ENVELOPE_VIEW_HEIGHT, true};
        envelopeView.setImage(image);
        envelopeView.setImagePlacement(juce::RectanglePlacement::stretchToFit);
        addAndMakeVisible(envelopeView);
    }
    {
        auto image = juce::Image{juce::Image::PixelFormat::RGB, SPECTRUM_VIEW_WIDTH, FREQ_SCOPE_SIZE, true};
        spectrumView.setImage(image);
        spectrumView.setImagePlacement(juce::RectanglePlacement::stretchToFit);
        spectrumView.addMouseListener(this, false);
        addAndMakeVisible(spectrumView);
    }
    highFreqGrip.addMouseListener(this, false);
    addAndMakeVisible(highFreqGrip);
    lowFreqGrip.addMouseListener(this, false);
    addAndMakeVisible(lowFreqGrip);
    addAndMakeVisible(highFreqMask);
    addAndMakeVisible(lowFreqMask);
    playStartGrip.addMouseListener(this, false);
    addAndMakeVisible(playStartGrip);
    addAndMakeVisible(playingPosition);

    addKeyListener(this);

    startTimerHz(30.0f);
}
AnalyserWindow2::~AnalyserWindow2() {}

void AnalyserWindow2::resized() {
    juce::Rectangle<int> bounds = getLocalBounds();
    auto inner = bounds.reduced(30, 0);

    auto toolsArea = inner.removeFromTop(30);
    for (auto& entryButton : entryButtons) {
        entryButton.setBounds(toolsArea.removeFromLeft(70));
    }
    toolsArea.removeFromLeft(20);
    recordButton.setBounds(toolsArea.removeFromLeft(100));
    playButton.setBounds(toolsArea.removeFromLeft(100));
    stopButton.setBounds(toolsArea.removeFromLeft(100));

    inner.removeFromTop(30);

    float width = inner.getWidth();
    float height = inner.getHeight();
    heatMap.setBounds(inner.withTrimmedRight(width * 0.2).withTrimmedBottom(height * 0.2));
    envelopeView.setBounds(inner.withTrimmedRight(width * 0.2).withTrimmedTop(height * 0.8 + 2.0));
    spectrumView.setBounds(inner.withTrimmedLeft(width * 0.8 + 2.0).withTrimmedBottom(height * 0.2));

    relocateFilterComponents();
    relocatePlayGuideComponents();
}
void AnalyserWindow2::timerCallback() {
    stopTimer();
    bool shouldRepaint = false;
    int currentEntryIndex = recorder.getCurrentEntryIndex();
    bool canOperate = recorder.canOperate();

    if (!calculated && canOperate) {
        for (int t = 0; t < TIME_SCOPE_SIZE; ++t) {
            calculateSpectrum(t);
        }
        calculated = true;
        drawHeatMap();
        drawEnvelopeView();
        drawSpectrumView();
        repaint();
    }
    startTimerHz(30.0f);

    auto heatMapBounds = heatMap.getBounds();
    auto focusedFreqIndex = getFocusedFreqIndex();
    envelopeLine.setBounds(
        heatMapBounds.getX(),
        heatMapBounds.getY() + heatMapBounds.getHeight() * (1.0f - (float)focusedFreqIndex / FREQ_SCOPE_SIZE),
        heatMapBounds.getWidth(),
        1);
    auto focusedTimeIndex = getFocusedTimeIndex();
    spectrumLine.setBounds(heatMapBounds.getX() + heatMapBounds.getWidth() * (float)focusedTimeIndex / TIME_SCOPE_SIZE,
                           heatMapBounds.getY(),
                           1,
                           heatMapBounds.getHeight());
    for (int i = 0; i < NUM_ENTRIES; i++) {
        entryButtons[i].setToggleState(i == currentEntryIndex, juce::dontSendNotification);
        entryButtons[i].setEnabled(canOperate);
    }
    recordButton.setEnabled(canOperate);
    playButton.setEnabled(canOperate);

    relocateFilterComponents();
    relocatePlayGuideComponents();
}
void AnalyserWindow2::relocateFilterComponents() {
    int currentEntryIndex = recorder.getCurrentEntryIndex();
    {
        float freq = allParams.entryParams[currentEntryIndex].FilterHighFreq->get();
        float scopeY = hzToX(VIEW_MIN_FREQ, VIEW_MAX_FREQ, freq) * FREQ_SCOPE_SIZE;
        float viewY = heatMap.getY() + heatMap.getHeight() * (1.0f - scopeY / FREQ_SCOPE_SIZE);
        highFreqGrip.setBounds(
            heatMap.getX() - GRIP_MARGIN - GRIP_LENGTH, viewY - (GRIP_WIDTH / 2), GRIP_LENGTH, GRIP_WIDTH);
        highFreqMask.setBounds(heatMap.getBounds().removeFromTop(viewY - heatMap.getY()));
    }
    {
        float freq = allParams.entryParams[currentEntryIndex].FilterLowFreq->get();
        float scopeY = hzToX(VIEW_MIN_FREQ, VIEW_MAX_FREQ, freq) * FREQ_SCOPE_SIZE;
        float viewY = heatMap.getY() + heatMap.getHeight() * (1.0f - scopeY / FREQ_SCOPE_SIZE);
        lowFreqGrip.setBounds(
            heatMap.getX() - GRIP_MARGIN - GRIP_LENGTH, viewY - (GRIP_WIDTH / 2), GRIP_LENGTH, GRIP_WIDTH);
        lowFreqMask.setBounds(heatMap.getBounds().removeFromBottom(heatMap.getBottom() - viewY));
    }
}
void AnalyserWindow2::relocatePlayGuideComponents() {
    int currentEntryIndex = recorder.getCurrentEntryIndex();
    {
        float playStartSec = allParams.entryParams[currentEntryIndex].PlayStartSec->get();
        float x = heatMap.getX() + heatMap.getWidth() * (playStartSec / MAX_REC_SECONDS);
        playStartGrip.setBounds(
            x - (GRIP_WIDTH / 2), heatMap.getY() - GRIP_MARGIN - GRIP_LENGTH, GRIP_WIDTH, GRIP_LENGTH);
    }
    if (recorder.isPlaying()) {
        playingPosition.setVisible(true);
        float pos = recorder.getPlayingPositionInSec();
        float x = heatMap.getX() + heatMap.getWidth() * (pos / MAX_REC_SECONDS);
        playingPosition.setBounds(x, heatMap.getY(), 1, heatMap.getHeight());
    } else {
        playingPosition.setVisible(false);
    }
}
void AnalyserWindow2::buttonClicked(juce::Button* button) {
    for (int i = 0; i < NUM_ENTRIES; i++) {
        if (button == &entryButtons[i]) {
            recorder.setCurrentEntryIndex(i);
            calculated = false;
        }
    }
    if (button == &recordButton) {
        if (recorder.canOperate()) {
            calculated = false;
            recorder.record();
            recordButton.setToggleState(false, juce::dontSendNotification);
            recordButton.setEnabled(false);
        }
    } else if (button == &playButton) {
        if (recorder.canOperate()) {
            int entryIndex = recorder.getCurrentEntryIndex();
            auto& entryParams = allParams.entryParams[entryIndex];
            recorder.play(entryParams.PlayStartSec->get(),
                          true,
                          //   allParams.FilterN->get(),
                          400,
                          entryParams.FilterLowFreq->get(),
                          entryParams.FilterHighFreq->get());  // TODO
            recordButton.setToggleState(false, juce::dontSendNotification);
            recordButton.setEnabled(false);
        }
    } else if (button == &stopButton) {
        recorder.stop();
        stopButton.setToggleState(false, juce::dontSendNotification);
    }
}
void AnalyserWindow2::mouseDown(const MouseEvent& event) {
    if (event.eventComponent == &heatMap) {
        auto bounds = heatMap.getBounds();
        auto xratio = (float)event.x / bounds.getWidth();
        auto yratio = (float)event.y / bounds.getHeight();
        auto sec = MAX_REC_SECONDS * xratio;
        auto freq = xToHz(VIEW_MIN_FREQ, VIEW_MAX_FREQ, 1.0f - yratio);
        auto& entryParams = allParams.entryParams[recorder.getCurrentEntryIndex()];
        *entryParams.FocusSec = sec;
        *entryParams.FocusFreq = freq;

        drawEnvelopeView();
        drawSpectrumView();
        repaint();
    }
}
void AnalyserWindow2::mouseDrag(const MouseEvent& event) {
    if (event.eventComponent == &highFreqGrip) {
        auto bounds = heatMap.getBounds();
        float y = getMouseXYRelative().y;
        float top = bounds.getY();
        float height = bounds.getHeight();
        auto yratio = 1.0f - (y - top) / height;
        if (yratio < 0 || yratio > 1) {
            return;
        }
        auto freq = xToHz(VIEW_MIN_FREQ, VIEW_MAX_FREQ, yratio);
        *allParams.entryParams[recorder.getCurrentEntryIndex()].FilterHighFreq = freq;
    } else if (event.eventComponent == &lowFreqGrip) {
        auto bounds = heatMap.getBounds();
        float y = getMouseXYRelative().y;
        float top = bounds.getY();
        float height = bounds.getHeight();
        auto yratio = 1.0f - (y - top) / height;
        if (yratio < 0 || yratio > 1) {
            return;
        }
        auto freq = xToHz(VIEW_MIN_FREQ, VIEW_MAX_FREQ, yratio);
        *allParams.entryParams[recorder.getCurrentEntryIndex()].FilterLowFreq = freq;
    } else if (event.eventComponent == &playStartGrip) {
        auto bounds = heatMap.getBounds();
        float x = getMouseXYRelative().x;
        float left = bounds.getX();
        float width = bounds.getWidth();
        auto xratio = (x - left) / width;
        if (xratio < 0 || xratio > 1) {
            return;
        }
        auto sec = MAX_REC_SECONDS * xratio;
        *allParams.entryParams[recorder.getCurrentEntryIndex()].PlayStartSec = sec;
    }
}
void AnalyserWindow2::mouseDoubleClick(const MouseEvent& event) {
    if (event.eventComponent == &spectrumView) {
        auto bounds = heatMap.getBounds();
        auto minFreq = VIEW_MIN_FREQ;
        auto maxFreq = VIEW_MAX_FREQ;
        auto yratio = (float)event.y / bounds.getHeight();
        auto freq = xToHz(minFreq, maxFreq, 1.0f - yratio);
        int currentEntryIndex = recorder.getCurrentEntryIndex();
        *allParams.entryParams[currentEntryIndex].BaseFreq = freq;
        drawSpectrumView();
        repaint();
    }
}
void AnalyserWindow2::calculateSpectrum(int timeScopeIndex) {
    int currentEntryIndex = recorder.getCurrentEntryIndex();
    auto& entry = recorder.entries[currentEntryIndex];
    int sampleIndex = ((float)timeScopeIndex / (float)TIME_SCOPE_SIZE) * MAX_REC_SAMPLES;
    jassert(sampleIndex >= 0);
    jassert(sampleIndex < MAX_REC_SAMPLES);
    auto& fftData = allFftData[timeScopeIndex];
    for (int i = 0; i < FFT_SIZE; i++) {
        auto dataIndex = sampleIndex - FFT_SIZE + i;
        fftData[i] = dataIndex >= 0 ? (entry.dataL[dataIndex] + entry.dataR[dataIndex]) * 0.5f : 0;
        fftData[i + FFT_SIZE] = 0;
    }
    window.multiplyWithWindowingTable(fftData, FFT_SIZE);
    forwardFFT.performFrequencyOnlyForwardTransform(fftData);

    auto sampleRate = entry.sampleRate;
    auto mindB = -100.0f;
    auto maxdB = 0.0f;
    auto& scopeData = allScopeData[timeScopeIndex];
    for (int i = 0; i < FREQ_SCOPE_SIZE; ++i) {
        float hz = xToHz(VIEW_MIN_FREQ, VIEW_MAX_FREQ, (float)i / FREQ_SCOPE_SIZE);
        float gain = getFFTDataByHz(fftData, FFT_SIZE, sampleRate, hz);
        auto level = juce::jmap(juce::Decibels::gainToDecibels(gain) - juce::Decibels::gainToDecibels((float)FFT_SIZE),
                                mindB,
                                maxdB,
                                0.0f,
                                1.0f);
        scopeData[i] = level;
    }
}
void AnalyserWindow2::drawHeatMap() {
    Graphics g(heatMap.getImage());
    for (int t = 0; t < TIME_SCOPE_SIZE; ++t) {
        auto& scopeData = allScopeData[t];
        for (int i = 0; i < FREQ_SCOPE_SIZE; ++i) {
            g.setColour(Colour::greyLevel(scopeData[i]));
            g.drawRect(t, FREQ_SCOPE_SIZE - i, 1, 1);
        }
    }
}
void AnalyserWindow2::drawEnvelopeView() {
    auto& image = envelopeView.getImage();
    auto bounds = image.getBounds();
    float width = bounds.getWidth();
    float bottom = bounds.getBottom();
    Graphics g(image);
    g.setColour(juce::Colours::black);
    g.fillRect(bounds);

    g.setColour(colour::GUIDE_LINE);
    int currentEntryIndex = recorder.getCurrentEntryIndex();
    auto& entry = recorder.entries[currentEntryIndex];
    for (int i = 1; i < MAX_REC_SECONDS; i++) {
        float x = width * ((float)i / MAX_REC_SECONDS);
        g.drawLine({x, 0, x, bottom});
    }

    g.setColour(colour::ENVELOPE_LINE);
    int y = getFocusedFreqIndex();
    for (int x = 1; x < TIME_SCOPE_SIZE; ++x) {
        auto prev = allScopeData[x - 1][y];
        auto curr = allScopeData[x][y];
        g.drawLine({(float)x - 1, (1 - prev) * ENVELOPE_VIEW_HEIGHT, (float)x, (1 - curr) * ENVELOPE_VIEW_HEIGHT});
    }
}
void AnalyserWindow2::drawSpectrumView() {
    auto& image = spectrumView.getImage();
    Graphics g(image);
    g.setColour(juce::Colours::black);
    g.fillRect(image.getBounds());

    int currentEntryIndex = recorder.getCurrentEntryIndex();
    for (int i = 0; i < 16; i++) {
        float freq = allParams.entryParams[currentEntryIndex].BaseFreq->get() * (i + 1);
        if (freq > VIEW_MAX_FREQ) {
            break;
        }
        float y = hzToX(VIEW_MIN_FREQ, VIEW_MAX_FREQ, freq) * FREQ_SCOPE_SIZE;
        g.setColour(colour::GUIDE_LINE.brighter(i % 4 == 0 ? 0.7 : 0));
        g.drawLine({0, ((float)FREQ_SCOPE_SIZE - 1) - y, SPECTRUM_VIEW_WIDTH - 1, ((float)FREQ_SCOPE_SIZE - 1) - y});
    }

    g.setColour(colour::SPECTRUM_LINE);
    int x = getFocusedTimeIndex();
    for (int y = 1; y < FREQ_SCOPE_SIZE; ++y) {
        auto prev = allScopeData[x][y - 1];
        auto curr = allScopeData[x][y];
        g.drawLine({prev * SPECTRUM_VIEW_WIDTH,
                    ((float)FREQ_SCOPE_SIZE - 1) - ((float)y - 1),
                    curr * SPECTRUM_VIEW_WIDTH,
                    ((float)FREQ_SCOPE_SIZE - 1) - (float)y});
    }
}
void AnalyserWindow2::paint(juce::Graphics& g) {}
bool AnalyserWindow2::keyPressed(const KeyPress& key, Component* originatingComponent) {
    if (key.getKeyCode() == juce::KeyPress::upKey) {
        auto focusedFreqIndex = getFocusedFreqIndex();
        if (focusedFreqIndex < FREQ_SCOPE_SIZE - 1) {
            focusedFreqIndex++;
            drawEnvelopeView();
            repaint();
        }
        return true;
    } else if (key.getKeyCode() == juce::KeyPress::downKey) {
        auto focusedFreqIndex = getFocusedFreqIndex();
        if (focusedFreqIndex > 0) {
            focusedFreqIndex--;
            drawEnvelopeView();
            repaint();
        }
        return true;
    } else if (key.getKeyCode() == juce::KeyPress::leftKey) {
        auto focusedTimeIndex = getFocusedTimeIndex();
        if (focusedTimeIndex > 1) {
            focusedTimeIndex -= 2;
            drawSpectrumView();
            repaint();
        }
        return true;
    } else if (key.getKeyCode() == juce::KeyPress::rightKey) {
        auto focusedTimeIndex = getFocusedTimeIndex();
        if (focusedTimeIndex < TIME_SCOPE_SIZE - 2) {
            focusedTimeIndex += 2;
            drawSpectrumView();
            repaint();
        }
        return true;
    }
    return false;
}
bool AnalyserWindow2::keyStateChanged(bool isKeyDown, Component* originatingComponent) { return false; }