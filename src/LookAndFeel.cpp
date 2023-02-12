#include "LookAndFeel.h"

#include "StyleConstants.h"

using namespace styles;

SeedLookAndFeel::SeedLookAndFeel(bool controlled) : controlled(controlled) {
    setColour(juce::ComboBox::textColourId, colour::TEXT);
    setColour(juce::ComboBox::arrowColourId, colour::TEXT);
    setColour(juce::ComboBox::outlineColourId, colour::BORDER);
    setColour(juce::PopupMenu::textColourId, colour::TEXT);
    setColour(juce::PopupMenu::highlightedTextColourId, colour::POPUP_SELECTED_TEXT);
    setColour(juce::PopupMenu::backgroundColourId, colour::POPUP_BACKGROUND);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, colour::POPUP_SELECTED_BACKGROUND);
    setColour(juce::BubbleComponent::outlineColourId, colour::BORDER);
    setColour(juce::BubbleComponent::backgroundColourId, colour::PIT);
    setColour(juce::TextButton::textColourOnId, colour::TEXT);
    setColour(juce::TextButton::textColourOffId, colour::TEXT);  // ?
}
SeedLookAndFeel::~SeedLookAndFeel() {}
void SeedLookAndFeel::drawTickBox(juce::Graphics& g,
                                   juce::Component& component,
                                   float x,
                                   float y,
                                   float w,
                                   float h,
                                   const bool ticked,
                                   const bool isEnabled,
                                   const bool shouldDrawButtonAsHighlighted,
                                   const bool shouldDrawButtonAsDown) {
    juce::ignoreUnused(isEnabled, shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    juce::Rectangle<float> bounds = component.getLocalBounds().toFloat();
    auto reduced = bounds.reduced(0.5);

    g.setColour(colour::PIT.withAlpha(isEnabled ? 1.0f : 0.3f));
    g.fillRect(bounds);

    g.setColour(colour::BORDER.withBrightness(isEnabled ? 0.1 : 0.14));
    g.drawRect(reduced, 1.0f);
    if (ticked) {
        g.setColour(colour::SELECT.withAlpha(isEnabled ? 1.0f : 0.3f));
        g.fillRect(bounds.reduced(1.5f));
    }
}
void SeedLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                        int x,
                                        int y,
                                        int width,
                                        int height,
                                        float sliderPos,
                                        const float rotaryStartAngle,
                                        const float rotaryEndAngle,
                                        juce::Slider& slider) {
    // 数値のドラッグ
    if (height < 24) {
        return;
    }
    auto knobWidth = (float)juce::jmin(width, height);
    auto fromCentre = dynamic_cast<CentredSlider*>(&slider) != nullptr;
    auto radius = knobWidth / 2 - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    auto fromAngle = fromCentre ? (rotaryStartAngle + rotaryEndAngle) / 2 : rotaryStartAngle;
    {
        juce::Path p;
        p.addArc(rx, ry, rw, rw, rotaryStartAngle, rotaryEndAngle, true);
        juce::PathStrokeType strokeType(SLIT_WIDTH);
        g.setColour(colour::PIT.withAlpha(slider.isEnabled() ? 1.0f : 0.3f));
        g.strokePath(p, strokeType);
    }
    {
        juce::Path p;
        p.addArc(rx, ry, rw, rw, fromAngle, angle, true);
        juce::PathStrokeType strokeType(SLIT_WIDTH);
        auto color = controlled ? colour::CONTROLLED : colour::SELECT;
        g.setColour(color.withAlpha(slider.isEnabled() ? 1.0f : 0.3f));
        //        g.setColour (colour::SELECT.withAlpha(slider.isEnabled() ? 1.0f : 0.3f));
        g.strokePath(p, strokeType);
    }
    {
        juce::Path p;
        p.addEllipse(-POINTER_RADIUS / 2, -radius * 0.8, POINTER_RADIUS, POINTER_RADIUS);
        p.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
        auto color = controlled ? colour::CONTROLLED : colour::SELECT;
        g.setColour(color.withAlpha(slider.isEnabled() ? 1.0f : 0.3f));
        g.fillPath(p);
    }
}
void SeedLookAndFeel::drawComboBox(juce::Graphics& g,
                                    int width,
                                    int height,
                                    bool isButtonDown,
                                    int buttonX,
                                    int buttonY,
                                    int buttonW,
                                    int buttonH,
                                    juce::ComboBox& box) {
    {
        juce::Rectangle<float> boxBounds(0, 0, width, height);
        auto reduced = boxBounds.reduced(BORDER_WIDTH / 2);
        auto outlineColour = box.findColour(juce::ComboBox::outlineColourId);
        {
            juce::Path p;
            auto bottom = reduced.getBottom() - 0.5;
            p.addLineSegment(juce::Line<float>(reduced.getX() - 0.5, bottom, reduced.getRight() + 0.5, bottom), 0);
            g.setColour(colour::UNDERLINE.withAlpha(box.isEnabled() ? 1.0f : 0.3f));
            g.strokePath(p, juce::PathStrokeType(1));
        }
    }
    {
        auto arrowZone = juce::Rectangle<float>(width - ARROW_ZONE_WIDTH, 0, ARROW_ZONE_WIDTH, height);
        juce::Path p;
        p.addTriangle(arrowZone.getCentreX() - 4,
                      arrowZone.getCentreY() - 2,
                      arrowZone.getCentreX() + 4,
                      arrowZone.getCentreY() - 2,
                      arrowZone.getCentreX(),
                      arrowZone.getCentreY() + 2);
        g.setColour(box.findColour(juce::ComboBox::arrowColourId).withAlpha(box.isEnabled() ? 1.0f : 0.3f));
        g.fillPath(p);
    }
}
void SeedLookAndFeel::positionComboBoxText(juce::ComboBox& box, juce::Label& label) {
    label.setBounds(1, 1, box.getWidth() - ARROW_ZONE_WIDTH, box.getHeight() - 2);
    label.setFont(getComboBoxFont(box));
}
void SeedLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.fillAll(label.findColour(juce::Label::backgroundColourId));

    if (!label.isBeingEdited()) {
        auto alpha = label.isEnabled() ? 1.0f : 0.5f;
        const juce::Font font(getLabelFont(label));

        g.setColour(label.findColour(juce::Label::textColourId).withMultipliedAlpha(alpha));
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

        // MIDI CC の番号のみを表示するために...
        auto text = label.getText();
        auto idx = text.indexOfChar(':');
        if (idx >= 0) {
            text = text.substring(0, idx);
        }
        g.drawFittedText(text,
                         textArea,
                         label.getJustificationType(),
                         juce::jmax(1, (int)((float)textArea.getHeight() / font.getHeight())),
                         label.getMinimumHorizontalScale());

        g.setColour(label.findColour(juce::Label::outlineColourId).withMultipliedAlpha(alpha));
    } else if (label.isEnabled()) {
        g.setColour(label.findColour(juce::Label::outlineColourId));
    }

    g.drawRect(label.getLocalBounds());
}
void SeedLookAndFeel::drawPopupMenuBackground(juce::Graphics& g, int width, int height) {
    auto background = findColour(juce::PopupMenu::backgroundColourId);
    g.fillAll(background);
    g.setColour(background.overlaidWith(juce::Colour(0x2badd8e6)));
}
void SeedLookAndFeel::drawPopupMenuItem(juce::Graphics& g,
                                         const juce::Rectangle<int>& area,
                                         const bool isSeparator,
                                         const bool isActive,
                                         const bool isHighlighted,
                                         const bool isTicked,
                                         const bool hasSubMenu,
                                         const juce::String& text,
                                         const juce::String& shortcutKeyText,
                                         const juce::Drawable* icon,
                                         const juce::Colour* const textColourToUse) {
    if (isSeparator) {
        auto r = area.reduced(5, 0);
        r.removeFromTop(juce::roundToInt(((float)r.getHeight() * 0.5f) - 0.5f));

        g.setColour(findColour(juce::PopupMenu::textColourId).withAlpha(0.3f));
        g.fillRect(r.removeFromTop(1));
    } else {
        auto textColour = (textColourToUse == nullptr ? findColour(juce::PopupMenu::textColourId) : *textColourToUse);

        auto r = area.reduced(1);

        if (isHighlighted && isActive) {
            g.setColour(findColour(juce::PopupMenu::highlightedBackgroundColourId));
            g.fillRect(r);

            g.setColour(findColour(juce::PopupMenu::highlightedTextColourId));
        } else {
            g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
        }

        r.reduce(juce::jmin(5, area.getWidth() / 20), 0);

        auto font = getPopupMenuFont();

        auto maxFontHeight = (float)r.getHeight() / 1.3f;

        if (font.getHeight() > maxFontHeight) font.setHeight(maxFontHeight);

        g.setFont(font);

        auto iconArea = r.removeFromLeft(juce::roundToInt(maxFontHeight)).toFloat();

        if (icon != nullptr) {
            icon->drawWithin(
                g, iconArea, juce::RectanglePlacement::centred | juce::RectanglePlacement::onlyReduceInSize, 1.0f);
            r.removeFromLeft(juce::roundToInt(maxFontHeight * 0.5f));
        } else if (isTicked) {
            auto tick = getTickShape(1.0f);
            g.fillPath(tick,
                       tick.getTransformToScaleToFit(iconArea.reduced(iconArea.getWidth() / 3, 0).toFloat(), true));
        }

        if (hasSubMenu) {
            auto arrowH = 0.6f * getPopupMenuFont().getAscent();

            auto x = static_cast<float>(r.removeFromRight((int)arrowH).getX());
            auto halfH = static_cast<float>(r.getCentreY());

            juce::Path path;
            path.startNewSubPath(x, halfH - arrowH * 0.5f);
            path.lineTo(x + arrowH * 0.6f, halfH);
            path.lineTo(x, halfH + arrowH * 0.5f);

            g.strokePath(path, juce::PathStrokeType(2.0f));
        }

        r.removeFromRight(3);
        g.drawFittedText(text, r, juce::Justification::centredLeft, 1);

        if (shortcutKeyText.isNotEmpty()) {
            auto f2 = font;
            f2.setHeight(f2.getHeight() * 0.75f);
            f2.setHorizontalScale(0.95f);
            g.setFont(f2);

            g.drawText(shortcutKeyText, r, juce::Justification::centredRight, true);
        }
    }
}

