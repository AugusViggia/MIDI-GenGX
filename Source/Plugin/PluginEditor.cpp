#include "PluginEditor.h"
#include "../Domain/AbletonOctaveConvention.h"

#include <cmath>
#if JUCE_WINDOWS
 #include <windows.h>
#endif


namespace
{
constexpr float kMinUiZoom = 0.75f;
constexpr float kMaxUiZoom = 1.50f;
constexpr int kPopupRowHeight = 29;

constexpr int keyFirstId = 1;
constexpr int scaleFirstId = 101;
constexpr int roleFirstId = 201;
constexpr int lengthFirstId = 301;
constexpr int octaveFirstId = 401;
}

MIDIGenGXAudioProcessorEditor::DownwardSelector::
Popup::Popup(DownwardSelector& selectorToUse)
    : selector(selectorToUse)
{
    setOpaque(true);

    addAndMakeVisible(
        verticalScrollBar);

    verticalScrollBar.addListener(
        this);

    verticalScrollBar.setAutoHide(
        true);

    verticalScrollBar.setRangeLimits(
        0.0,
        0.0);
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
Popup::paint(juce::Graphics& g)
{
    g.fillAll(
        juce::Colour(0xff26323a));

    g.setColour(
        juce::Colour(0xff4b5961));

    g.drawRect(
        getLocalBounds(),
        1);
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
Popup::resized()
{
    constexpr int rowHeight =
        kPopupRowHeight;

    const bool needsScroll =
        buttons.size() > 4;

    const int scrollbarWidth =
        needsScroll ? 12 : 0;

    verticalScrollBar.setVisible(
        needsScroll);

    const int contentWidth =
        juce::jmax(
            1,
            getWidth() -
                scrollbarWidth -
                2);

    const int visibleHeight =
        juce::jmax(
            1,
            getHeight() - 2);

    contentHeight =
        buttons.size() * rowHeight + 2;

    const int maxScroll =
        juce::jmax(
            0,
            contentHeight -
                visibleHeight);

    scrollOffset =
        juce::jlimit(
            0,
            maxScroll,
            scrollOffset);

    verticalScrollBar.setBounds(
        getWidth() - 12,
        1,
        11,
        visibleHeight);

    if (needsScroll)
    {
        verticalScrollBar.setRangeLimits(
            0.0,
            static_cast<double>(
                contentHeight));

        verticalScrollBar.setCurrentRange(
            static_cast<double>(
                scrollOffset),
            static_cast<double>(
                visibleHeight),
            juce::dontSendNotification);
    }
    else
    {
        scrollOffset = 0;

        verticalScrollBar.setRangeLimits(
            0.0,
            0.0);

        verticalScrollBar.setCurrentRange(
            0.0,
            static_cast<double>(
                visibleHeight),
            juce::dontSendNotification);
    }

    for (int i = 0; i < buttons.size(); ++i)
    {
        const int y =
            1 +
            i * rowHeight -
            scrollOffset;

        buttons[i]->setBounds(
            1,
            y,
            contentWidth,
            rowHeight);
    }
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
Popup::mouseWheelMove(
    const juce::MouseEvent&,
    const juce::MouseWheelDetails& wheel)
{
    if (contentHeight <= getHeight())
        return;

    const int maxScroll =
        juce::jmax(
            0,
            contentHeight -
                getHeight() +
                2);

    const int delta =
        juce::roundToInt(
            wheel.deltaY *
            static_cast<float>(
                kPopupRowHeight * 1.5f));

    scrollOffset =
        juce::jlimit(
            0,
            maxScroll,
            scrollOffset - delta);

    verticalScrollBar.setCurrentRangeStart(
        static_cast<double>(
            scrollOffset),
        juce::dontSendNotification);

    resized();
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
Popup::scrollBarMoved(
    juce::ScrollBar* scrollBar,
    double newRangeStart)
{
    if (scrollBar !=
        &verticalScrollBar)
        return;

    scrollOffset =
        juce::jlimit(
            0,
            juce::jmax(
                0,
                contentHeight -
                    getHeight() +
                    2),
            juce::roundToInt(
                newRangeStart));

    resized();
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
Popup::setContentHeight(
    int height)
{
    contentHeight =
        juce::jmax(
            0,
            height);

    resized();
}

bool MIDIGenGXAudioProcessorEditor::DownwardSelector::
Popup::containsComponent(
    const juce::Component* component) const noexcept
{
    if (component == nullptr)
        return false;

    return component == this ||
           isParentOf(component);
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
Popup::mouseDown(const juce::MouseEvent&)
{
    selector.hidePopup();
}

MIDIGenGXAudioProcessorEditor::DownwardSelector::
DownwardSelector(
    MIDIGenGXAudioProcessorEditor& editor,
    const juce::StringArray& initialItems)
    : owner(editor),
      items(initialItems)
{
    setOpaque(false);

    addAndMakeVisible(textLabel);
    textLabel.setInterceptsMouseClicks(false, false);
    textLabel.setJustificationType(juce::Justification::centredLeft);
    textLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::white);
    textLabel.setFont(
        juce::FontOptions(14.0f));

    addAndMakeVisible(arrowButton);
    arrowButton.setButtonText({});
    arrowButton.setColour(
        juce::TextButton::buttonColourId,
        juce::Colours::transparentBlack);
    arrowButton.setColour(
        juce::TextButton::textColourOffId,
        juce::Colours::transparentBlack);
    arrowButton.onClick = [this]()
    {
        if (open)
            hidePopup();
        else
            showPopup();
    };

    updateVisuals();
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
setItems(const juce::StringArray& newItems)
{
    items = newItems;
    selectedIndex =
        juce::jlimit(
            0,
            juce::jmax(0, items.size() - 1),
            selectedIndex);
    updateVisuals();
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
setUiScale(float scale)
{
    const float clampedScale =
        juce::jlimit(kMinUiZoom, kMaxUiZoom, scale);

    textLabel.setFont(
        juce::FontOptions(
            14.0f * clampedScale,
            juce::Font::plain));

    repaint();
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
setSelectedIndex(
    int index,
    bool sendCallback)
{
    if (items.isEmpty())
        return;

    const int newIndex =
        juce::jlimit(
            0,
            items.size() - 1,
            index);

    const bool changed =
        newIndex != selectedIndex;

    selectedIndex = newIndex;
    updateVisuals();

    if (open)
        hidePopup();

    if (changed && sendCallback)
    {
        if (onChange)
            onChange(selectedIndex);
        else
            owner.selectorValueChanged();
    }
}

int MIDIGenGXAudioProcessorEditor::DownwardSelector::
getSelectedIndex() const noexcept
{
    return selectedIndex;
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
updateVisuals()
{
    textLabel.setText(
        items.isEmpty()
            ? juce::String()
            : items[selectedIndex],
        juce::dontSendNotification);

    repaint();
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
paint(juce::Graphics& g)
{
    g.setColour(juce::Colour(0xff26323a));
    g.fillRoundedRectangle(
        getLocalBounds().toFloat(),
        5.0f);

    g.setColour(juce::Colour(0xff5b6870));
    g.drawRoundedRectangle(
        getLocalBounds().toFloat().reduced(0.5f),
        5.0f,
        1.0f);

    // Encoding-safe down chevron. Do not use a Unicode glyph here because
    // the host/editor font path previously rendered it as a broken "a".
    const float cx = static_cast<float>(getWidth() - 19);
    const float cy = static_cast<float>(getHeight() / 2);

    juce::Path chevron;
    chevron.startNewSubPath(cx - 5.0f, cy - 2.5f);
    chevron.lineTo(cx,        cy + 2.5f);
    chevron.lineTo(cx + 5.0f, cy - 2.5f);

    g.setColour(juce::Colours::white);
    g.strokePath(
        chevron,
        juce::PathStrokeType(
            1.8f,
            juce::PathStrokeType::curved,
            juce::PathStrokeType::rounded));
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
resized()
{
    arrowButton.setBounds(
        getWidth() - 38,
        1,
        36,
        getHeight() - 2);

    textLabel.setBounds(
        14,
        1,
        getWidth() - 54,
        getHeight() - 2);
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
mouseDown(const juce::MouseEvent&)
{
    showPopup();
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
mouseUp(const juce::MouseEvent&)
{
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
showPopup()
{
    if (open || items.isEmpty())
        return;

    owner.closeInfoPopup();
    owner.closeSelectorPopups();
    open = true;

    popup =
        std::make_unique<Popup>(*this);

    popup->setAlwaysOnTop(true);

    constexpr int rowHeight = kPopupRowHeight;
    constexpr int maxVisibleRows = 4;

    const int contentHeight =
        items.size() * rowHeight + 2;

    const int maxPopupHeight =
        maxVisibleRows * rowHeight + 2;

    const int popupHeight =
        juce::jmin(
            contentHeight,
            maxPopupHeight);

    // IMPORTANT:
    // This is a CHILD of the plugin editor. It is placed directly below
    // the selector in editor-local coordinates, so there is no screen-space
    // popup auto-flip as with juce::PopupMenu.
    const auto bounds =
        getBoundsInParent();

    const auto parentBounds =
        owner.getLocalBounds();

    const int popupX =
        juce::jlimit(
            0,
            juce::jmax(
                0,
                parentBounds.getWidth() - getWidth()),
            bounds.getX());

    const int popupY =
        bounds.getBottom();

    const int availableHeight =
        parentBounds.getBottom() - popupY;

    const int finalHeight =
        juce::jmin(
            popupHeight,
            juce::jmax(
                kPopupRowHeight + 2,
                availableHeight));

    popup->setBounds(
        popupX,
        popupY,
        getWidth(),
        finalHeight);

    popup->buttons.clear();

    for (int i = 0; i < items.size(); ++i)
    {
        auto* button =
            popup->buttons.add(
                new juce::TextButton(
                    items[i]));

        button->setClickingTogglesState(false);

        button->setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xff26323a));

        button->setColour(
            juce::TextButton::textColourOffId,
            juce::Colours::white);

        button->onClick = [this, i]()
        {
            setSelectedIndex(i, true);
        };

        popup->addAndMakeVisible(button);
    }

    owner.addAndMakeVisible(
        popup.get());

    popup->toFront(true);
    popup->setContentHeight(
        contentHeight);
    popup->resized();
    popup->repaint();
}

void MIDIGenGXAudioProcessorEditor::DownwardSelector::
hidePopup(bool notify)
{
    juce::ignoreUnused(notify);

    if (!open)
        return;

    open = false;
    popup.reset();
}

MIDIGenGXAudioProcessorEditor::
MIDIGenGXAudioProcessorEditor(
    MIDIGenGXAudioProcessor& processor)
    : AudioProcessorEditor(&processor),
      audioProcessor(processor),
      genreBox(*this, {}),
      keyBox(*this, {}),
      scaleBox(*this, {}),
      roleBox(*this, {}),
      lengthBox(*this, {}),
      octaveLowBox(*this, {}),
      octaveHighBox(*this, {}),
      octaveShiftBox(*this, {}),
      noteLengthBox(*this, {}),
      phraseContourBox(*this, {}),
      cadenceStyleBox(*this, {}),
      generationCpuModeBox(*this, {
          "Low (2 cores)",
          "Balanced (4 cores)",
          "High (6 cores)",
          "Pro (8 cores)"
      })
{
    constexpr int baseWidth = 820;
    constexpr int baseHeight = 860;

    setSize(baseWidth, baseHeight);
    setResizable(false, false);
    startTimerHz(10);

    juce::Desktop::getInstance().addGlobalMouseListener(
        this);

    titleLabel.setText(
        "MIDI-GenGX",
        juce::dontSendNotification);

    phaseLabel.setText(
        "PHRASE ENGINE  |  MUSICAL CONTEXT + REGISTER",
        juce::dontSendNotification);

    styleLabel(
        titleLabel,
        28.0f,
        juce::Colours::white);

    styleLabel(
        phaseLabel,
        12.0f,
        juce::Colour(0xff9a9a9a));

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(phaseLabel);

    const juce::String selectorNames[] =
    {
        "GENRE",
        "KEY",
        "SCALE",
        "ROLE",
        "LENGTH",
        "OCTAVE LOW",
        "OCTAVE HIGH",
        "OCTAVE SHIFT",
        "NOTE LENGTH",
        "PHRASE CONTOUR",
        "CADENCE"
    };

    juce::Label* selectorLabels[] =
    {
        &genreLabel,
        &keyLabel,
        &scaleLabel,
        &roleLabel,
        &lengthLabel,
        &octaveLowLabel,
        &octaveHighLabel,
        &octaveShiftLabel,
        &noteLengthLabel,
        &phraseContourLabel,
        &cadenceStyleLabel
    };

    for (int i = 0; i < 11; ++i)
    {
        selectorLabels[i]->setText(
            selectorNames[i],
            juce::dontSendNotification);

        styleLabel(
            *selectorLabels[i],
            10.0f,
            juce::Colour(0xff8d8d8d),
            juce::Justification::centredLeft);

        addAndMakeVisible(*selectorLabels[i]);
    }

    populateSelectors();

    addAndMakeVisible(genreBox);
    addAndMakeVisible(keyBox);
    addAndMakeVisible(scaleBox);
    addAndMakeVisible(roleBox);
    addAndMakeVisible(lengthBox);
    addAndMakeVisible(octaveLowBox);
    addAndMakeVisible(octaveHighBox);
    addAndMakeVisible(octaveShiftBox);
    addAndMakeVisible(noteLengthBox);
    addAndMakeVisible(phraseContourBox);
    addAndMakeVisible(cadenceStyleBox);

    // ---------- Musical Character ----------

    const char* characterNames[] =
    {
        "DENSITY",
        "VARIATION",
        "COMPLEXITY",
        "SYNCOPATION",
        "TENSION",
        "REPETITION",
        "HUMANIZE",
        "LENGTH VARIATION",
        "CADENCE STRENGTH"
    };

    juce::Label* characterLabels[] =
    {
        &densityLabel,
        &variationLabel,
        &complexityLabel,
        &syncopationLabel,
        &tensionLabel,
        &repetitionLabel,
        &humanizationLabel,
        &noteLengthVariationLabel,
        &cadenceStrengthLabel
    };

    for (int i = 0; i < 9; ++i)
    {
        characterLabels[i]->setText(
            characterNames[i],
            juce::dontSendNotification);

        styleLabel(
            *characterLabels[i],
            10.0f,
            juce::Colour(0xff8d8d8d),
            juce::Justification::centredLeft);

        addAndMakeVisible(
            *characterLabels[i]);
    }

    juce::Slider* characterSliders[] =
    {
        &densitySlider,
        &variationSlider,
        &complexitySlider,
        &syncopationSlider,
        &tensionSlider,
        &repetitionSlider,
        &humanizationSlider,
        &noteLengthVariationSlider,
        &cadenceStrengthSlider
    };

    juce::Label* characterValues[] =
    {
        &densityValueLabel,
        &variationValueLabel,
        &complexityValueLabel,
        &syncopationValueLabel,
        &tensionValueLabel,
        &repetitionValueLabel,
        &humanizationValueLabel,
        &noteLengthVariationValueLabel,
        &cadenceStrengthValueLabel
    };

    for (int i = 0; i < 9; ++i)
    {
        characterSliders[i]->setRange(
            0.0,
            100.0,
            1.0);

        characterSliders[i]->setSliderStyle(
            juce::Slider::LinearHorizontal);

        characterSliders[i]->setTextBoxStyle(
            juce::Slider::NoTextBox,
            false,
            0,
            0);

        addAndMakeVisible(
            *characterSliders[i]);

        characterValues[i]->setJustificationType(
            juce::Justification::centred);

        characterValues[i]->setColour(
            juce::Label::textColourId,
            juce::Colours::white);

        addAndMakeVisible(
            *characterValues[i]);
    }

    const char* const helpTooltips[] =
    {
        "Genre", "Key", "Scale", "Role", "Length",
        "Octave Low", "Octave High", "Octave Shift",
        "Note Length", "Phrase Contour", "Cadence",
        "Density", "Variation", "Complexity", "Syncopation",
        "Tension", "Repetition", "Humanize",
        "Length Variation", "Cadence Strength",
        "CPU Mode"
    };

    for (int i = 0; i < static_cast<int>(infoButtons.size()); ++i)
    {
        auto& button =
            infoButtons[static_cast<std::size_t>(i)];

        button.setButtonText("?");
        button.setTooltip(helpTooltips[i]);
        button.setClickingTogglesState(false);

        button.setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xff26323a));

        button.setColour(
            juce::TextButton::buttonOnColourId,
            juce::Colour(0xff34444e));

        button.setColour(
            juce::TextButton::textColourOffId,
            juce::Colour(0xffb9c0c4));

        button.setColour(
            juce::TextButton::textColourOnId,
            juce::Colours::white);

        addAndMakeVisible(button);

        button.onClick =
            [this, i]()
            {
                showInfoPopup(
                    i,
                    infoButtons[static_cast<std::size_t>(i)]);
            };
    }

    genreBox.onChange = [this](int genreIndex)
    {
        if (updatingControls)
            return;

        const int genre =
            juce::jlimit(
                0,
                static_cast<int>(
                    midigengx::domain::GenrePreset::Count) - 1,
                genreIndex);

        audioProcessor.applyGenrePreset(genre);

        // The processor owns the preset state. Re-read all dependent controls
        // so the UI becomes an immediate visual representation of the preset.
        syncControlsFromProcessor();
    };

    densitySlider.onValueChange = [this]()
    {
        if (!updatingControls)
        {
            closeInfoPopup();
            markGenreAsCustom();
            const int value =
                static_cast<int>(
                    std::lround(
                        densitySlider.getValue()));

            densityValueLabel.setText(
                juce::String(value) + "%",
                juce::dontSendNotification);

            audioProcessor.setDensity(value);
        }
    };

    variationSlider.onValueChange = [this]()
    {
        if (!updatingControls)
        {
            closeInfoPopup();
            markGenreAsCustom();
            const int value =
                static_cast<int>(
                    std::lround(
                        variationSlider.getValue()));

            variationValueLabel.setText(
                juce::String(value) + "%",
                juce::dontSendNotification);

            audioProcessor.setVariation(value);
        }
    };

    complexitySlider.onValueChange = [this]()
    {
        if (!updatingControls)
        {
            closeInfoPopup();
            markGenreAsCustom();
            const int value =
                static_cast<int>(
                    std::lround(
                        complexitySlider.getValue()));

            complexityValueLabel.setText(
                juce::String(value) + "%",
                juce::dontSendNotification);

            audioProcessor.setComplexity(value);
        }
    };

    syncopationSlider.onValueChange = [this]()
    {
        if (!updatingControls)
        {
            closeInfoPopup();
            markGenreAsCustom();
            const int value =
                static_cast<int>(
                    std::lround(
                        syncopationSlider.getValue()));

            syncopationValueLabel.setText(
                juce::String(value) + "%",
                juce::dontSendNotification);

            audioProcessor.setSyncopation(value);
        }
    };

    tensionSlider.onValueChange = [this]()
    {
        if (!updatingControls)
        {
            closeInfoPopup();
            markGenreAsCustom();
            const int value =
                static_cast<int>(
                    std::lround(
                        tensionSlider.getValue()));

            tensionValueLabel.setText(
                juce::String(value) + "%",
                juce::dontSendNotification);

            audioProcessor.setTension(value);
        }
    };

    repetitionSlider.onValueChange = [this]()
    {
        if (!updatingControls)
        {
            closeInfoPopup();
            markGenreAsCustom();
            const int value =
                static_cast<int>(
                    std::lround(
                        repetitionSlider.getValue()));

            repetitionValueLabel.setText(
                juce::String(value) + "%",
                juce::dontSendNotification);

            audioProcessor.setRepetition(value);
        }
    };

    humanizationSlider.onValueChange = [this]()
    {
        if (!updatingControls)
        {
            closeInfoPopup();
            markGenreAsCustom();
            const int value =
                static_cast<int>(
                    std::lround(
                        humanizationSlider.getValue()));

            humanizationValueLabel.setText(
                juce::String(value) + "%",
                juce::dontSendNotification);

            audioProcessor.setHumanization(value);
        }
    };

    noteLengthVariationSlider.onValueChange = [this]()
    {
        if (!updatingControls)
        {
            closeInfoPopup();
            markGenreAsCustom();
            const int value =
                static_cast<int>(
                    std::lround(
                        noteLengthVariationSlider.getValue()));

            noteLengthVariationValueLabel.setText(
                juce::String(value) + "%",
                juce::dontSendNotification);

            audioProcessor.setNoteLengthVariation(value);
        }
    };

    generationCpuLabel.setText(
        "CPU MODE",
        juce::dontSendNotification);

    styleLabel(
        generationCpuLabel,
        10.0f,
        juce::Colour(0xff8d8d8d),
        juce::Justification::centredLeft);

    addAndMakeVisible(
        generationCpuLabel);
addAndMakeVisible(
        generationCpuModeBox);

    generatorButton.onClick = [this]()
    {
        closeInfoPopup();
        pushContextToProcessor();

        if (!audioProcessor.isGeneratorEnabled())
        {
            audioProcessor.setGeneratorEnabled(true);
        }
        else
        {
            audioProcessor.setGeneratorEnabled(false);
        }

        syncControlsFromProcessor();
    };

    addAndMakeVisible(settingsButton);
    settingsButton.setClickingTogglesState(false);
    settingsButton.setColour(
        juce::TextButton::buttonColourId,
        juce::Colour(0xff26323a));
    settingsButton.setColour(
        juce::TextButton::textColourOffId,
        juce::Colours::white);
    settingsButton.onClick = [this]()
    {
        closeInfoPopup();
        showSettingsMenu();
    };

    addAndMakeVisible(generatorButton);
addAndMakeVisible(aiGenerateButton);
    aiGenerateButton.setClickingTogglesState(false);
    aiGenerateButton.setColour(
        juce::TextButton::buttonColourId,
        juce::Colour(0xff34444e));
    aiGenerateButton.setColour(
        juce::TextButton::textColourOffId,
        juce::Colours::white);
    aiGenerateButton.onClick =
        [this]()
        {
            closeInfoPopup();

            if (!audioProcessor.hasLoadedAIRuntimeModel())
            {
                aiStatusLabel.setText(
                    "NO AI MODEL",
                    juce::dontSendNotification);
                return;
            }

            audioProcessor.setAIRuntimeEnabled(true);
            audioProcessor.setGeneratorEnabled(true);
            audioProcessor.requestAIRuntimeGeneration();

            aiStatusLabel.setText(
                "AI GENERATING",
                juce::dontSendNotification);
        };

    aiStatusLabel.setJustificationType(
        juce::Justification::centred);

    aiStatusLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xffaeb7bc));

    addAndMakeVisible(aiStatusLabel);

    configureGenerationCpuMode();

    statusLabel.setJustificationType(
        juce::Justification::centred);

    statusLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::lightgrey);

    addAndMakeVisible(statusLabel);

    syncControlsFromProcessor();
}


bool MIDIGenGXAudioProcessorEditor::DownwardSelector::
containsPopupComponent(
    const juce::Component* component) const noexcept
{
    return popup != nullptr &&
           popup->containsComponent(
               component);
}

MIDIGenGXAudioProcessorEditor::
~MIDIGenGXAudioProcessorEditor()
{
    stopTimer();

    juce::Desktop::getInstance().removeGlobalMouseListener(
        this);

    closeSelectorPopups();
    closeInfoPopup();

    activeCpuWarningOverlay.reset();
}

void MIDIGenGXAudioProcessorEditor::closeSelectorPopups()
{
    genreBox.hidePopup();
    keyBox.hidePopup();
    scaleBox.hidePopup();
    roleBox.hidePopup();
    lengthBox.hidePopup();
    octaveLowBox.hidePopup();
    octaveHighBox.hidePopup();
    octaveShiftBox.hidePopup();
    noteLengthBox.hidePopup();
    phraseContourBox.hidePopup();
    cadenceStyleBox.hidePopup();
}

void MIDIGenGXAudioProcessorEditor::selectorValueChanged()
{
    if (!updatingControls)
    {
        closeInfoPopup();
        pushContextToProcessor();
    }
}

void MIDIGenGXAudioProcessorEditor::markGenreAsCustom()
{
    if (updatingControls)
        return;

    if (audioProcessor.getGenrePreset() !=
        static_cast<int>(
            midigengx::domain::GenrePreset::Custom))
    {
        audioProcessor.setGenrePreset(
            static_cast<int>(
                midigengx::domain::GenrePreset::Custom));

        genreBox.setSelectedIndex(
            0,
            false);
    }
}

void MIDIGenGXAudioProcessorEditor::styleLabel(
    juce::Label& label,
    float fontSize,
    juce::Colour colour,
    juce::Justification justification)
{
    label.setJustificationType(justification);
    label.setColour(
        juce::Label::textColourId,
        colour);
    label.setFont(
        juce::FontOptions(fontSize));
}

juce::String MIDIGenGXAudioProcessorEditor::keyName(
    int pitchClass)
{
    static constexpr const char* names[] =
    {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };

    return names[
        juce::jlimit(
            0,
            11,
            pitchClass)];
}

juce::String MIDIGenGXAudioProcessorEditor::scaleName(
    int index)
{
    switch (index)
    {
        case 0: return "Major";
        case 1: return "Minor";
        case 2: return "Harmonic Minor";
        case 3: return "Melodic Minor";
        case 4: return "Dorian";
        case 5: return "Phrygian";
        case 6: return "Lydian";
        case 7: return "Mixolydian";
        case 8: return "Locrian";
        case 9: return "Pentatonic";
        case 10: return "Blues";
        case 11: return "Chromatic";
        case 12: return "Arabic";
        case 13: return "Rumanian";
        case 14: return "Hindu";
        case 15: return "Spanish";
        case 16: return "Hungarian";
        default: return "Custom";
    }
}

juce::String MIDIGenGXAudioProcessorEditor::roleName(
    int index)
{
    switch (index)
    {
        case 0: return "Lead";
        case 1: return "Bass";
        case 2: return "ARP";
        case 3: return "Melody";
        case 4: return "Chords";
        case 5: return "Pad";
        case 6: return "Pluck";
        case 7: return "Piano";
        default: return "Sequence";
    }
}



#if JUCE_WINDOWS
static int getDetectedLogicalProcessorCount() noexcept
{
    const auto count =
        GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);

    return static_cast<int>(
        juce::jmax(
            DWORD(1),
            count));
}
#endif

MIDIGenGXAudioProcessorEditor::CpuWarningOverlay::
CpuWarningOverlay(
    MIDIGenGXAudioProcessorEditor& editor,
    const juce::String& title,
    const juce::String& message,
    std::function<void(bool)> callback)
    : owner(editor),
      decisionCallback(std::move(callback))
{
    setOpaque(false);
    setWantsKeyboardFocus(true);

    titleLabel.setText(
        title,
        juce::dontSendNotification);
    titleLabel.setFont(
        juce::FontOptions(
            18.0f,
            juce::Font::bold));
    titleLabel.setColour(
        juce::Label::textColourId,
        juce::Colours::white);
    titleLabel.setJustificationType(
        juce::Justification::centredLeft);
    titleLabel.setInterceptsMouseClicks(
        false,
        false);

    messageLabel.setText(
        message,
        juce::dontSendNotification);
    messageLabel.setFont(
        juce::FontOptions(
            13.0f,
            juce::Font::plain));
    messageLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xffe4e8ea));
    messageLabel.setJustificationType(
        juce::Justification::centredLeft);
    messageLabel.setInterceptsMouseClicks(
        false,
        false);

    scanResultLabel.setColour(
        juce::Label::textColourId,
        juce::Colour(0xffaeb7bc));
    scanResultLabel.setFont(
        juce::FontOptions(11.0f));
    scanResultLabel.setJustificationType(
        juce::Justification::centredLeft);
    scanResultLabel.setText(
        "System capability not scanned.",
        juce::dontSendNotification);
    scanResultLabel.setInterceptsMouseClicks(
        false,
        false);

    addAndMakeVisible(
        scanResultLabel);

    for (auto* button : {
             &scanButton,
             &cancelButton,
             &continueButton })
    {
        button->setClickingTogglesState(false);
        button->setColour(
            juce::TextButton::buttonColourId,
            juce::Colour(0xff26323a));
        button->setColour(
            juce::TextButton::textColourOffId,
            juce::Colours::white);
        button->setColour(
            juce::TextButton::buttonOnColourId,
            juce::Colour(0xff34444e));

        addAndMakeVisible(*button);
    }

    addAndMakeVisible(titleLabel);
    addAndMakeVisible(messageLabel);

    cancelButton.onClick =
        [this]()
        {
            if (decisionCallback)
                decisionCallback(false);
        };

    continueButton.onClick =
        [this]()
        {
            if (decisionCallback)
                decisionCallback(true);
        };

    scanButton.onClick =
        [this]()
        {
            scanCpuCapability();
        };

    addKeyListener(nullptr);
    grabKeyboardFocus();
}

void MIDIGenGXAudioProcessorEditor::CpuWarningOverlay::
scanCpuCapability()
{
#if JUCE_WINDOWS
    const int logicalProcessors =
        getDetectedLogicalProcessorCount();

    const auto recommended =
        logicalProcessors >= 8
            ? "Pro"
            : logicalProcessors >= 6
                ? "High"
                : logicalProcessors >= 4
                    ? "Balanced"
                    : "Low";

    scanResultLabel.setText(
        "Detected " +
            juce::String(logicalProcessors) +
            " logical CPU threads. Recommended mode: " +
            recommended +
            ".",
        juce::dontSendNotification);

    cpuScanComplete = true;
#else
    scanResultLabel.setText(
        "CPU capability scan is available on Windows only.",
        juce::dontSendNotification);

    cpuScanComplete = true;
#endif

    scanButton.setButtonText(
        cpuScanComplete
            ? "RESCAN SYSTEM"
            : "SCAN SYSTEM");
}

void MIDIGenGXAudioProcessorEditor::CpuWarningOverlay::
paint(juce::Graphics& g)
{
    g.fillAll(
        juce::Colour(0x99000000));

    auto dialogBounds =
        getLocalBounds().reduced(32, 140);

    const int dialogWidth =
        juce::jmin(
            560,
            dialogBounds.getWidth());

    const int dialogHeight =
        juce::jmin(
            255,
            dialogBounds.getHeight());

    dialogBounds.setSize(
        dialogWidth,
        dialogHeight);
    dialogBounds.setCentre(
        getLocalBounds().getCentre());

    g.setColour(
        juce::Colour(0xff26323a));
    g.fillRoundedRectangle(
        dialogBounds.toFloat(),
        8.0f);

    g.setColour(
        juce::Colour(0xff59666d));
    g.drawRoundedRectangle(
        dialogBounds.toFloat().reduced(0.5f),
        8.0f,
        1.0f);

    // Compact warning marker.
    const auto marker =
        juce::Rectangle<float>(
            static_cast<float>(dialogBounds.getX() + 20),
            static_cast<float>(dialogBounds.getY() + 23),
            8.0f,
            42.0f);

    g.setColour(
        juce::Colour(0xffd06a3a));
    g.fillRoundedRectangle(
        marker,
        2.0f);

    g.fillEllipse(
        marker.getCentreX() - 3.0f,
        marker.getBottom() + 8.0f,
        6.0f,
        6.0f);
}

void MIDIGenGXAudioProcessorEditor::CpuWarningOverlay::
resized()
{
    auto dialogBounds =
        getLocalBounds().reduced(32, 140);

    const int dialogWidth =
        juce::jmin(
            560,
            dialogBounds.getWidth());

    const int dialogHeight =
        juce::jmin(
            255,
            dialogBounds.getHeight());

    dialogBounds.setSize(
        dialogWidth,
        dialogHeight);
    dialogBounds.setCentre(
        getLocalBounds().getCentre());

    constexpr int padding = 20;

    titleLabel.setBounds(
        dialogBounds.getX() + padding + 20,
        dialogBounds.getY() + 18,
        dialogBounds.getWidth() - 2 * padding - 20,
        28);

    messageLabel.setBounds(
        dialogBounds.getX() + padding + 20,
        dialogBounds.getY() + 56,
        dialogBounds.getWidth() - 2 * padding - 20,
        82);

    scanResultLabel.setBounds(
        dialogBounds.getX() + padding + 20,
        dialogBounds.getY() + 140,
        dialogBounds.getWidth() - 2 * padding - 20,
        22);

    const int buttonWidth = 112;
    const int buttonHeight = 34;
    const int gap = 8;

    scanButton.setBounds(
        dialogBounds.getX() + padding + 20,
        dialogBounds.getBottom() - padding - buttonHeight,
        buttonWidth,
        buttonHeight);

    continueButton.setBounds(
        dialogBounds.getRight() - padding - buttonWidth,
        dialogBounds.getBottom() - padding - buttonHeight,
        buttonWidth,
        buttonHeight);

    cancelButton.setBounds(
        dialogBounds.getRight() -
            padding -
            buttonWidth -
            gap -
            buttonWidth,
        dialogBounds.getBottom() - padding - buttonHeight,
        buttonWidth,
        buttonHeight);
}

bool MIDIGenGXAudioProcessorEditor::CpuWarningOverlay::
keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        if (decisionCallback)
            decisionCallback(false);

        return true;
    }

    if (key == juce::KeyPress::returnKey)
    {
        if (decisionCallback)
            decisionCallback(true);

        return true;
    }

    return false;
}

MIDIGenGXAudioProcessorEditor::InfoPopup::
InfoPopup(float scale)
{
    setInterceptsMouseClicks(false, false);

    label.setColour(
        juce::Label::textColourId,
        juce::Colours::white);

    label.setColour(
        juce::Label::backgroundColourId,
        juce::Colours::transparentBlack);

    label.setJustificationType(
        juce::Justification::centredLeft);

    addAndMakeVisible(label);

    setMessage(
        {},
        scale);
}

void MIDIGenGXAudioProcessorEditor::InfoPopup::
setMessage(
    const juce::String& message,
    float scale)
{
    const float safeScale =
        juce::jlimit(
            kMinUiZoom,
            kMaxUiZoom,
            scale);

    label.setText(
        message,
        juce::dontSendNotification);

    label.setFont(
        juce::FontOptions(
            11.0f * safeScale,
            juce::Font::plain));

    const int width =
        juce::jmax(
            168,
            juce::jmin(
                250,
                juce::roundToInt(
                    224.0f * safeScale)));

    const int height =
        juce::jmax(
            54,
            juce::roundToInt(
                64.0f * safeScale));

    setSize(
        width,
        height);
}

void MIDIGenGXAudioProcessorEditor::InfoPopup::
paint(
    juce::Graphics& g)
{
    g.setColour(
        juce::Colour(0xff20272b));

    g.fillRoundedRectangle(
        getLocalBounds().toFloat(),
        7.0f);

    g.setColour(
        juce::Colour(0xff59666d));

    g.drawRoundedRectangle(
        getLocalBounds().toFloat().reduced(0.5f),
        7.0f,
        1.0f);
}

void MIDIGenGXAudioProcessorEditor::InfoPopup::
resized()
{
    label.setBounds(
        10,
        7,
        getWidth() - 20,
        getHeight() - 14);
}


void MIDIGenGXAudioProcessorEditor::configureGenerationCpuMode()
{
    generationCpuMode =
        GenerationCpuMode::low;

    previousGenerationCpuModeIndex =
        0;

    generationCpuModeBox.setSelectedIndex(
        0,
        false);

    generationCpuModeBox.onChange =
        [this](int selectedIndex)
        {
            const int previousIndex =
                previousGenerationCpuModeIndex;

            const auto requestedMode =
                static_cast<GenerationCpuMode>(
                    juce::jlimit(
                        0,
                        3,
                        selectedIndex));

            const bool requiresWarning =
                requestedMode ==
                    GenerationCpuMode::high ||
                requestedMode ==
                    GenerationCpuMode::pro;

            if (!requiresWarning)
            {
                generationCpuMode =
                    requestedMode;

                previousGenerationCpuModeIndex =
                    selectedIndex;

                return;
            }

            generationCpuModeBox.setSelectedIndex(
                previousIndex,
                false);

            showHighCpuWarning(
                requestedMode,
                selectedIndex,
                previousIndex);
        };
}

void MIDIGenGXAudioProcessorEditor::showHighCpuWarning(
    GenerationCpuMode requestedMode,
    int requestedIndex,
    int previousIndex)
{
    const bool isPro =
        requestedMode ==
        GenerationCpuMode::pro;

    const juce::String title =
        isPro
            ? "PRO CPU MODE"
            : "HIGH CPU MODE";

    const juce::String body =
        isPro
            ? "MIDI-GenGX may use up to 8 CPU cores during generation.\n\n"
              "This mode is intended for high-performance systems. "
              "High CPU usage may cause audio dropouts, increased latency, "
              "freezes, or reduced performance in Ableton Live or other "
              "applications if your system cannot sustain the workload.\n\n"
              "Continue with Pro Mode?"
            : "MIDI-GenGX may use up to 6 CPU cores during generation.\n\n"
              "This can significantly increase CPU usage and may affect "
              "Ableton Live, virtual instruments, or other plugins on systems "
              "with limited processing capacity.\n\n"
              "Continue with High Mode?";

    closeInfoPopup();
    closeSelectorPopups();

    juce::Component::SafePointer<
        MIDIGenGXAudioProcessorEditor> safeThis(this);

    activeCpuWarningOverlay =
        std::make_unique<CpuWarningOverlay>(
            *this,
            title,
            body,
            [safeThis,
             requestedMode,
             requestedIndex,
             previousIndex](
                bool confirmed)
            {
                if (safeThis == nullptr)
                    return;

                auto& editor =
                    *safeThis;

                if (confirmed)
                {
                    editor.generationCpuMode =
                        requestedMode;

                    editor.previousGenerationCpuModeIndex =
                        requestedIndex;

                    editor.generationCpuModeBox.setSelectedIndex(
                        requestedIndex,
                        false);
                }
                else
                {
                    editor.generationCpuMode =
                        static_cast<GenerationCpuMode>(
                            juce::jlimit(
                                0,
                                3,
                                previousIndex));

                    editor.previousGenerationCpuModeIndex =
                        previousIndex;

                    editor.generationCpuModeBox.setSelectedIndex(
                        previousIndex,
                        false);
                }

                editor.activeCpuWarningOverlay.reset();
                editor.repaint();
            });

    addAndMakeVisible(
        *activeCpuWarningOverlay);

    activeCpuWarningOverlay->toFront(true);
    activeCpuWarningOverlay->setBounds(
        getLocalBounds());
    activeCpuWarningOverlay->resized();
    activeCpuWarningOverlay->grabKeyboardFocus();
    activeCpuWarningOverlay->repaint();
}

void MIDIGenGXAudioProcessorEditor::updateAIControls()
{
    const bool modelLoaded =
        audioProcessor.hasLoadedAIRuntimeModel();

    const bool aiEnabled =
        audioProcessor.isAIRuntimeEnabled();

    aiGenerateButton.setEnabled(
        modelLoaded);

    if (!modelLoaded)
    {
        aiStatusLabel.setText(
            "AI MODEL NOT EMBEDDED",
            juce::dontSendNotification);
    }
    else if (aiEnabled)
    {
        aiStatusLabel.setText(
            "AI READY",
            juce::dontSendNotification);
    }
    else
    {
        aiStatusLabel.setText(
            "AI MODEL READY",
            juce::dontSendNotification);
    }
}

void MIDIGenGXAudioProcessorEditor::showInfoPopup(
    int infoIndex,
    juce::Component& target)
{
    static const char* const descriptions[] =
    {
        "Starting musical profile for the generator.",
        "Tonal root note used by the phrase.",
        "Pitch collection used to keep notes in key.",
        "Musical function the generated phrase should serve.",
        "Total phrase length in bars.",
        "Lowest register the generator may use.",
        "Highest register the generator may use.",
        "Relative register shift inside the selected range.",
        "Default duration behaviour for generated notes.",
        "Overall register movement of the phrase.",
        "Strength of phrase-ending resolution.",
        "Amount of MIDI events generated.",
        "Amount of local musical change.",
        "Complexity of pitch and rhythmic decisions.",
        "Amount of off-beat rhythmic movement.",
        "Harmonic and register pressure.",
        "Strength of motif repetition.",
        "Subtle timing and velocity variation.",
        "Variation applied to note durations.",
        "Strength of phrase-final resolution.",
        "CPU processing budget used during generation. Low uses 2 cores, "
        "Balanced uses 4, High uses 6, and Pro uses 8. Higher modes can "
        "increase CPU usage and may affect system performance."
    };

    constexpr int count =
        static_cast<int>(
            sizeof(descriptions) /
            sizeof(descriptions[0]));

    if (infoIndex < 0 || infoIndex >= count)
        return;

    closeSelectorPopups();
    closeInfoPopup();

    const float scale =
        juce::jlimit(
            0.75f,
            1.50f,
            uiZoom);

    activeInfoPopup =
        std::make_unique<InfoPopup>(scale);

    activeInfoPopup->setMessage(
        juce::String(descriptions[infoIndex]),
        scale);

    addAndMakeVisible(
        *activeInfoPopup);

    // All popup coordinates remain in editor-local space. This is the key
    // difference from the previous CallOutBox implementation: there is no
    // screen-space/local-space conversion and no post-construction
    // setBounds() on a CallOutBox.
    const auto anchor =
        target.getBoundsInParent();

    const auto visible =
        getLocalBounds().reduced(
            juce::jmax(
                4,
                juce::roundToInt(
                    4.0f * scale)));

    const int popupWidth =
        activeInfoPopup->getWidth();

    const int popupHeight =
        activeInfoPopup->getHeight();

    const int gap =
        juce::jmax(
            5,
            juce::roundToInt(
                6.0f * scale));

    // Primary placement: immediately below the ? button.
    int x =
        anchor.getX() +
        (anchor.getWidth() -
         popupWidth) / 2;

    int y =
        anchor.getBottom() +
        gap;

    // If the lower area is too small, flip above the anchor.
    if (y + popupHeight >
        visible.getBottom())
    {
        y =
            anchor.getY() -
            popupHeight -
            gap;
    }

    // Final hard containment. The popup can never leave the editor bounds.
    x = juce::jlimit(
        visible.getX(),
        juce::jmax(
            visible.getX(),
            visible.getRight() -
                popupWidth),
        x);

    y = juce::jlimit(
        visible.getY(),
        juce::jmax(
            visible.getY(),
            visible.getBottom() -
                popupHeight),
        y);

    activeInfoPopup->setBounds(
        x,
        y,
        popupWidth,
        popupHeight);

    activeInfoPopup->toFront(false);
}

void MIDIGenGXAudioProcessorEditor::closeInfoPopup()
{
    activeInfoPopup.reset();
}


void MIDIGenGXAudioProcessorEditor::populateSelectors()
{
    juce::StringArray genres;
    genres.add("Custom");
    genres.add("House");
    genres.add("Deep House");
    genres.add("Organic House");
    genres.add("Progressive House");
    genres.add("Techno");
    genres.add("Trance");
    genres.add("Drum & Bass");
    genres.add("Dubstep");
    genres.add("Hip-Hop");
    genres.add("Trap");

    genreBox.setItems(genres);

    juce::StringArray keys;
    for (int i = 0; i < 12; ++i)
        keys.add(keyName(i));

    juce::StringArray scales;
    const int concreteScaleCount =
        static_cast<int>(
            midigengx::domain::ScaleType::Count);

    for (int i = 0;
         i < concreteScaleCount;
         ++i)
    {
        scales.add(scaleName(i));
    }

    juce::StringArray roles;
    for (int i = 0; i < 9; ++i)
        roles.add(roleName(i));

    juce::StringArray lengths;
    for (int value : {4, 8, 16, 32, 64})
        lengths.add(juce::String(value) + " bars");

    juce::StringArray octaves;

    for (int internalRegister =
             midigengx::domain::AbletonOctaveConvention::
                 minInternalRegister;
         internalRegister <=
             midigengx::domain::AbletonOctaveConvention::
                 maxInternalRegister;
         ++internalRegister)
    {
        const int abletonOctave =
            midigengx::domain::AbletonOctaveConvention::
                internalToAbletonOctave(
                    internalRegister);

        octaves.add(
            "C" +
            juce::String(
                abletonOctave));
    }

    juce::StringArray shifts;
    for (int shift = -2; shift <= 2; ++shift)
    {
        if (shift > 0)
            shifts.add("+" + juce::String(shift));
        else
            shifts.add(juce::String(shift));
    }

    keyBox.setItems(keys);
    scaleBox.setItems(scales);
    roleBox.setItems(roles);
    lengthBox.setItems(lengths);
    octaveLowBox.setItems(octaves);
    octaveHighBox.setItems(octaves);
    octaveShiftBox.setItems(shifts);

    juce::StringArray noteLengths;
    noteLengths.add("Auto");
    noteLengths.add("Short");
    noteLengths.add("Medium");
    noteLengths.add("Long");
    noteLengths.add("Legato");
    noteLengths.add("Staccato");

    noteLengthBox.setItems(noteLengths);

    juce::StringArray contours;
    contours.add("Arch");
    contours.add("Ascending");
    contours.add("Descending");
    contours.add("Flat");
    contours.add("Valley");

    phraseContourBox.setItems(contours);

    juce::StringArray cadences;
    cadences.add("Root");
    cadences.add("Fifth");
    cadences.add("Third");
    cadences.add("Open");

    cadenceStyleBox.setItems(cadences);
}

void MIDIGenGXAudioProcessorEditor::syncControlsFromProcessor()
{
    updateAIControls();

    updatingControls = true;

    genreBox.setSelectedIndex(
        juce::jlimit(
            0,
            static_cast<int>(
                midigengx::domain::GenrePreset::Count) - 1,
            audioProcessor.getGenrePreset()),
        false);

    keyBox.setSelectedIndex(
        audioProcessor.getKey(),
        false);

    scaleBox.setSelectedIndex(
        audioProcessor.getScale(),
        false);

    roleBox.setSelectedIndex(
        audioProcessor.getRole(),
        false);

    const int lengths[] = {4, 8, 16, 32, 64};

    int lengthIndex = 0;
    for (int i = 0; i < 5; ++i)
    {
        if (lengths[i] == audioProcessor.getLengthBars())
        {
            lengthIndex = i;
            break;
        }
    }

    lengthBox.setSelectedIndex(
        lengthIndex,
        false);

    const auto octaveIndexForInternalRegister =
        [](int internalRegister) noexcept
        {
            return juce::jlimit(
                0,
                8,
                internalRegister -
                    midigengx::domain::AbletonOctaveConvention::
                        minInternalRegister);
        };

    octaveLowBox.setSelectedIndex(
        octaveIndexForInternalRegister(
            audioProcessor.getOctaveLow()),
        false);

    octaveHighBox.setSelectedIndex(
        octaveIndexForInternalRegister(
            audioProcessor.getOctaveHigh()),
        false);

    octaveShiftBox.setSelectedIndex(
        juce::jlimit(
            0,
            4,
            audioProcessor.getOctaveShift() + 2),
        false);

    noteLengthBox.setSelectedIndex(
        juce::jlimit(
            0,
            5,
            audioProcessor.getNoteLength()),
        false);

    phraseContourBox.setSelectedIndex(
        juce::jlimit(
            0,
            4,
            audioProcessor.getPhraseContour()),
        false);

    cadenceStyleBox.setSelectedIndex(
        juce::jlimit(
            0,
            3,
            audioProcessor.getCadenceStyle()),
        false);

    densitySlider.setValue(
        audioProcessor.getDensity(),
        juce::dontSendNotification);

    variationSlider.setValue(
        audioProcessor.getVariation(),
        juce::dontSendNotification);

    densityValueLabel.setText(
        juce::String(audioProcessor.getDensity()) + "%",
        juce::dontSendNotification);

    variationValueLabel.setText(
        juce::String(audioProcessor.getVariation()) + "%",
        juce::dontSendNotification);

    complexitySlider.setValue(
        audioProcessor.getComplexity(),
        juce::dontSendNotification);

    syncopationSlider.setValue(
        audioProcessor.getSyncopation(),
        juce::dontSendNotification);

    tensionSlider.setValue(
        audioProcessor.getTension(),
        juce::dontSendNotification);

    repetitionSlider.setValue(
        audioProcessor.getRepetition(),
        juce::dontSendNotification);

    complexityValueLabel.setText(
        juce::String(audioProcessor.getComplexity()) + "%",
        juce::dontSendNotification);

    syncopationValueLabel.setText(
        juce::String(audioProcessor.getSyncopation()) + "%",
        juce::dontSendNotification);

    tensionValueLabel.setText(
        juce::String(audioProcessor.getTension()) + "%",
        juce::dontSendNotification);

    repetitionValueLabel.setText(
        juce::String(audioProcessor.getRepetition()) + "%",
        juce::dontSendNotification);

    humanizationSlider.setValue(
        audioProcessor.getHumanization(),
        juce::dontSendNotification);

    noteLengthVariationSlider.setValue(
        audioProcessor.getNoteLengthVariation(),
        juce::dontSendNotification);

    humanizationValueLabel.setText(
        juce::String(audioProcessor.getHumanization()) + "%",
        juce::dontSendNotification);

    noteLengthVariationValueLabel.setText(
        juce::String(audioProcessor.getNoteLengthVariation()) + "%",
        juce::dontSendNotification);

    cadenceStrengthSlider.setValue(
        audioProcessor.getCadenceStrength(),
        juce::dontSendNotification);

    cadenceStrengthValueLabel.setText(
        juce::String(audioProcessor.getCadenceStrength()) + "%",
        juce::dontSendNotification);

    generatorButton.setButtonText(
        audioProcessor.isGeneratorEnabled()
            ? "STOP GENERATION"
            : "START GENERATION");

    const juce::String pending =
        audioProcessor.isGenerationPending()
            ? "Generation pending"
            : "Phrase ready";

    const juce::String transport =
        audioProcessor.isTransportPlaying()
            ? "PLAYING"
            : "PAUSED / STOPPED";

    statusLabel.setText(
        pending +
        "   |   Transport: " +
        transport,
        juce::dontSendNotification);

    updatingControls = false;
}

void MIDIGenGXAudioProcessorEditor::pushContextToProcessor()
{
    const int key = keyBox.getSelectedIndex();
    const int scale = scaleBox.getSelectedIndex();
    const int role = roleBox.getSelectedIndex();

    const int lengths[] = {4, 8, 16, 32, 64};

    const int lengthIndex =
        juce::jlimit(
            0,
            4,
            lengthBox.getSelectedIndex());

    const auto internalRegisterFromSelector =
        [](int selectorIndex) noexcept
        {
            return juce::jlimit(
                midigengx::domain::AbletonOctaveConvention::
                    minInternalRegister,
                midigengx::domain::AbletonOctaveConvention::
                    maxInternalRegister,
                selectorIndex +
                    midigengx::domain::AbletonOctaveConvention::
                        minInternalRegister);
        };

    const int lowOctave =
        internalRegisterFromSelector(
            octaveLowBox.getSelectedIndex());

    const int highOctave =
        internalRegisterFromSelector(
            octaveHighBox.getSelectedIndex());

    const int octaveRegisterShift =
        juce::jlimit(
            -2,
            2,
            octaveShiftBox.getSelectedIndex() - 2);

    const int noteLength =
        juce::jlimit(
            0,
            5,
            noteLengthBox.getSelectedIndex());

    const int phraseContour =
        juce::jlimit(
            0,
            4,
            phraseContourBox.getSelectedIndex());

    const int cadenceStyle =
        juce::jlimit(
            0,
            3,
            cadenceStyleBox.getSelectedIndex());

    audioProcessor.setKey(key);
    audioProcessor.setScale(scale);
    audioProcessor.setRole(role);
    audioProcessor.setLengthBars(lengths[lengthIndex]);

    audioProcessor.setOctaveLow(
        juce::jmin(lowOctave, highOctave));

    audioProcessor.setOctaveHigh(
        juce::jmax(lowOctave, highOctave));

    audioProcessor.setOctaveShift(
        octaveRegisterShift);

    audioProcessor.setNoteLength(
        noteLength);

    audioProcessor.setPhraseContour(
        phraseContour);

    audioProcessor.setCadenceStyle(
        cadenceStyle);

    audioProcessor.setDensity(
        static_cast<int>(
            std::lround(
                densitySlider.getValue())));

    audioProcessor.setVariation(
        static_cast<int>(
            std::lround(
                variationSlider.getValue())));

    audioProcessor.setHumanization(
        static_cast<int>(
            std::lround(
                humanizationSlider.getValue())));

    audioProcessor.setNoteLengthVariation(
        static_cast<int>(
            std::lround(
                noteLengthVariationSlider.getValue())));

    audioProcessor.setCadenceStrength(
        static_cast<int>(
            std::lround(
                cadenceStrengthSlider.getValue())));
}

void MIDIGenGXAudioProcessorEditor::timerCallback()
{
    syncControlsFromProcessor();
}

void MIDIGenGXAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    constexpr int baseWidth = 820;
    constexpr int baseHeight = 820;

    const float scale =
        juce::jlimit(
            0.50f,
            2.00f,
            juce::jmin(
                static_cast<float>(getWidth()) /
                    static_cast<float>(baseWidth),
                static_cast<float>(getHeight()) /
                    static_cast<float>(baseHeight)));

    const auto S = [scale](float value)
    {
        return juce::roundToInt(value * scale);
    };

    g.fillAll(juce::Colour(0xff151515));

    const juce::Rectangle<int> selectorPanel(
        S(32), S(105),
        getWidth() - S(64), S(315));

    const juce::Rectangle<int> dynamicsPanel(
        S(32), S(435),
        getWidth() - S(64), S(205));

    for (const auto panel : { selectorPanel, dynamicsPanel })
    {
        g.setColour(juce::Colour(0xff1d1d1d));
        g.fillRoundedRectangle(
            panel.toFloat(),
            static_cast<float>(S(10)));

        g.setColour(juce::Colour(0xff2a2a2a));
        g.drawRoundedRectangle(
            panel.toFloat(),
            static_cast<float>(S(10)),
            static_cast<float>(S(1)));
    }

    g.setColour(juce::Colours::white);
    g.setFont(
        juce::FontOptions(
            28.0f * scale,
            juce::Font::plain));

    g.drawText(
        "MIDI-GenGX",
        juce::Rectangle<int>(
            S(40), S(20),
            getWidth() - S(80), S(42)),
        juce::Justification::centred,
        false);

    g.setColour(juce::Colour(0xff909090));
    g.setFont(
        juce::FontOptions(
            12.0f * scale,
            juce::Font::plain));

    g.drawText(
        "PHRASE ENGINE  |  MUSICAL CONTEXT + REGISTER",
        juce::Rectangle<int>(
            S(40), S(63),
            getWidth() - S(80), S(24)),
        juce::Justification::centred,
        false);
}


void MIDIGenGXAudioProcessorEditor::showSettingsMenu()
{
    closeSelectorPopups();

    juce::PopupMenu menu;

    constexpr float scales[] =
    {
        0.75f, 0.85f, 1.00f, 1.15f, 1.30f, 1.50f
    };

    const char* labels[] =
    {
        "75%", "85%", "100%", "115%", "130%", "150%"
    };

    for (int i = 0; i < 6; ++i)
    {
        menu.addItem(
            i + 1,
            labels[i],
            true,
            std::abs(uiZoom - scales[i]) < 0.001f);
    }

    menu.showMenuAsync(
        juce::PopupMenu::Options()
            .withTargetScreenArea(
                settingsButton.getScreenBounds())
            .withMinimumWidth(120)
            .withMaximumNumColumns(1),
        [this](int result)
        {
            if (result < 1 || result > 6)
                return;

            constexpr float selectedScales[] =
            {
                0.75f, 0.85f, 1.00f, 1.15f, 1.30f, 1.50f
            };

            setUiZoom(
                selectedScales[result - 1]);
        });
}

void MIDIGenGXAudioProcessorEditor::setUiZoom(float zoom)
{
    const float clamped =
        juce::jlimit(kMinUiZoom, kMaxUiZoom, zoom);

    if (std::abs(uiZoom - clamped) < 0.001f)
        return;

    uiZoom = clamped;
    closeSelectorPopups();

    constexpr int baseWidth = 820;
    constexpr int baseHeight = 820;

    setSize(
        juce::roundToInt(
            static_cast<float>(baseWidth) * uiZoom),
        juce::roundToInt(
            static_cast<float>(baseHeight) * uiZoom));
}

void MIDIGenGXAudioProcessorEditor::resized()
{
    constexpr int baseWidth = 820;
    constexpr int baseHeight = 820;

    const float scale =
        juce::jlimit(
            0.50f,
            2.00f,
            juce::jmin(
                static_cast<float>(getWidth()) /
                    static_cast<float>(baseWidth),
                static_cast<float>(getHeight()) /
                    static_cast<float>(baseHeight)));

    const auto S = [scale](float value)
    {
        return juce::roundToInt(value * scale);
    };

    settingsButton.setBounds(
        getWidth() - S(148),
        S(20),
        S(108),
        S(34));

    constexpr int left = 56;
    constexpr int right = 56;
    constexpr int gap = 12;
    constexpr int selectorH = 38;

    const int contentWidth = getWidth();
    const int rowWidth =
        (contentWidth -
         S(left + right + gap * 3)) / 4;

    juce::Component* row1Boxes[] =
    {
        &genreBox, &keyBox, &scaleBox, &roleBox
    };

    juce::Label* row1Labels[] =
    {
        &genreLabel, &keyLabel, &scaleLabel, &roleLabel
    };

    juce::Component* row2Boxes[] =
    {
        &lengthBox, &octaveLowBox,
        &octaveHighBox, &octaveShiftBox
    };

    juce::Label* row2Labels[] =
    {
        &lengthLabel, &octaveLowLabel,
        &octaveHighLabel, &octaveShiftLabel
    };

    for (int i = 0; i < 4; ++i)
    {
        const int x =
            S(left) + i * (rowWidth + S(gap));

        row1Labels[i]->setBounds(
            x, S(128),
            rowWidth - S(24), S(16));

        row1Boxes[i]->setBounds(
            x, S(147),
            rowWidth, S(selectorH));

        row2Labels[i]->setBounds(
            x, S(224),
            rowWidth - S(24), S(16));

        row2Boxes[i]->setBounds(
            x, S(243),
            rowWidth, S(selectorH));
    }

    const int row3Width =
        (contentWidth -
         S(left + right + gap * 2)) / 3;

    juce::Component* row3Boxes[] =
    {
        &noteLengthBox,
        &phraseContourBox,
        &cadenceStyleBox
    };

    juce::Label* row3Labels[] =
    {
        &noteLengthLabel,
        &phraseContourLabel,
        &cadenceStyleLabel
    };

    for (int i = 0; i < 3; ++i)
    {
        const int x =
            S(left) + i * (row3Width + S(gap));

        row3Labels[i]->setBounds(
            x, S(320),
            row3Width - S(24), S(16));

        row3Boxes[i]->setBounds(
            x, S(339),
            row3Width, S(selectorH));
    }

    for (auto* selector : {
             &genreBox, &keyBox, &scaleBox, &roleBox,
             &lengthBox, &octaveLowBox, &octaveHighBox,
             &octaveShiftBox, &noteLengthBox,
             &phraseContourBox, &cadenceStyleBox })
    {
        selector->setUiScale(scale);
    }

    const int sliderLeft = S(58);
    const int sliderRight = getWidth() - S(58);
    const int sliderGap = S(18);

    const int charColumnWidth =
        (sliderRight -
         sliderLeft -
         sliderGap * 2) / 3;

    juce::Label* characterLabels[] =
    {
        &densityLabel, &variationLabel, &complexityLabel,
        &syncopationLabel, &tensionLabel, &repetitionLabel,
        &humanizationLabel, &noteLengthVariationLabel,
        &cadenceStrengthLabel
    };

    juce::Label* characterValues[] =
    {
        &densityValueLabel, &variationValueLabel,
        &complexityValueLabel, &syncopationValueLabel,
        &tensionValueLabel, &repetitionValueLabel,
        &humanizationValueLabel, &noteLengthVariationValueLabel,
        &cadenceStrengthValueLabel
    };

    juce::Slider* characterSliders[] =
    {
        &densitySlider, &variationSlider, &complexitySlider,
        &syncopationSlider, &tensionSlider, &repetitionSlider,
        &humanizationSlider, &noteLengthVariationSlider,
        &cadenceStrengthSlider
    };

    for (int i = 0; i < 9; ++i)
    {
        const int row = i / 3;
        const int col = i % 3;

        const int x =
            sliderLeft +
            col * (charColumnWidth + sliderGap);

        const int y =
            S(static_cast<float>(457 + row * 54));

        characterLabels[i]->setBounds(
            x, y,
            charColumnWidth - S(22),
            S(16));

        characterValues[i]->setBounds(
            x, y + S(19),
            S(48), S(18));

        characterSliders[i]->setBounds(
            x + S(54), y + S(19),
            charColumnWidth - S(54),
            S(18));

        characterLabels[i]->setFont(
            juce::FontOptions(
                10.0f * scale,
                juce::Font::plain));

        characterValues[i]->setFont(
            juce::FontOptions(
                12.0f * scale,
                juce::Font::plain));
    }

    for (auto* label : {
             &genreLabel, &keyLabel, &scaleLabel, &roleLabel,
             &lengthLabel, &octaveLowLabel, &octaveHighLabel,
             &octaveShiftLabel, &noteLengthLabel,
             &phraseContourLabel, &cadenceStyleLabel })
    {
        label->setFont(
            juce::FontOptions(
                10.0f * scale,
                juce::Font::plain));
    }

    juce::Label* selectorHelpLabels[] =
    {
        &genreLabel, &keyLabel, &scaleLabel, &roleLabel,
        &lengthLabel, &octaveLowLabel, &octaveHighLabel,
        &octaveShiftLabel, &noteLengthLabel,
        &phraseContourLabel, &cadenceStyleLabel
    };

    for (int i = 0; i < 11; ++i)
    {
        const auto b =
            selectorHelpLabels[i]->getBounds();

        infoButtons[static_cast<std::size_t>(i)].setBounds(
            b.getRight() + S(3),
            b.getY() - S(1),
            S(18), S(18));
    }

    for (int i = 0; i < 9; ++i)
    {
        const auto b =
            characterLabels[i]->getBounds();

        infoButtons[static_cast<std::size_t>(11 + i)].setBounds(
            b.getRight() + S(3),
            b.getY() - S(1),
            S(18), S(18));
    }

    titleLabel.setFont(
        juce::FontOptions(
            28.0f * scale,
            juce::Font::plain));

    phaseLabel.setFont(
        juce::FontOptions(
            12.0f * scale,
            juce::Font::plain));

    // Generation actions.
    const int actionY = S(670);
    const int actionH = S(42);
    const int sideButtonWidth = S(150);
    const int cpuControlWidth =
        noteLengthBox.getWidth();
    const int cpuControlX =
        noteLengthBox.getX();
    const int mainButtonWidth = S(230);

    generatorButton.setBounds(
        getWidth() / 2 - mainButtonWidth / 2,
        actionY,
        mainButtonWidth,
        actionH);

    generationCpuModeBox.setBounds(
        cpuControlX,
        actionY,
        cpuControlWidth,
        actionH);

    generationCpuLabel.setBounds(
        cpuControlX,
        actionY - S(18),
        cpuControlWidth - S(24),
        S(16));

    {
        const auto b =
            generationCpuLabel.getBounds();

        infoButtons[20].setBounds(
            b.getRight() + S(3),
            b.getY() - S(1),
            S(18), S(18));
    }
aiGenerateButton.setBounds(
        getWidth() - S(56) - sideButtonWidth,
        actionY,
        sideButtonWidth,
        actionH);

    aiStatusLabel.setBounds(
        S(56),
        actionY + actionH + S(6),
        getWidth() - S(112),
        S(18));

    aiStatusLabel.setFont(
        juce::FontOptions(
            11.0f * scale,
            juce::Font::plain));

    statusLabel.setBounds(
        S(60),
        S(736),
        getWidth() - S(120),
        S(18));

    statusLabel.setFont(
        juce::FontOptions(
            12.0f * scale,
            juce::Font::plain));
    if (activeCpuWarningOverlay)
    {
        activeCpuWarningOverlay->setBounds(
            getLocalBounds());
        activeCpuWarningOverlay->resized();
    }
}

void MIDIGenGXAudioProcessorEditor::mouseDown(
    const juce::MouseEvent& event)
{
    const auto* eventComponent =
        event.eventComponent;

    // Do not dismiss a popup when the click belongs to:
    // - a selector itself (it manages its own open/close state),
    // - one of the ? help buttons,
    // - a selector dropdown/scrollbar,
    // - the in-editor CPU warning overlay.
    if (dynamic_cast<const DownwardSelector*>(
            eventComponent) != nullptr)
        return;

    for (const auto& button : infoButtons)
    {
        if (&button == eventComponent ||
            button.isParentOf(eventComponent))
            return;
    }

    const DownwardSelector* selectors[] =
    {
        &genreBox,
        &keyBox,
        &scaleBox,
        &roleBox,
        &lengthBox,
        &octaveLowBox,
        &octaveHighBox,
        &octaveShiftBox,
        &noteLengthBox,
        &phraseContourBox,
        &cadenceStyleBox,
        &generationCpuModeBox
    };

    for (const auto* selector : selectors)
    {
        if (selector == eventComponent ||
            selector->isParentOf(eventComponent) ||
            selector->containsPopupComponent(
                eventComponent))
            return;
    }

    if (activeInfoPopup != nullptr &&
        activeInfoPopup->isParentOf(
            eventComponent))
        return;

    if (activeCpuWarningOverlay != nullptr &&
        activeCpuWarningOverlay->isParentOf(
            eventComponent))
        return;

    closeSelectorPopups();
    closeInfoPopup();
}

bool MIDIGenGXAudioProcessorEditor::keyPressed(
    const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        closeSelectorPopups();
        return true;
    }

    return false;
}
