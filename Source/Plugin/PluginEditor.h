#pragma once

#include <array>
#include <functional>

#include <JuceHeader.h>
#include "PluginProcessor.h"

class MIDIGenGXAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
private:
    class DownwardSelector : public juce::Component
    {
    public:
        DownwardSelector(MIDIGenGXAudioProcessorEditor& owner,
                         const juce::StringArray& items);
        ~DownwardSelector() override = default;

        void paint(juce::Graphics&) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent&) override;
        void mouseUp(const juce::MouseEvent&) override;

        void setSelectedIndex(int index, bool sendCallback);
        int getSelectedIndex() const noexcept;

        void setItems(const juce::StringArray& newItems);
        std::function<void(int)> onChange;
        void setUiScale(float scale);
        void hidePopup(bool notify = false);
        bool containsPopupComponent(
            const juce::Component* component) const noexcept;

    private:
        MIDIGenGXAudioProcessorEditor& owner;
        juce::StringArray items;
        int selectedIndex = 0;
        bool open = false;

        juce::Label textLabel;
        juce::TextButton arrowButton;

        class Popup
            : public juce::Component,
              private juce::ScrollBar::Listener
        {
        public:
            Popup(DownwardSelector& selector);

            void paint(juce::Graphics&) override;
            void resized() override;
            void mouseDown(const juce::MouseEvent&) override;
            void mouseWheelMove(
                const juce::MouseEvent&,
                const juce::MouseWheelDetails&) override;

            void setContentHeight(int height);
            bool containsComponent(
                const juce::Component* component) const noexcept;

            DownwardSelector& selector;
            juce::OwnedArray<juce::TextButton> buttons;

        private:
            void scrollBarMoved(
                juce::ScrollBar*,
                double newRangeStart) override;

            juce::ScrollBar verticalScrollBar{ true };
            int contentHeight = 0;
            int scrollOffset = 0;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Popup)
        };

        std::unique_ptr<Popup> popup;

        void showPopup();
        void updateVisuals();

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DownwardSelector)
    };

private:
    class CpuWarningOverlay : public juce::Component
    {
    public:
        CpuWarningOverlay(
            MIDIGenGXAudioProcessorEditor& owner,
            const juce::String& title,
            const juce::String& message,
            std::function<void(bool)> decisionCallback);

        void paint(juce::Graphics&) override;
        void resized() override;
        bool keyPressed(const juce::KeyPress&) override;

        void scanCpuCapability();

    private:
        MIDIGenGXAudioProcessorEditor& owner;
        std::function<void(bool)> decisionCallback;
        juce::Label titleLabel;
        juce::Label messageLabel;
        juce::TextButton scanButton{ "SCAN SYSTEM" };
        juce::TextButton cancelButton{ "CANCEL" };
        juce::TextButton continueButton{ "CONTINUE" };

        juce::Label scanResultLabel;

        bool cpuScanComplete = false;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
            CpuWarningOverlay)
    };

    class InfoPopup : public juce::Component
    {
    public:
        explicit InfoPopup(float scale);

        void setMessage(
            const juce::String& message,
            float scale);

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        juce::Label label;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(InfoPopup)
    };

public:
    explicit MIDIGenGXAudioProcessorEditor(
        MIDIGenGXAudioProcessor&);

    ~MIDIGenGXAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    bool keyPressed(const juce::KeyPress&) override;
    void mouseDown(const juce::MouseEvent&) override;

private:
    void timerCallback() override;
    void populateSelectors();
    void syncControlsFromProcessor();
    void pushContextToProcessor();
    void closeSelectorPopups();
    void selectorValueChanged();
    void markGenreAsCustom();
    void showInfoPopup(int infoIndex, juce::Component& target);
    void closeInfoPopup();
    void showSettingsMenu();
    void showAIModelFileChooser();
    void updateAIControls();
    void setUiZoom(float zoom);

    static void styleLabel(
        juce::Label&,
        float fontSize,
        juce::Colour,
        juce::Justification =
            juce::Justification::centred);

    static juce::String keyName(int pitchClass);
    static juce::String scaleName(int index);
    static juce::String roleName(int index);

    MIDIGenGXAudioProcessor& audioProcessor;

    DownwardSelector genreBox;
    DownwardSelector keyBox;
    DownwardSelector scaleBox;
    DownwardSelector roleBox;
    DownwardSelector lengthBox;
    DownwardSelector octaveLowBox;
    DownwardSelector octaveHighBox;
    DownwardSelector octaveShiftBox;
    DownwardSelector noteLengthBox;
    DownwardSelector phraseContourBox;
    DownwardSelector cadenceStyleBox;

    juce::Label titleLabel;
    juce::Label phaseLabel;

    juce::Label genreLabel;
    juce::Label keyLabel;
    juce::Label scaleLabel;
    juce::Label roleLabel;
    juce::Label lengthLabel;
    juce::Label octaveLowLabel;
    juce::Label octaveHighLabel;
    juce::Label octaveShiftLabel;
    juce::Label noteLengthLabel;
    juce::Label phraseContourLabel;
    juce::Label cadenceStyleLabel;

    juce::Label densityLabel;
    juce::Label variationLabel;
    juce::Label complexityLabel;
    juce::Label syncopationLabel;
    juce::Label tensionLabel;
    juce::Label repetitionLabel;
    juce::Label humanizationLabel;
    juce::Label noteLengthVariationLabel;
    juce::Label cadenceStrengthLabel;

    juce::Label densityValueLabel;
    juce::Label variationValueLabel;
    juce::Label complexityValueLabel;
    juce::Label syncopationValueLabel;
    juce::Label tensionValueLabel;
    juce::Label repetitionValueLabel;
    juce::Label humanizationValueLabel;
    juce::Label noteLengthVariationValueLabel;
    juce::Label cadenceStrengthValueLabel;

    juce::Slider densitySlider;
    juce::Slider variationSlider;
    juce::Slider complexitySlider;
    juce::Slider syncopationSlider;
    juce::Slider tensionSlider;
    juce::Slider repetitionSlider;
    juce::Slider humanizationSlider;
    juce::Slider noteLengthVariationSlider;
    juce::Slider cadenceStrengthSlider;

    juce::Label statusLabel;

    std::array<juce::TextButton, 21> infoButtons;
    std::unique_ptr<InfoPopup> activeInfoPopup;
    std::unique_ptr<CpuWarningOverlay> activeCpuWarningOverlay;

    juce::TextButton settingsButton{
        "SETTINGS"
    };

    enum class GenerationCpuMode
    {
        low = 0,
        balanced,
        high,
        pro
    };

    void configureGenerationCpuMode();
    void showHighCpuWarning(
        GenerationCpuMode requestedMode,
        int requestedIndex,
        int previousIndex);

    juce::Label generationCpuLabel;
    DownwardSelector generationCpuModeBox;

    GenerationCpuMode generationCpuMode =
        GenerationCpuMode::low;

    int previousGenerationCpuModeIndex =
        0;

    juce::TextButton generatorButton{
        "START GENERATION"
    };

    juce::TextButton aiGenerateButton{
        "AI GENERATE"
    };

    juce::Label aiStatusLabel;

    float uiZoom = 1.0f;
    bool updatingControls = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        MIDIGenGXAudioProcessorEditor
    )
};