juce::Path SeedLookAndFeel::getTickShape(float height) {
    auto w = height / 4;
    auto h = height / 4;
    auto x = height / 2 - w / 2;
    auto y = height / 2 - h / 2;

    juce::Path path;
    path.addEllipse(x, y, w, h);

    return path;
}
void SeedLookAndFeel::drawButtonBackground(Graphics& g,
                                            Button& button,
                                            const Colour& backgroundColour,
                                            bool shouldDrawButtonAsHighlighted,
                                            bool shouldDrawButtonAsDown) {
    {
        auto bounds = button.getLocalBounds();
        auto width = bounds.getWidth();
        auto height = bounds.getHeight();
        juce::Rectangle<float> boxBounds(0, 0, width, height);
        auto reduced = boxBounds.reduced(BORDER_WIDTH / 2);
        auto outlineColour = button.findColour(juce::ComboBox::outlineColourId);
        {
            juce::Path p;
            auto bottom = reduced.getBottom() - 0.5;
            p.addLineSegment(juce::Line<float>(reduced.getX() - 0.5, bottom, reduced.getRight() + 0.5, bottom), 0);
            g.setColour(colour::UNDERLINE.withAlpha(button.isEnabled() ? 1.0f : 0.3f));
            g.strokePath(p, juce::PathStrokeType(1));
        }
    }
    // auto cornerSize = 6.0f;
    // auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
    // auto baseColour = backgroundColour.withMultipliedSaturation(button.hasKeyboardFocus(true) ? 1.3f : 0.9f)
    //                       .withMultipliedAlpha(button.isEnabled() ? 1.0f : 0.5f);

    // if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
    //     baseColour = baseColour.contrasting(shouldDrawButtonAsDown ? 0.2f : 0.05f);

    // g.setColour(baseColour);

    // auto flatOnLeft = button.isConnectedOnLeft();
    // auto flatOnRight = button.isConnectedOnRight();
    // auto flatOnTop = button.isConnectedOnTop();
    // auto flatOnBottom = button.isConnectedOnBottom();

    // if (flatOnLeft || flatOnRight || flatOnTop || flatOnBottom) {
    //     Path path;
    //     path.addRoundedRectangle(bounds.getX(),
    //                              bounds.getY(),
    //                              bounds.getWidth(),
    //                              bounds.getHeight(),
    //                              cornerSize,
    //                              cornerSize,
    //                              !(flatOnLeft || flatOnTop),
    //                              !(flatOnRight || flatOnTop),
    //                              !(flatOnLeft || flatOnBottom),
    //                              !(flatOnRight || flatOnBottom));

    //     g.fillPath(path);

    //     g.setColour(button.findColour(ComboBox::outlineColourId));
    //     g.strokePath(path, PathStrokeType(1.0f));
    // } else {
    //     g.fillRoundedRectangle(bounds, cornerSize);

    //     g.setColour(button.findColour(ComboBox::outlineColourId));
    //     g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    // }
}