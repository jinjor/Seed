#pragma once

#include <JuceHeader.h>

#include "LookAndFeel.h"
#include "PluginProcessor.h"
#include "StyleConstants.h"

using namespace styles;

enum class ANALYSER_MODE { Spectrum };

//==============================================================================

class JustRectangle : public Component {
public:
    JustRectangle(Colour colour);
    ~JustRectangle() override;

private:
    Colour colour;
    virtual void paint(juce::Graphics& g) override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JustRectangle)
};

//==============================================================================

class ArrowButton2 : public Button {
public:
    ArrowButton2(const String& buttonName, float arrowDirection, Colour arrowColour);
    ~ArrowButton2() override;
    void paintButton(Graphics&, bool, bool) override;

private:
    Colour colour;
    Path path;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ArrowButton2)
};

//==============================================================================

class IncDecButton : public juce::Component, juce::Button::Listener, juce::Slider::Listener {
public:
    IncDecButton();
    virtual ~IncDecButton();
    IncDecButton(const IncDecButton&) = delete;
    ArrowButton2 incButton;
    ArrowButton2 decButton;
    juce::Label label;
    juce::Slider slider;
    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;
    void setRange(int min, int max);
    void setValue(int newValue, NotificationType notification);
    int getValue();

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void incDecValueChanged(IncDecButton*) = 0;
    };
    void addListener(Listener* newListener);
    void removeListener(Listener* listener);
    virtual void buttonClicked(juce::Button* button) override;

private:
    int min = 0;
    int max = 1;
    int value = 0;
    std::string name;
    ListenerList<Listener> listeners;
    virtual void sliderValueChanged(juce::Slider* slider) override;
};

//==============================================================================
class ComponentHelper {
protected:
    SeedLookAndFeel seedLookAndFeel;
    SeedLookAndFeel seedLookAndFeelControlled = SeedLookAndFeel(true);
    juce::Font paramLabelFont = juce::Font(PARAM_LABEL_FONT_SIZE, juce::Font::plain).withTypefaceStyle("Regular");
    juce::Font paramValueLabelFont =
        juce::Font(PARAM_VALUE_LABEL_FONT_SIZE, juce::Font::plain).withTypefaceStyle("Regular");

    void initLabel(juce::Label& label,
                   int fontSize,
                   std::string&& typeFaceStyle,
                   juce::Justification justification,
                   std::string&& text,
                   juce::Component& parent) {
        juce::Font paramLabelFont = juce::Font(fontSize, juce::Font::plain).withTypefaceStyle(typeFaceStyle);

        label.setFont(paramLabelFont);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(justification);
        label.setInterceptsMouseClicks(false, false);
        parent.addAndMakeVisible(label);
    }
    void initLabel(juce::Label& label, std::string&& text, juce::Component& parent) {
        label.setFont(paramLabelFont);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setEditable(false, false, false);
        parent.addAndMakeVisible(label);
    }
    void initStatusValue(juce::Label& label, std::string&& text, juce::Component& parent) {
        label.setFont(paramValueLabelFont);
        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::left);
        label.setEditable(false, false, false);
        parent.addAndMakeVisible(label);
    }
    void initStatusKey(juce::Label& label, std::string&& text, juce::Component& parent) {
        label.setFont(paramLabelFont);
        label.setText(text + ":", juce::dontSendNotification);
        label.setJustificationType(juce::Justification::right);
        label.setEditable(false, false, false);
        parent.addAndMakeVisible(label);
    }
    void initChoice(juce::ComboBox& box,
                    const juce::StringArray& allValueStrings,
                    int selectedIndex,
                    juce::ComboBox::Listener* listener,
                    juce::Component& parent) {
        box.setLookAndFeel(&seedLookAndFeel);
        box.addItemList(allValueStrings, 1);
        box.setSelectedItemIndex(selectedIndex, juce::dontSendNotification);
        box.setJustificationType(juce::Justification::centred);
        box.addListener(listener);
        parent.addAndMakeVisible(box);
    }
    void initChoice(juce::ComboBox& box,
                    juce::AudioParameterChoice* param,
                    juce::ComboBox::Listener* listener,
                    juce::Component& parent) {
        initChoice(box, param->getAllValueStrings(), param->getIndex(), listener, parent);
    }
    void initChoice(juce::ComboBox& box,
                    juce::AudioParameterBool* param,
                    juce::ComboBox::Listener* listener,
                    juce::Component& parent) {
        initChoice(box, param->getAllValueStrings(), param->get(), listener, parent);
    }
    void initChoiceToggle(juce::ToggleButton& toggle,
                          int checkIndex,
                          juce::AudioParameterChoice* param,
                          juce::ToggleButton::Listener* listener,
                          juce::Component& parent) {
        toggle.setLookAndFeel(&seedLookAndFeel);
        toggle.addListener(listener);
        toggle.setButtonText("");
        toggle.setToggleState(param->getIndex() == checkIndex, juce::dontSendNotification);
        parent.addAndMakeVisible(toggle);
    }
    void initChoiceToggle(juce::ToggleButton& toggle,
                          juce::AudioParameterBool* param,
                          juce::ToggleButton::Listener* listener,
                          juce::Component& parent) {
        toggle.setLookAndFeel(&seedLookAndFeel);
        toggle.addListener(listener);
        toggle.setButtonText("");
        toggle.setToggleState(param->get(), juce::dontSendNotification);
        parent.addAndMakeVisible(toggle);
    }
    void initSkewFromMid(juce::Slider& slider,
                         juce::AudioParameterFloat* param,
                         float step,
                         const char* unit,
                         std::function<juce::String(double)>&& format,
                         juce::Slider::Listener* listener,
                         juce::Component& parent) {
        slider.setLookAndFeel(&seedLookAndFeel);
        auto nrange = NormalisableRange<double>{
            param->range.start, param->range.end, step, param->range.skew, param->range.symmetricSkew};
        slider.setNormalisableRange(nrange);
        slider.setValue(param->get(), juce::dontSendNotification);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        slider.setScrollWheelEnabled(false);
        if (unit != nullptr) {
            slider.setTextValueSuffix(unit);
        }
        if (format != nullptr) {
            slider.textFromValueFunction = format;
        }
        slider.addListener(listener);
        parent.addAndMakeVisible(slider);
    }
    void initLinear(juce::Slider& slider,
                    juce::AudioParameterFloat* param,
                    float step,
                    const char* unit,
                    std::function<juce::String(double)>&& format,
                    juce::Slider::Listener* listener,
                    juce::Component& parent) {
        slider.setLookAndFeel(&seedLookAndFeel);
        slider.setRange(param->range.start, param->range.end, step);
        slider.setValue(param->get(), juce::dontSendNotification);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        slider.setScrollWheelEnabled(false);
        if (unit != nullptr) {
            slider.setTextValueSuffix(unit);
        }
        if (format != nullptr) {
            slider.textFromValueFunction = format;
        }
        slider.addListener(listener);
        parent.addAndMakeVisible(slider);
    }
    void initLinear(juce::Slider& slider,
                    juce::AudioParameterInt* param,
                    float step,
                    const char* unit,
                    std::function<juce::String(double)>&& format,
                    juce::Slider::Listener* listener,
                    juce::Component& parent) {
        slider.setLookAndFeel(&seedLookAndFeel);
        slider.setRange(param->getRange().getStart(), param->getRange().getEnd(), step);
        slider.setValue(param->get(), juce::dontSendNotification);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        slider.setScrollWheelEnabled(false);
        if (unit != nullptr) {
            slider.setTextValueSuffix(unit);
        }
        if (format != nullptr) {
            slider.textFromValueFunction = format;
        }
        slider.addListener(listener);
        parent.addAndMakeVisible(slider);
    }
    void initLinear(juce::Slider& slider,
                    juce::AudioParameterFloat* param,
                    float step,
                    juce::Slider::Listener* listener,
                    juce::Component& parent) {
        initLinear(slider, param, step, nullptr, nullptr, listener, parent);
    }
    void initLinear(juce::Slider& slider,
                    juce::AudioParameterInt* param,
                    juce::Slider::Listener* listener,
                    juce::Component& parent) {
        initLinear(slider, param, 1, nullptr, nullptr, listener, parent);
    }
    void initLinearPercent(juce::Slider& slider,
                           juce::AudioParameterFloat* param,
                           float step,
                           juce::Slider::Listener* listener,
                           juce::Component& parent) {
        auto f = [](double gain) { return juce::String(gain * 100, 0) + " %"; };
        initLinear(slider, param, step, nullptr, std::move(f), listener, parent);
    }
    void initIncDec(IncDecButton& incDec,
                    juce::AudioParameterInt* param,
                    IncDecButton::Listener* listener,
                    juce::Component& parent) {
        incDec.setLookAndFeel(&seedLookAndFeel);
        incDec.setRange(param->getRange().getStart(), param->getRange().getEnd());
        incDec.setValue(param->get(), juce::dontSendNotification);
        incDec.addListener(listener);
        parent.addAndMakeVisible(incDec);
    }
    void initEnum(juce::Slider& slider,
                  juce::AudioParameterChoice* param,
                  juce::Slider::Listener* listener,
                  juce::Component& parent) {
        const juce::StringArray& values = param->getAllValueStrings();
        slider.setLookAndFeel(&seedLookAndFeel);
        slider.setRange(0, values.size() - 1, 1);
        slider.setValue(param->getIndex(), juce::dontSendNotification);
        slider.setPopupDisplayEnabled(true, true, nullptr);
        slider.setScrollWheelEnabled(false);
        slider.textFromValueFunction = [values](double index) { return values[index]; };
        slider.addListener(listener);
        parent.addAndMakeVisible(slider);
    }

    void consumeLabeledKnob(juce::Rectangle<int>& parentArea, juce::Label& label, juce::Slider& knob) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(SLIDER_WIDTH);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        knob.setBounds(area.removeFromTop(KNOB_HEIGHT));
    }
    void consumeLabeledKnob(juce::Rectangle<int>& parentArea,
                            juce::Label& label,
                            juce::Slider& knob1,
                            juce::Slider& knob2) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(SLIDER_WIDTH);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        auto knobBounds = area.removeFromTop(KNOB_HEIGHT);
        knob1.setBounds(knobBounds);
        knob2.setBounds(knobBounds);
    }
    void consumeLabeledKnob(juce::Rectangle<int>& parentArea,
                            juce::Label& label1,
                            juce::Slider& knob1,
                            juce::Label& label2,
                            juce::Slider& knob2) {
        auto copied = parentArea;
        consumeLabeledKnob(parentArea, label1, knob1);
        consumeLabeledKnob(copied, label2, knob2);
    }
    void consumeLabeledComboBox(juce::Rectangle<int>& parentArea, int width, juce::Label& label, juce::Component& box) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(width);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        box.setBounds(area.removeFromTop(COMBO_BOX_HEIGHT));
    }
    void consumeLabeledIncDecButton(juce::Rectangle<int>& parentArea,
                                    int width,
                                    juce::Label& label,
                                    juce::Component& button) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(width);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        button.setBounds(area.removeFromTop(KNOB_HEIGHT));  // TODO
    }
    void consumeLabeledToggle(juce::Rectangle<int>& parentArea,
                              int width,
                              juce::Label& label,
                              juce::ToggleButton& toggle) {
        parentArea.removeFromLeft(PARAM_MARGIN_LEFT);
        auto area = parentArea.removeFromLeft(width);
        label.setBounds(area.removeFromTop(LABEL_HEIGHT));
        area.removeFromTop(LABEL_MARGIN_BOTTOM);
        const auto SIZE = 12.0f;
        auto padding = (width - SIZE) / 2.0f;
        toggle.setBounds(area.removeFromTop(SIZE).removeFromLeft(padding + SIZE).removeFromRight(SIZE));
    }
    void consumeKeyValueText(
        juce::Rectangle<int>& parentArea, int height, int width, juce::Label& keyLabel, juce::Label& valueLabel) {
        auto area = parentArea.removeFromTop(height);
        keyLabel.setBounds(area.removeFromLeft(width));
        area.removeFromLeft(3);
        valueLabel.setBounds(area);
    }
};

//==============================================================================

enum class HEADER_CHECK { Hidden, Disabled, Enabled };

class HeaderComponent : public juce::Component {
public:
    HeaderComponent(std::string name, HEADER_CHECK check);
    virtual ~HeaderComponent();
    HeaderComponent(const HeaderComponent&) = delete;
    juce::ToggleButton enabledButton;
    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    std::string name;
    HEADER_CHECK check;
};

//==============================================================================

class SectionComponent : public juce::Component, juce::Button::Listener {
public:
    SectionComponent(std::string name, HEADER_CHECK check, std::unique_ptr<juce::Component> body);
    virtual ~SectionComponent();
    SectionComponent(const SectionComponent&) = delete;
    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void enabledChanged(SectionComponent*) = 0;
    };
    void addListener(Listener* newListener);
    void setEnabled(bool enabled);
    bool getEnabled();

private:
    SeedLookAndFeel seedLookAndFeel;
    HeaderComponent header;
    std::unique_ptr<juce::Component> body;
    ListenerList<Listener> listeners;
    virtual void buttonClicked(juce::Button* button) override;
};

//==============================================================================
class StatusComponent : public juce::Component, private juce::Timer, ComponentHelper {
public:
    StatusComponent(LatestDataProvider* latestDataProvider);
    virtual ~StatusComponent();
    StatusComponent(const StatusComponent&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    virtual void timerCallback() override;
    TimeConsumptionState* timeConsumptionState;
    LatestDataProvider* latestDataProvider;

    juce::Label volumeValueLabel;

    juce::Label volumeLabel;

    float levelDataL[2048];
    float levelDataR[2048];
    LatestDataProvider::Consumer levelConsumer{levelDataL, levelDataR, 2048, false};
    float overflowedLevel = 0;
    int overflowWarning = 0;
};

//==============================================================================
class AnalyserToggleItem : public juce::Component, private ComponentHelper {
public:
    AnalyserToggleItem(std::string name);
    virtual ~AnalyserToggleItem();
    AnalyserToggleItem(const AnalyserToggleItem&) = delete;

    void setValue(bool value) {
        this->value = value;
        repaint();
    };
    bool getValue() { return value; };

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

    class Listener {
    public:
        virtual ~Listener() = default;
        virtual void toggleItemSelected(AnalyserToggleItem*) = 0;
    };
    void addListener(Listener* e);

private:
    virtual void mouseUp(const juce::MouseEvent& e) override;

    juce::ListenerList<Listener> listeners;
    ANALYSER_MODE* analyserMode;
    juce::Label nameLabel;
    bool value;
};

//==============================================================================
class AnalyserToggle : public juce::Component, private AnalyserToggleItem::Listener {
public:
    AnalyserToggle(ANALYSER_MODE* analyserMode);
    virtual ~AnalyserToggle();
    AnalyserToggle(const AnalyserToggle&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    ANALYSER_MODE* analyserMode;
    AnalyserToggleItem spectrumToggle;

    virtual void toggleItemSelected(AnalyserToggleItem* toggleItem) override;
};

//==============================================================================
class AnalyserWindow : public juce::Component, private juce::Timer {
public:
    AnalyserWindow(ANALYSER_MODE* analyserMode, LatestDataProvider* latestDataProvider);
    virtual ~AnalyserWindow();
    AnalyserWindow(const AnalyserWindow&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    enum { scopeSize = 512 };
    ANALYSER_MODE* analyserMode;
    LatestDataProvider* latestDataProvider;
    ANALYSER_MODE lastAnalyserMode = ANALYSER_MODE::Spectrum;

    // FFT
    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    static const int fftOrder = 11;
    static const int fftSize = 2048;
    float fftData[fftSize * 2];
    LatestDataProvider::Consumer fftConsumer{fftData, fftData + fftSize, fftSize, false};
    float scopeData[scopeSize]{};
    bool readyToDrawFrame = false;

    // Level
    float levelDataL[2048];
    float levelDataR[2048];
    LatestDataProvider::Consumer levelConsumer{levelDataL, levelDataR, 2048, false};
    float currentLevel[2]{};
    float overflowedLevelL = 0;
    float overflowedLevelR = 0;
    int overflowWarningL = 0;
    int overflowWarningR = 0;

    // methods
    virtual void timerCallback() override;
    bool drawNextFrameOfSpectrum();
    bool drawNextFrameOfLevel();
    void paintSpectrum(
        juce::Graphics& g, juce::Colour colour, int offsetX, int offsetY, int width, int height, float* scopeData);
    void paintLevel(juce::Graphics& g, int offsetX, int offsetY, int width, int height, float level);
    static float xToHz(float minFreq, float maxFreq, float notmalizedX) {
        return minFreq * std::pow(maxFreq / minFreq, notmalizedX);
    }
    static float getFFTDataByHz(float* processedFFTData, float fftSize, float sampleRate, float hz) {
        float indexFloat = hz * ((fftSize * 0.5) / (sampleRate * 0.5));
        int index = indexFloat;
        float frac = indexFloat - index;
        return processedFFTData[index] * (1 - frac) + processedFFTData[index + 1] * frac;
    }
};

//==============================================================================
namespace {
constexpr int FREQ_SCOPE_SIZE = 512;
constexpr int TIME_SCOPE_SIZE = 1024;
constexpr int ENVELOPE_VIEW_HEIGHT = 200;
constexpr int SPECTRUM_VIEW_WIDTH = 200;
constexpr int FFT_ORDER = 12;
constexpr int FFT_SIZE = 4096;
}  // namespace
class AnalyserWindow2 : public juce::Component, juce::Button::Listener, private juce::Timer {
public:
    AnalyserWindow2(Recorder& recorder, AllParams& allParams);
    virtual ~AnalyserWindow2();
    AnalyserWindow2(const AnalyserWindow2&) = delete;

    virtual void paint(juce::Graphics& g) override;
    virtual void resized() override;

private:
    SeedLookAndFeel seedLookAndFeel;

    Recorder& recorder;
    AllParams& allParams;

    juce::dsp::FFT forwardFFT;
    juce::dsp::WindowingFunction<float> window;
    float allFftData[TIME_SCOPE_SIZE][FFT_SIZE * 2]{};
    float allScopeData[TIME_SCOPE_SIZE][FREQ_SCOPE_SIZE]{};
    bool calculated = false;
    int focusedTimeIndex = TIME_SCOPE_SIZE / 2;
    int focusedFreqIndex = FREQ_SCOPE_SIZE / 2;
    int currentEntryIndex = 0;

    std::array<juce::ToggleButton, NUM_ENTRIES> entryButtons;
    juce::ToggleButton recordingButton;
    juce::ImageComponent heatMap;
    JustRectangle envelopeLine;
    JustRectangle spectrumLine;
    juce::ImageComponent envelopeView;
    juce::ImageComponent spectrumView;

    // methods
    virtual void timerCallback() override;
    virtual void buttonClicked(juce::Button* button) override;
    virtual void mouseDown(const MouseEvent& event) override;
    virtual void mouseDoubleClick(const MouseEvent& event) override;

    void calculateSpectrum(int timeScopeIndex);
    void drawHeatMap();
    void drawEnvelopeView();
    void drawSpectrumView();
    static float xToHz(float minFreq, float maxFreq, float normalizedX) {
        return minFreq * std::pow(maxFreq / minFreq, normalizedX);
    }
    static float hzToX(float minFreq, float maxFreq, float freq) {
        return std::logf(freq / minFreq) / std::logf(maxFreq / minFreq);
    }
    static float xToHz2(float minFreq, float midFreq, float maxFreq, float normalizedX) {
        // TODO: constexpr
        auto A = (midFreq - maxFreq) / (midFreq - minFreq);
        auto B = (maxFreq - minFreq) / (std::pow(A, 2) - 1);
        return B * std::pow(-A, 2 * normalizedX) + minFreq - B;
    }
    static float hzToX2(float minFreq, float midFreq, float maxFreq, float freq) {
        // TODO: constexpr
        auto A = (midFreq - maxFreq) / (midFreq - minFreq);
        auto B = (maxFreq - minFreq) / (std::pow(A, 2) - 1);
        return (std::logf((freq + B - minFreq) / B) / std::logf(-A)) / 2;
    }
    static float getFFTDataByHz(float* processedFFTData, float fftSize, float sampleRate, float hz) {
        float indexFloat = hz * ((fftSize * 0.5) / (sampleRate * 0.5));
        int index = indexFloat;
        float frac = indexFloat - index;
        return processedFFTData[index] * (1 - frac) + processedFFTData[index + 1] * frac;
    }
};
