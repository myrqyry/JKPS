#include "../Headers/StyleWizard.hpp"
#include "../Headers/ConfigHelper.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/Utility.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>


StyleWizard::StyleWizard(const FontHolder &fontHolder, const TextureHolder &textureHolder)
: mCurrentStep(Step::Welcome)
, mFontHolder(&fontHolder)
, mTextureHolder(&textureHolder)
, mSelectedPreset(-1)
, mMousePressed(false)
, mStatusTimer(0.f)
, mColorCustomizationDirty(false)
, mAnimationSettingsDirty(false)
, mDraggingSpeed(false)
, mSelectedColorChannel(-1)
, mDraggingSlider(-1)
, mDraggingAnimSlider(-1)
, mSelectedAnimColor(-1)
, mSelectedButtonShape(0)
, mSelectedAnimStyle(0)
, mApplyRequested(false)
{
    mPresets = {
        {
            "Modern",
            "Clean modern look with blue tones",
            sf::Color(52, 152, 219, 255),
            sf::Color(255, 255, 255, 255),
            sf::Color(30, 30, 30, 255),
            sf::Color(120, 180, 255, 255),
            true, true, 1.0f,
            100, sf::Color(255, 255, 255, 255),
            3,
            false, 40, 0, 500, sf::Color(255, 255, 255, 255), 100, 0,
            0, 0,
            sf::Color(0, 0, 0, 255), 0, 20
        },
        {
            "Dark",
            "Dark theme with high contrast",
            sf::Color(60, 60, 60, 255),
            sf::Color(200, 200, 200, 255),
            sf::Color(20, 20, 20, 255),
            sf::Color(100, 100, 100, 255),
            true, true, 1.2f,
            100, sf::Color(255, 255, 255, 255),
            3,
            false, 40, 0, 500, sf::Color(255, 255, 255, 255), 100, 0,
            1, 1,
            sf::Color(0, 0, 0, 255), 0, 20
        },
        {
            "Gaming",
            "High contrast gaming style",
            sf::Color(231, 76, 60, 255),
            sf::Color(255, 255, 0, 255),
            sf::Color(10, 10, 10, 255),
            sf::Color(255, 165, 0, 255),
            true, true, 1.1f,
            120, sf::Color(255, 200, 50, 255),
            5,
            true, 60, 15, 600, sf::Color(255, 255, 0, 255), 120, 0,
            2, 2,
            sf::Color(0, 0, 0, 255), 1, 22
        },
        {
            "Minimal",
            "Simple minimal design",
            sf::Color(200, 200, 200, 255),
            sf::Color(50, 50, 50, 255),
            sf::Color(240, 240, 240, 255),
            sf::Color(180, 180, 180, 255),
            false, false, 1.0f,
            100, sf::Color(255, 255, 255, 255),
            3,
            false, 40, 0, 500, sf::Color(255, 255, 255, 255), 100, 0,
            0, 0,
            sf::Color(0, 0, 0, 255), 0, 18
        },
        {
            "Neon",
            "Neon glow effect style",
            sf::Color(0, 255, 255, 255),
            sf::Color(255, 0, 255, 255),
            sf::Color(10, 0, 20, 255),
            sf::Color(255, 0, 255, 255),
            true, true, 1.3f,
            150, sf::Color(0, 255, 255, 255),
            6,
            true, 80, 30, 700, sf::Color(255, 0, 255, 255), 150, 0,
            3, 3,
            sf::Color(0, 0, 0, 255), 2, 20
        },
        {
            "Retro",
            "Retro pixel art style",
            sf::Color(139, 69, 19, 255),
            sf::Color(255, 215, 0, 255),
            sf::Color(40, 20, 0, 255),
            sf::Color(139, 69, 19, 255),
            false, true, 1.0f,
            100, sf::Color(255, 215, 0, 255),
            3,
            false, 40, 0, 500, sf::Color(255, 215, 0, 255), 100, 0,
            0, 2,
            sf::Color(0, 0, 0, 255), 0, 20
        },
        {
            "Ocean",
            "Deep blues and teals",
            sf::Color(0, 105, 148, 255),
            sf::Color(255, 255, 255, 255),
            sf::Color(10, 25, 40, 255),
            sf::Color(0, 168, 204, 255),
            true, true, 1.05f,
            120, sf::Color(0, 200, 255, 255),
            5,
            false, 40, 0, 500, sf::Color(255, 255, 255, 255), 100, 0,
            1, 3,
            sf::Color(0, 0, 0, 255), 1, 20
        },
        {
            "Forest",
            "Earth tones and greens",
            sf::Color(34, 139, 34, 255),
            sf::Color(240, 255, 240, 255),
            sf::Color(20, 40, 20, 255),
            sf::Color(50, 205, 50, 255),
            true, true, 1.1f,
            100, sf::Color(144, 238, 144, 255),
            3,
            false, 40, 0, 500, sf::Color(255, 255, 255, 255), 100, 0,
            2, 0,
            sf::Color(0, 0, 0, 255), 0, 20
        },
        {
            "Sunset",
            "Warm oranges and pinks",
            sf::Color(255, 127, 80, 255),
            sf::Color(255, 255, 255, 255),
            sf::Color(40, 10, 20, 255),
            sf::Color(255, 105, 180, 255),
            true, true, 1.2f,
            110, sf::Color(255, 218, 185, 255),
            4,
            false, 40, 0, 500, sf::Color(255, 255, 255, 255), 100, 0,
            3, 1,
            sf::Color(0, 0, 0, 255), 0, 20
        },
        {
            "Cyberpunk",
            "High contrast neon",
            sf::Color(255, 0, 60, 255),
            sf::Color(0, 255, 255, 255),
            sf::Color(10, 10, 20, 255),
            sf::Color(255, 255, 0, 255),
            true, true, 1.4f,
            150, sf::Color(0, 255, 255, 255),
            6,
            true, 60, 15, 600, sf::Color(255, 0, 255, 255), 150, 0,
            8, 6,
            sf::Color(0, 0, 0, 255), 2, 22
        },
        {
            "Monochrome",
            "Sleek greyscale design",
            sf::Color(100, 100, 100, 255),
            sf::Color(255, 255, 255, 255),
            sf::Color(0, 0, 0, 255),
            sf::Color(200, 200, 200, 255),
            true, false, 1.0f,
            100, sf::Color(255, 255, 255, 255),
            2,
            false, 40, 0, 500, sf::Color(255, 255, 255, 255), 100, 0,
            0, 0,
            sf::Color(0, 0, 0, 255), 0, 20
        }
    };

    mSettings.buttonColor = sf::Color(30, 30, 30, 255);
    mSettings.textColor = sf::Color(255, 255, 255, 255);
    mSettings.backgroundColor = sf::Color(140, 140, 140, 255);
    mSettings.borderColor = sf::Color(120, 180, 255, 255);
    mSettings.lightAnimation = true;
    mSettings.pressAnimation = false;
    mSettings.animationScale = 1.0f;
    mSettings.lightAnimScale = 100;
    mSettings.lightAnimColor = sf::Color(255, 255, 255, 255);
    mSettings.pressAnimOffset = 3;
    mSettings.keyPressVisEnabled = false;
    mSettings.keyPressVisSpeed = 40;
    mSettings.keyPressVisRotation = 0;
    mSettings.keyPressVisFadeDistance = 500;
    mSettings.keyPressVisColor = sf::Color(255, 255, 255, 255);
    mSettings.keyPressVisBonusWidth = 100;
    mSettings.keyPressVisFixedHeight = 0;
    mSettings.buttonShape = 0;
    mSettings.animationStyle = 0;
    mSettings.textOutlineColor = sf::Color(0, 0, 0, 255);
    mSettings.textOutlineThickness = 0;
    mSettings.textSize = 20;

    mOriginalSettings = mSettings;
}

StyleWizard::~StyleWizard()
{
    closeWindow();
}

void StyleWizard::openWindow(sf::Vector2i position)
{
    if (mWindow.isOpen())
        return;

    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    mWindow.create(sf::VideoMode(900, 800), "JKPS Style Wizard", sf::Style::Close, settings);
    mWindow.setPosition(position);
    mView = mWindow.getDefaultView();
    mWindow.setView(mView);

    loadConfig();
}

void StyleWizard::closeWindow()
{
    if (mWindow.isOpen())
        mWindow.close();
}

bool StyleWizard::isWindowOpen() const
{
    return mWindow.isOpen();
}

void StyleWizard::processInput()
{
    if (!mWindow.isOpen())
        return;

    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            closeWindow();
            return;
        }

        if (event.type == sf::Event::MouseMoved)
        {
            mMousePos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                mMousePressed = true;
            }
        }

        if (event.type == sf::Event::MouseButtonReleased)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                mMousePressed = false;
                mDraggingSpeed = false;
                mDraggingAnimSlider = -1;
            }
        }

        if (event.type == sf::Event::KeyPressed)
        {
            if (event.key.code == sf::Keyboard::Escape)
            {
                closeWindow();
            }
        }
    }

    switch (mCurrentStep)
    {
        case Step::Welcome:
            handleWelcomeStep();
            break;
        case Step::PresetSelection:
            handlePresetSelectionStep();
            break;
        case Step::ColorCustomization:
            handleColorCustomizationStep();
            break;
        case Step::AnimationSettings:
            handleAnimationSettingsStep();
            break;
        case Step::WindowSettings:
            handleWindowSettingsStep();
            break;
        case Step::ReviewApply:
            handleReviewApplyStep();
            break;
    }
}

void StyleWizard::render()
{
    if (!mWindow.isOpen())
        return;

    if (mStatusTimer > 0.f)
        mStatusTimer -= 0.016f;

    mWindow.clear(sf::Color(30, 30, 30));

    drawStepIndicator();

    switch (mCurrentStep)
    {
        case Step::Welcome:
            drawWelcomeStep();
            break;
        case Step::PresetSelection:
            drawPresetSelectionStep();
            break;
        case Step::ColorCustomization:
            drawColorCustomizationStep();
            break;
        case Step::AnimationSettings:
            drawAnimationSettingsStep();
            break;
        case Step::WindowSettings:
            drawWindowSettingsStep();
            break;
        case Step::ReviewApply:
            drawReviewApplyStep();
            break;
    }

    drawStatusMessage();

    mWindow.display();
}

void StyleWizard::drawWelcomeStep()
{
    sf::Text title;
    title.setFont(getFont());
    title.setString("JKPS Style Wizard");
    title.setCharacterSize(48);
    title.setFillColor(sf::Color::White);
    title.setPosition(450.f - title.getLocalBounds().width / 2.f, 150.f);
    mWindow.draw(title);

    sf::Text subtitle;
    subtitle.setFont(getFont());
    subtitle.setString("Customize your key overlay style");
    subtitle.setCharacterSize(24);
    subtitle.setFillColor(sf::Color(200, 200, 200));
    subtitle.setPosition(450.f - subtitle.getLocalBounds().width / 2.f, 220.f);
    mWindow.draw(subtitle);

    drawButton("Start", sf::Vector2f(350, 350), sf::Vector2f(200, 60), isButtonHovered(mMousePos, sf::FloatRect(350, 350, 200, 60)));
    drawButton("Cancel", sf::Vector2f(390, 430), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(390, 430, 120, 40)));
}

void StyleWizard::drawPresetSelectionStep()
{
    sf::Text title;
    title.setFont(getFont());
    title.setString("Choose a Preset Style");
    title.setCharacterSize(36);
    title.setFillColor(sf::Color::White);
    title.setPosition(450.f - title.getLocalBounds().width / 2.f, 80.f);
    mWindow.draw(title);

    const float startY = 130.f;
    const float spacingY = 70.f;
    const float buttonWidth = 350.f;
    const float buttonHeight = 60.f;
    const float colSpacing = 390.f;

    for (std::size_t i = 0; i < mPresets.size(); ++i)
    {
        float xPos = 80.f + static_cast<float>(i % 2) * colSpacing;
        float yPos = startY + static_cast<float>(i / 2) * spacingY;
        sf::Vector2f buttonPos(xPos, yPos);
        sf::FloatRect buttonBounds(buttonPos.x, buttonPos.y, buttonWidth, buttonHeight);

        bool hovered = isButtonHovered(mMousePos, buttonBounds);
        bool selected = (static_cast<int>(i) == mSelectedPreset);

        sf::Color buttonColor = hovered ? sf::Color(80, 80, 80, 255) : sf::Color(60, 60, 60, 255);
        if (selected)
            buttonColor = sf::Color(100, 100, 140, 255);

        sf::RectangleShape buttonShape;
        buttonShape.setSize(sf::Vector2f(buttonWidth, buttonHeight));
        buttonShape.setPosition(buttonPos);
        buttonShape.setFillColor(buttonColor);
        buttonShape.setOutlineThickness(selected ? 3.f : 1.f);
        buttonShape.setOutlineColor(selected ? sf::Color(120, 180, 255, 255) : sf::Color(100, 100, 100, 255));
        mWindow.draw(buttonShape);

        drawColorSwatch(mPresets[i].buttonColor, sf::Vector2f(buttonPos.x + 20, buttonPos.y + 20), 20.f);
        drawColorSwatch(mPresets[i].textColor, sf::Vector2f(buttonPos.x + 50, buttonPos.y + 20), 20.f);
        drawColorSwatch(mPresets[i].backgroundColor, sf::Vector2f(buttonPos.x + 80, buttonPos.y + 20), 20.f);

        sf::Text nameText;
        nameText.setFont(getFont());
        nameText.setString(mPresets[i].name);
        nameText.setCharacterSize(24);
        nameText.setFillColor(sf::Color::White);
        nameText.setPosition(buttonPos.x + 110, buttonPos.y + 10);
        mWindow.draw(nameText);

        sf::Text descText;
        descText.setFont(getFont());
        descText.setString(mPresets[i].description);
        descText.setCharacterSize(16);
        descText.setFillColor(sf::Color(180, 180, 180));
        descText.setPosition(buttonPos.x + 110, buttonPos.y + 35);
        mWindow.draw(descText);

        mPresetBounds[i] = buttonBounds;
    }

    drawButton("Back", sf::Vector2f(80, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(80, 730, 120, 40)));
    drawButton("Cancel", sf::Vector2f(220, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(220, 730, 120, 40)));
    drawButton("Next", sf::Vector2f(700, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(700, 730, 120, 40)));
}

void StyleWizard::drawColorCustomizationStep()
{
    sf::Text title;
    title.setFont(getFont());
    title.setString("Customize Colors");
    title.setCharacterSize(36);
    title.setFillColor(sf::Color::White);
    title.setPosition(450.f - title.getLocalBounds().width / 2.f, 80.f);
    mWindow.draw(title);

    sf::Text info;
    info.setFont(getFont());
    info.setString("Click a swatch or its label to change its color.\nColors are applied from the selected preset.");
    info.setCharacterSize(18);
    info.setFillColor(sf::Color(180, 180, 180));
    info.setPosition(450.f - info.getLocalBounds().width / 2.f, 130.f);
    mWindow.draw(info);

    drawColorSwatch(mSettings.buttonColor, sf::Vector2f(150, 180), 40.f);
    drawButton("Button Texture", sf::Vector2f(210, 180), sf::Vector2f(200, 40), mSelectedColorChannel == 0);

    drawColorSwatch(mSettings.textColor, sf::Vector2f(150, 240), 40.f);
    drawButton("Button Text", sf::Vector2f(210, 240), sf::Vector2f(200, 40), mSelectedColorChannel == 1);

    drawColorSwatch(mSettings.backgroundColor, sf::Vector2f(150, 300), 40.f);
    drawButton("Background", sf::Vector2f(210, 300), sf::Vector2f(200, 40), mSelectedColorChannel == 2);

    drawColorSwatch(mSettings.borderColor, sf::Vector2f(150, 360), 40.f);
    drawButton("Border/Accent", sf::Vector2f(210, 360), sf::Vector2f(200, 40), mSelectedColorChannel == 3);

    drawColorSwatch(mSettings.textOutlineColor, sf::Vector2f(150, 420), 40.f);
    drawButton("Text Outline Color", sf::Vector2f(210, 420), sf::Vector2f(200, 40), mSelectedColorChannel == 4);

    sf::Text shapeLabel;
    shapeLabel.setFont(getFont());
    shapeLabel.setString("Button Shape:");
    shapeLabel.setCharacterSize(16);
    shapeLabel.setFillColor(sf::Color::White);
    shapeLabel.setPosition(150, 480);
    mWindow.draw(shapeLabel);

    const char* shapeNames[] = {"Square", "Rounded", "Pill", "Circle", "Diamond", "Star", "Hex", "Triangle", "Octagon"};
    for (int i = 0; i < 9; ++i)
    {
        bool selected = (mSettings.buttonShape == i);
        float xOffset = static_cast<float>(i % 6) * 90.f;
        float yOffset = static_cast<float>(i / 6) * 40.f;
        sf::Vector2f btnPos(150.f + xOffset, 505.f + yOffset);
        drawButton(shapeNames[i], btnPos, sf::Vector2f(85, 30), selected, i);
        if (selected)
        {
            sf::RectangleShape highlight;
            highlight.setSize(sf::Vector2f(85.f, 30.f));
            highlight.setPosition(btnPos);
            highlight.setFillColor(sf::Color(100, 180, 255, 50));
            highlight.setOutlineThickness(2.f);
            highlight.setOutlineColor(sf::Color(100, 180, 255, 255));
            mWindow.draw(highlight);
        }
    }

    drawNumericSlider("Outline Thickness", mSettings.textOutlineThickness, 0, 10, sf::Vector2f(150, 590));
    drawNumericSlider("Text Size", mSettings.textSize, 10, 40, sf::Vector2f(150, 630));

    sf::FloatRect sliderRects[3];
    if (mSelectedColorChannel >= 0)
    {
        sf::Color *target = nullptr;
        if (mSelectedColorChannel == 0) target = &mSettings.buttonColor;
        else if (mSelectedColorChannel == 1) target = &mSettings.textColor;
        else if (mSelectedColorChannel == 2) target = &mSettings.backgroundColor;
        else if (mSelectedColorChannel == 3) target = &mSettings.borderColor;
        else if (mSelectedColorChannel == 4) target = &mSettings.textOutlineColor;

        if (target)
        {
            sf::Vector2f sliderPos(430, 180);
            drawColorSliders(*target, sliderPos);

            const float barWidth = 200.f;
            const float barHeight = 12.f;
            const float spacing = 35.f;
            for (int i = 0; i < 3; ++i)
            {
                sliderRects[i] = sf::FloatRect(sliderPos.x + 30, sliderPos.y + static_cast<float>(i) * spacing + 10, barWidth, barHeight);
            }

            sf::Text selectedLabel;
            selectedLabel.setFont(getFont());
            selectedLabel.setString("Editing: " + std::string(
                mSelectedColorChannel == 0 ? "Button Texture" :
                mSelectedColorChannel == 1 ? "Button Text" :
                mSelectedColorChannel == 2 ? "Background" :
                mSelectedColorChannel == 3 ? "Border/Accent" : "Text Outline"));
            selectedLabel.setCharacterSize(16);
            selectedLabel.setFillColor(sf::Color(255, 200, 100));
            selectedLabel.setPosition(430, 140);
            mWindow.draw(selectedLabel);
        }
    }

    sf::Text previewLabel;
    previewLabel.setFont(getFont());
    previewLabel.setString("Live Preview");
    previewLabel.setCharacterSize(18);
    previewLabel.setFillColor(sf::Color(180, 180, 180));
    previewLabel.setPosition(520, 180);
    mWindow.draw(previewLabel);

    drawPreviewButton(mSettings.buttonColor, mSettings.textColor, sf::Vector2f(520, 210));

    if (mColorCustomizationDirty)
    {
        sf::Text dirty;
        dirty.setFont(getFont());
        dirty.setString("(unsaved changes)");
        dirty.setCharacterSize(16);
        dirty.setFillColor(sf::Color(255, 200, 100));
        dirty.setPosition(420, 360);
        mWindow.draw(dirty);
    }

    drawButton("Back", sf::Vector2f(80, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(80, 730, 120, 40)));
    drawButton("Cancel", sf::Vector2f(220, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(220, 730, 120, 40)));
    drawButton("Reset", sf::Vector2f(360, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(360, 730, 120, 40)));
    drawButton("Next", sf::Vector2f(700, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(700, 730, 120, 40)));
}

void StyleWizard::drawAnimationSettingsStep()
{
    sf::Text title;
    title.setFont(getFont());
    title.setString("Animation & Effects");
    title.setCharacterSize(36);
    title.setFillColor(sf::Color::White);
    title.setPosition(450.f - title.getLocalBounds().width / 2.f, 80.f);
    mWindow.draw(title);

    sf::Text info;
    info.setFont(getFont());
    info.setString("Configure animations and visual effects.");
    info.setCharacterSize(18);
    info.setFillColor(sf::Color(180, 180, 180));
    info.setPosition(450.f - info.getLocalBounds().width / 2.f, 130.f);
    mWindow.draw(info);

    // ==========================================
    // LEFT COLUMN (X = 100)
    // ==========================================
    float leftX = 100.f;

    // Header 1: General & Press Animation
    sf::Text generalHeader;
    generalHeader.setFont(getFont());
    generalHeader.setString("General & Press Animation");
    generalHeader.setCharacterSize(18);
    generalHeader.setFillColor(sf::Color(100, 180, 255));
    generalHeader.setPosition(leftX, 180.f);
    mWindow.draw(generalHeader);

    drawToggle("Light Animation", mSettings.lightAnimation, sf::Vector2f(leftX, 210.f));
    drawToggle("Press Animation", mSettings.pressAnimation, sf::Vector2f(leftX, 250.f));

    // Animation Speed Slider
    sf::Text speedLabel;
    speedLabel.setFont(getFont());
    speedLabel.setString("Animation Speed");
    speedLabel.setCharacterSize(16);
    speedLabel.setFillColor(sf::Color::White);
    speedLabel.setPosition(leftX, 295.f);
    mWindow.draw(speedLabel);

    char speedBuf[32];
    std::snprintf(speedBuf, sizeof(speedBuf), "%.1fx", mSettings.animationScale);
    sf::Text speedValue;
    speedValue.setFont(getFont());
    speedValue.setString(speedBuf);
    speedValue.setCharacterSize(16);
    speedValue.setFillColor(sf::Color(200, 200, 200));
    speedValue.setPosition(leftX + 250.f, 295.f);
    mWindow.draw(speedValue);

    // Speed bar background first
    sf::RectangleShape speedBg;
    speedBg.setSize(sf::Vector2f(300.f, 10.f));
    speedBg.setPosition(sf::Vector2f(leftX, 320.f));
    speedBg.setFillColor(sf::Color(60, 60, 60, 255));
    mWindow.draw(speedBg);

    // Speed bar fill second
    float speedFraction = (mSettings.animationScale - 0.5f) / 1.5f;
    sf::RectangleShape speedBar;
    speedBar.setSize(sf::Vector2f(300.f * speedFraction, 10.f));
    speedBar.setPosition(sf::Vector2f(leftX, 320.f));
    speedBar.setFillColor(sf::Color(100, 180, 255, 255));
    mWindow.draw(speedBar);

    // Press Offset Slider
    sf::Text offsetLabel;
    offsetLabel.setFont(getFont());
    offsetLabel.setString("Press Offset:");
    offsetLabel.setCharacterSize(16);
    offsetLabel.setFillColor(sf::Color::White);
    offsetLabel.setPosition(leftX, 350.f);
    mWindow.draw(offsetLabel);

    sf::Text offsetValue;
    offsetValue.setFont(getFont());
    offsetValue.setString(std::to_string(mSettings.pressAnimOffset) + "px");
    offsetValue.setCharacterSize(16);
    offsetValue.setFillColor(sf::Color(200, 200, 200));
    offsetValue.setPosition(leftX + 250.f, 350.f);
    mWindow.draw(offsetValue);

    // Offset bg first
    sf::RectangleShape offsetBg;
    offsetBg.setSize(sf::Vector2f(300.f, 10.f));
    offsetBg.setPosition(sf::Vector2f(leftX, 375.f));
    offsetBg.setFillColor(sf::Color(60, 60, 60, 255));
    mWindow.draw(offsetBg);

    // Offset fill second
    sf::RectangleShape offsetBar;
    offsetBar.setSize(sf::Vector2f(300.f * static_cast<float>(mSettings.pressAnimOffset) / 20.f, 10.f));
    offsetBar.setPosition(sf::Vector2f(leftX, 375.f));
    offsetBar.setFillColor(sf::Color(180, 100, 100, 255));
    mWindow.draw(offsetBar);

    // Animation Style Section
    sf::Text animStyleLabel;
    animStyleLabel.setFont(getFont());
    animStyleLabel.setString("Animation Style:");
    animStyleLabel.setCharacterSize(18);
    animStyleLabel.setFillColor(sf::Color(100, 180, 255));
    animStyleLabel.setPosition(leftX, 420.f);
    mWindow.draw(animStyleLabel);

    const char* animStyleNames[] = {"Default", "Pulse", "Bounce", "Smooth", "Wave", "Spin", "Glow"};
    // Row 1: Default, Pulse, Bounce, Smooth
    for (int i = 0; i < 4; ++i)
    {
        bool selected = (mSettings.animationStyle == i);
        sf::Vector2f btnPos(leftX + static_cast<float>(i) * 85.f, 455.f);
        drawButton(animStyleNames[i], btnPos, sf::Vector2f(80.f, 30.f), selected);
        if (selected)
        {
            sf::RectangleShape highlight;
            highlight.setSize(sf::Vector2f(80.f, 30.f));
            highlight.setPosition(btnPos);
            highlight.setFillColor(sf::Color(100, 180, 255, 50));
            highlight.setOutlineThickness(2.f);
            highlight.setOutlineColor(sf::Color(100, 180, 255, 255));
            mWindow.draw(highlight);
        }
    }
    // Row 2: Wave, Spin, Glow
    for (int i = 4; i < 7; ++i)
    {
        bool selected = (mSettings.animationStyle == i);
        sf::Vector2f btnPos(leftX + static_cast<float>(i - 4) * 85.f, 495.f);
        drawButton(animStyleNames[i], btnPos, sf::Vector2f(80.f, 30.f), selected);
        if (selected)
        {
            sf::RectangleShape highlight;
            highlight.setSize(sf::Vector2f(80.f, 30.f));
            highlight.setPosition(btnPos);
            highlight.setFillColor(sf::Color(100, 180, 255, 50));
            highlight.setOutlineThickness(2.f);
            highlight.setOutlineColor(sf::Color(100, 180, 255, 255));
            mWindow.draw(highlight);
        }
    }

    // ==========================================
    // RIGHT COLUMN (X = 500)
    // ==========================================
    float rightX = 500.f;

    // Header 2: Key Press Visualization
    sf::Text kpHeader;
    kpHeader.setFont(getFont());
    kpHeader.setString("Key Press Visualization");
    kpHeader.setCharacterSize(18);
    kpHeader.setFillColor(sf::Color(100, 180, 255));
    kpHeader.setPosition(rightX, 180.f);
    mWindow.draw(kpHeader);

    drawToggle("Enable Key Press Vis", mSettings.keyPressVisEnabled, sf::Vector2f(rightX, 210.f));

    // Key Press Speed Slider
    sf::Text kpSpeedLabel;
    kpSpeedLabel.setFont(getFont());
    kpSpeedLabel.setString("Key Press Speed:");
    kpSpeedLabel.setCharacterSize(16);
    kpSpeedLabel.setFillColor(sf::Color::White);
    kpSpeedLabel.setPosition(rightX, 250.f);
    mWindow.draw(kpSpeedLabel);

    sf::Text kpSpeedValue;
    kpSpeedValue.setFont(getFont());
    kpSpeedValue.setString(std::to_string(mSettings.keyPressVisSpeed));
    kpSpeedValue.setCharacterSize(16);
    kpSpeedValue.setFillColor(sf::Color(200, 200, 200));
    kpSpeedValue.setPosition(rightX + 250.f, 250.f);
    mWindow.draw(kpSpeedValue);

    // Speed bg first
    sf::RectangleShape kpSpeedBg;
    kpSpeedBg.setSize(sf::Vector2f(300.f, 10.f));
    kpSpeedBg.setPosition(sf::Vector2f(rightX, 275.f));
    kpSpeedBg.setFillColor(sf::Color(60, 60, 60, 255));
    mWindow.draw(kpSpeedBg);

    // Speed fill second
    sf::RectangleShape kpSpeedBar;
    kpSpeedBar.setSize(sf::Vector2f(300.f * static_cast<float>(mSettings.keyPressVisSpeed) / 100.f, 10.f));
    kpSpeedBar.setPosition(sf::Vector2f(rightX, 275.f));
    kpSpeedBar.setFillColor(sf::Color(100, 200, 255, 255));
    mWindow.draw(kpSpeedBar);

    // Key Press Rotation Slider
    sf::Text kpRotLabel;
    kpRotLabel.setFont(getFont());
    kpRotLabel.setString("Key Press Rotation:");
    kpRotLabel.setCharacterSize(16);
    kpRotLabel.setFillColor(sf::Color::White);
    kpRotLabel.setPosition(rightX, 305.f);
    mWindow.draw(kpRotLabel);

    sf::Text kpRotValue;
    kpRotValue.setFont(getFont());
    kpRotValue.setString(std::to_string(mSettings.keyPressVisRotation) + " deg");
    kpRotValue.setCharacterSize(16);
    kpRotValue.setFillColor(sf::Color(200, 200, 200));
    kpRotValue.setPosition(rightX + 250.f, 305.f);
    mWindow.draw(kpRotValue);

    // Rotation bg first
    sf::RectangleShape kpRotBg;
    kpRotBg.setSize(sf::Vector2f(300.f, 10.f));
    kpRotBg.setPosition(sf::Vector2f(rightX, 330.f));
    kpRotBg.setFillColor(sf::Color(60, 60, 60, 255));
    mWindow.draw(kpRotBg);

    // Rotation fill second
    sf::RectangleShape kpRotBar;
    kpRotBar.setSize(sf::Vector2f(300.f * static_cast<float>(mSettings.keyPressVisRotation) / 360.f, 10.f));
    kpRotBar.setPosition(sf::Vector2f(rightX, 330.f));
    kpRotBar.setFillColor(sf::Color(100, 200, 255, 255));
    mWindow.draw(kpRotBar);

    // Key Press Fade Distance Slider
    sf::Text kpFadeLabel;
    kpFadeLabel.setFont(getFont());
    kpFadeLabel.setString("Key Press Fade Distance:");
    kpFadeLabel.setCharacterSize(16);
    kpFadeLabel.setFillColor(sf::Color::White);
    kpFadeLabel.setPosition(rightX, 360.f);
    mWindow.draw(kpFadeLabel);

    sf::Text kpFadeValue;
    kpFadeValue.setFont(getFont());
    kpFadeValue.setString(std::to_string(mSettings.keyPressVisFadeDistance) + " px");
    kpFadeValue.setCharacterSize(16);
    kpFadeValue.setFillColor(sf::Color(200, 200, 200));
    kpFadeValue.setPosition(rightX + 250.f, 360.f);
    mWindow.draw(kpFadeValue);

    // Fade bg first
    sf::RectangleShape kpFadeBg;
    kpFadeBg.setSize(sf::Vector2f(300.f, 10.f));
    kpFadeBg.setPosition(sf::Vector2f(rightX, 385.f));
    kpFadeBg.setFillColor(sf::Color(60, 60, 60, 255));
    mWindow.draw(kpFadeBg);

    // Fade fill second
    sf::RectangleShape kpFadeBar;
    kpFadeBar.setSize(sf::Vector2f(300.f * static_cast<float>(mSettings.keyPressVisFadeDistance) / 1000.f, 10.f));
    kpFadeBar.setPosition(sf::Vector2f(rightX, 385.f));
    kpFadeBar.setFillColor(sf::Color(100, 200, 255, 255));
    mWindow.draw(kpFadeBar);

    // Key Press Color
    drawColorSwatch(mSettings.keyPressVisColor, sf::Vector2f(rightX, 415.f), 30.f);
    drawButton("Key Press Color", sf::Vector2f(rightX + 45.f, 415.f), sf::Vector2f(160.f, 30.f), false);

    // Header 3: Light Effects
    sf::Text lightHeader;
    lightHeader.setFont(getFont());
    lightHeader.setString("Light Effects");
    lightHeader.setCharacterSize(18);
    lightHeader.setFillColor(sf::Color(100, 180, 255));
    lightHeader.setPosition(rightX, 470.f);
    mWindow.draw(lightHeader);

    // Light Scale Slider
    sf::Text scaleLabel;
    scaleLabel.setFont(getFont());
    scaleLabel.setString("Light Scale on Click (%):");
    scaleLabel.setCharacterSize(16);
    scaleLabel.setFillColor(sf::Color::White);
    scaleLabel.setPosition(rightX, 500.f);
    mWindow.draw(scaleLabel);

    sf::Text scaleValue;
    scaleValue.setFont(getFont());
    scaleValue.setString(std::to_string(mSettings.lightAnimScale) + "%");
    scaleValue.setCharacterSize(16);
    scaleValue.setFillColor(sf::Color(200, 200, 200));
    scaleValue.setPosition(rightX + 250.f, 500.f);
    mWindow.draw(scaleValue);

    // Scale bg first
    sf::RectangleShape scaleBg;
    scaleBg.setSize(sf::Vector2f(300.f, 10.f));
    scaleBg.setPosition(sf::Vector2f(rightX, 525.f));
    scaleBg.setFillColor(sf::Color(60, 60, 60, 255));
    mWindow.draw(scaleBg);

    // Scale fill second
    sf::RectangleShape scaleBar;
    scaleBar.setSize(sf::Vector2f(300.f * static_cast<float>(mSettings.lightAnimScale) / 200.f, 10.f));
    scaleBar.setPosition(sf::Vector2f(rightX, 525.f));
    scaleBar.setFillColor(sf::Color(100, 180, 100, 255));
    mWindow.draw(scaleBar);

    // Light Color
    drawColorSwatch(mSettings.lightAnimColor, sf::Vector2f(rightX, 555.f), 30.f);
    drawButton("Light Color", sf::Vector2f(rightX + 45.f, 555.f), sf::Vector2f(140.f, 30.f), false);

    sf::Text animPreviewLabel;
    animPreviewLabel.setFont(getFont());
    animPreviewLabel.setString("Live Animation Preview");
    animPreviewLabel.setCharacterSize(18);
    animPreviewLabel.setFillColor(sf::Color(100, 180, 255));
    animPreviewLabel.setPosition(rightX, 595.f);
    mWindow.draw(animPreviewLabel);

    drawAnimStylePreview(mSettings.animationStyle, sf::Vector2f(rightX, 620.f), mSettings.animationScale);

    // ==========================================
    // COLOR PICKER SUB-PANEL
    // ==========================================
    if (mSelectedAnimColor >= 0)
    {
        sf::Color *target = (mSelectedAnimColor == 0) ? &mSettings.lightAnimColor : &mSettings.keyPressVisColor;
        sf::Vector2f sliderPos(100.f, 580.f);

        sf::Text editLabel;
        editLabel.setFont(getFont());
        editLabel.setString("Editing: " + std::string(mSelectedAnimColor == 0 ? "Light Color" : "Key Press Color"));
        editLabel.setCharacterSize(16);
        editLabel.setFillColor(sf::Color(255, 200, 100));
        editLabel.setPosition(100.f, 550.f);
        mWindow.draw(editLabel);

        drawColorSliders(*target, sliderPos);
    }

    // ==========================================
    // FOOTER & DIRTY INDICATOR
    // ==========================================
    if (mAnimationSettingsDirty)
    {
        sf::Text dirty;
        dirty.setFont(getFont());
        dirty.setString("(unsaved changes)");
        dirty.setCharacterSize(16);
        dirty.setFillColor(sf::Color(255, 200, 100));
        dirty.setPosition(450.f - dirty.getLocalBounds().width / 2.f, 680.f);
        mWindow.draw(dirty);
    }

    drawButton("Back", sf::Vector2f(80, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(80, 730, 120, 40)));
    drawButton("Cancel", sf::Vector2f(220, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(220, 730, 120, 40)));
    drawButton("Reset", sf::Vector2f(360, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(360, 730, 120, 40)));
    drawButton("Next", sf::Vector2f(700, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(700, 730, 120, 40)));
}

void StyleWizard::drawWindowSettingsStep()
{
    sf::Text title;
    title.setFont(getFont());
    title.setString("Window Settings");
    title.setCharacterSize(36);
    title.setFillColor(sf::Color::White);
    title.setPosition(450.f - title.getLocalBounds().width / 2.f, 80.f);
    mWindow.draw(title);

    sf::Text info;
    info.setFont(getFont());
    info.setString("Configure window features and padding borders.");
    info.setCharacterSize(18);
    info.setFillColor(sf::Color(180, 180, 180));
    info.setPosition(450.f - info.getLocalBounds().width / 2.f, 130.f);
    mWindow.draw(info);

    drawToggle("Window Title Bar", mSettings.windowTitleBar, sf::Vector2f(150, 210));
    drawToggle("Window Resizable", mSettings.windowResizable, sf::Vector2f(150, 260));

    sf::Text paddingLabel;
    paddingLabel.setFont(getFont());
    paddingLabel.setString("Window Padding (Bonus Sizes):");
    paddingLabel.setCharacterSize(20);
    paddingLabel.setFillColor(sf::Color(100, 180, 255));
    paddingLabel.setPosition(150, 300);
    mWindow.draw(paddingLabel);

    drawNumericSlider("Top Padding", mSettings.windowBonusSizeTop, -500, 500, sf::Vector2f(150, 330));
    drawNumericSlider("Bottom Padding", mSettings.windowBonusSizeBottom, -500, 500, sf::Vector2f(150, 380));
    drawNumericSlider("Left Padding", mSettings.windowBonusSizeLeft, -500, 500, sf::Vector2f(150, 430));
    drawNumericSlider("Right Padding", mSettings.windowBonusSizeRight, -500, 500, sf::Vector2f(150, 480));

    drawButton("Back", sf::Vector2f(80, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(80, 730, 120, 40)));
    drawButton("Cancel", sf::Vector2f(220, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(220, 730, 120, 40)));
    drawButton("Reset", sf::Vector2f(360, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(360, 730, 120, 40)));
    drawButton("Next", sf::Vector2f(700, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(700, 730, 120, 40)));
}

void StyleWizard::drawReviewApplyStep()
{
    sf::Text title;
    title.setFont(getFont());
    title.setString("Review & Apply");
    title.setCharacterSize(36);
    title.setFillColor(sf::Color::White);
    title.setPosition(450.f - title.getLocalBounds().width / 2.f, 80.f);
    mWindow.draw(title);

    auto colorStr = [](sf::Color c) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%d, %d, %d, %d", c.r, c.g, c.b, c.a);
        return std::string(buf);
    };

    sf::Text leftInfo;
    leftInfo.setFont(getFont());
    leftInfo.setString(
        "Button Texture: " + colorStr(mSettings.buttonColor) + "\n\n" +
        "Button Text: " + colorStr(mSettings.textColor) + "\n\n" +
        "Background: " + colorStr(mSettings.backgroundColor) + "\n\n" +
        "Border/Accent: " + colorStr(mSettings.borderColor) + "\n\n" +
        "Button Shape: " + std::string(
            mSettings.buttonShape == 0 ? "Square" :
            mSettings.buttonShape == 1 ? "Rounded" :
            mSettings.buttonShape == 2 ? "Pill" :
            mSettings.buttonShape == 3 ? "Circle" :
            mSettings.buttonShape == 4 ? "Diamond" :
            mSettings.buttonShape == 5 ? "Star" :
            mSettings.buttonShape == 6 ? "Hex" :
            mSettings.buttonShape == 7 ? "Triangle" : "Octagon") + "\n\n" +
        "Light Animation: " + std::string(mSettings.lightAnimation ? "On" : "Off") + "\n\n" +
        "Press Animation: " + std::string(mSettings.pressAnimation ? "On" : "Off")
    );
    leftInfo.setCharacterSize(18);
    leftInfo.setFillColor(sf::Color(200, 200, 200));
    leftInfo.setPosition(150, 160);
    mWindow.draw(leftInfo);

    sf::Text rightInfo;
    rightInfo.setFont(getFont());
    rightInfo.setString(
        "Animation Style: " + std::string(
            mSettings.animationStyle == 0 ? "Default" :
            mSettings.animationStyle == 1 ? "Pulse" :
            mSettings.animationStyle == 2 ? "Bounce" :
            mSettings.animationStyle == 3 ? "Smooth" :
            mSettings.animationStyle == 4 ? "Wave" :
            mSettings.animationStyle == 5 ? "Spin" : "Glow") + "\n\n" +
        "Animation Speed: " + std::to_string(static_cast<int>(mSettings.animationScale * 5)) + " frames\n\n" +
        "Light Scale: " + std::to_string(mSettings.lightAnimScale) + "%\n\n" +
        "Press Offset: " + std::to_string(mSettings.pressAnimOffset) + "px\n\n" +
        "Key Press Vis: " + std::string(mSettings.keyPressVisEnabled ? "On" : "Off") + "\n\n" +
        "Key Press Speed: " + std::to_string(mSettings.keyPressVisSpeed) + "\n\n" +
        "Key Press Rotation: " + std::to_string(mSettings.keyPressVisRotation) + " deg\n\n" +
        "Key Press Fade: " + std::to_string(mSettings.keyPressVisFadeDistance) + " px"
    );
    rightInfo.setCharacterSize(18);
    rightInfo.setFillColor(sf::Color(200, 200, 200));
    rightInfo.setPosition(500, 160);
    mWindow.draw(rightInfo);

    drawButton("Back", sf::Vector2f(80, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(80, 730, 120, 40)));
    drawButton("Cancel", sf::Vector2f(220, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(220, 730, 120, 40)));
    drawButton("Reset", sf::Vector2f(360, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(360, 730, 120, 40)));
    drawButton("Apply", sf::Vector2f(700, 730), sf::Vector2f(120, 40), isButtonHovered(mMousePos, sf::FloatRect(700, 730, 120, 40)));
}

void StyleWizard::handleWelcomeStep()
{
    if (mMousePressed)
    {
        sf::FloatRect startButton(350, 350, 200, 60);
        if (isButtonHovered(mMousePos, startButton))
        {
            mCurrentStep = Step::PresetSelection;
            mMousePressed = false;
        }

        sf::FloatRect cancelButton(390, 430, 120, 40);
        if (isButtonHovered(mMousePos, cancelButton))
        {
            closeWindow();
            mMousePressed = false;
        }
    }
}

void StyleWizard::handlePresetSelectionStep()
{
    if (mMousePressed)
    {
        for (std::size_t i = 0; i < mPresets.size(); ++i)
        {
            if (isButtonHovered(mMousePos, mPresetBounds[i]))
            {
                mSelectedPreset = static_cast<int>(i);
                applyPreset(mPresets[i].name);
                mMousePressed = false;
                break;
            }
        }

        sf::FloatRect nextButton(700, 730, 120, 40);
        if (isButtonHovered(mMousePos, nextButton))
        {
            mCurrentStep = Step::ColorCustomization;
            mMousePressed = false;
        }

        sf::FloatRect backButton(80, 730, 120, 40);
        if (isButtonHovered(mMousePos, backButton))
        {
            mCurrentStep = Step::Welcome;
            mMousePressed = false;
        }

        sf::FloatRect cancelButton(220, 730, 120, 40);
        if (isButtonHovered(mMousePos, cancelButton))
        {
            closeWindow();
            mMousePressed = false;
        }
    }
}

void StyleWizard::handleColorCustomizationStep()
{
    if (mMousePressed)
    {
        sf::FloatRect nextButton(700, 730, 120, 40);
        if (isButtonHovered(mMousePos, nextButton))
        {
            mCurrentStep = Step::AnimationSettings;
            mColorCustomizationDirty = false;
            mMousePressed = false;
            mSelectedColorChannel = -1;
            mDraggingSlider = -1;
        }

        sf::FloatRect backButton(80, 730, 120, 40);
        if (isButtonHovered(mMousePos, backButton))
        {
            mCurrentStep = Step::PresetSelection;
            mColorCustomizationDirty = false;
            mMousePressed = false;
            mSelectedColorChannel = -1;
            mDraggingSlider = -1;
        }

        sf::FloatRect cancelButton(220, 730, 120, 40);
        if (isButtonHovered(mMousePos, cancelButton))
        {
            closeWindow();
            mMousePressed = false;
            mSelectedColorChannel = -1;
            mDraggingSlider = -1;
        }

        sf::FloatRect resetButton(360, 730, 120, 40);
        if (isButtonHovered(mMousePos, resetButton))
        {
            resetToOriginal();
            mMousePressed = false;
            mSelectedColorChannel = -1;
            mDraggingSlider = -1;
        }

        if (isButtonHovered(mMousePos, sf::FloatRect(150, 180, 260, 40)))
        {
            mSettings.buttonColor = mPresets[static_cast<std::size_t>(mSelectedPreset >= 0 ? mSelectedPreset : 0)].buttonColor;
            mColorCustomizationDirty = true;
            mSelectedColorChannel = 0;
            mMousePressed = false;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(150, 240, 260, 40)))
        {
            mSettings.textColor = mPresets[static_cast<std::size_t>(mSelectedPreset >= 0 ? mSelectedPreset : 0)].textColor;
            mColorCustomizationDirty = true;
            mSelectedColorChannel = 1;
            mMousePressed = false;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(150, 300, 260, 40)))
        {
            mSettings.backgroundColor = mPresets[static_cast<std::size_t>(mSelectedPreset >= 0 ? mSelectedPreset : 0)].backgroundColor;
            mColorCustomizationDirty = true;
            mSelectedColorChannel = 2;
            mMousePressed = false;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(150, 360, 260, 40)))
        {
            mSettings.borderColor = mPresets[static_cast<std::size_t>(mSelectedPreset >= 0 ? mSelectedPreset : 0)].borderColor;
            mColorCustomizationDirty = true;
            mSelectedColorChannel = 3;
            mMousePressed = false;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(150, 420, 260, 40)))
        {
            mSettings.textOutlineColor = mPresets[static_cast<std::size_t>(mSelectedPreset >= 0 ? mSelectedPreset : 0)].textOutlineColor;
            mColorCustomizationDirty = true;
            mSelectedColorChannel = 4;
            mMousePressed = false;
        }

        for (int i = 0; i < 9; ++i)
        {
            float xOffset = static_cast<float>(i % 6) * 90.f;
            float yOffset = static_cast<float>(i / 6) * 40.f;
            if (isButtonHovered(mMousePos, sf::FloatRect(150.f + xOffset, 505.f + yOffset, 85.f, 30.f)))
            {
                mSettings.buttonShape = i;
                mSelectedButtonShape = i;
                mColorCustomizationDirty = true;
                mMousePressed = false;
                break;
            }
        }
    }

    handleNumericSlider(mSettings.textOutlineThickness, 0, 10, sf::Vector2f(150, 590));
    handleNumericSlider(mSettings.textSize, 10, 40, sf::Vector2f(150, 630));

    if (mSelectedColorChannel >= 0)
    {
        sf::Color *target = nullptr;
        if (mSelectedColorChannel == 0) target = &mSettings.buttonColor;
        else if (mSelectedColorChannel == 1) target = &mSettings.textColor;
        else if (mSelectedColorChannel == 2) target = &mSettings.backgroundColor;
        else if (mSelectedColorChannel == 3) target = &mSettings.borderColor;
        else if (mSelectedColorChannel == 4) target = &mSettings.textOutlineColor;

        if (target)
        {
            sf::FloatRect sliderRects[3];
            const float barWidth = 200.f;
            const float barHeight = 12.f;
            const float spacing = 35.f;
            sf::Vector2f sliderPos(430, 180);
            for (int i = 0; i < 3; ++i)
            {
                sliderRects[i] = sf::FloatRect(sliderPos.x + 30, sliderPos.y + static_cast<float>(i) * spacing + 10, barWidth, barHeight);
            }
            handleColorSliders(*target, sliderPos, sliderRects);
        }
    }
}

void StyleWizard::handleAnimationSettingsStep()
{
    if (mMousePressed)
    {
        sf::FloatRect nextButton(700, 730, 120, 40);
        if (isButtonHovered(mMousePos, nextButton))
        {
            mCurrentStep = Step::WindowSettings;
            mAnimationSettingsDirty = false;
            mMousePressed = false;
            mDraggingSpeed = false;
            mDraggingAnimSlider = -1;
            mSelectedAnimColor = -1;
        }

        sf::FloatRect backButton(80, 730, 120, 40);
        if (isButtonHovered(mMousePos, backButton))
        {
            mCurrentStep = Step::ColorCustomization;
            mAnimationSettingsDirty = false;
            mMousePressed = false;
            mDraggingSpeed = false;
            mDraggingAnimSlider = -1;
            mSelectedAnimColor = -1;
        }

        sf::FloatRect cancelButton(220, 730, 120, 40);
        if (isButtonHovered(mMousePos, cancelButton))
        {
            closeWindow();
            mMousePressed = false;
            mDraggingSpeed = false;
            mDraggingAnimSlider = -1;
            mSelectedAnimColor = -1;
        }

        sf::FloatRect resetButton(360, 730, 120, 40);
        if (isButtonHovered(mMousePos, resetButton))
        {
            resetToOriginal();
            mMousePressed = false;
            mDraggingSpeed = false;
            mDraggingAnimSlider = -1;
            mSelectedAnimColor = -1;
        }

        if (isButtonHovered(mMousePos, sf::FloatRect(100, 210, 300, 30)))
        {
            mSettings.lightAnimation = !mSettings.lightAnimation;
            mAnimationSettingsDirty = true;
            mMousePressed = false;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(100, 250, 300, 30)))
        {
            mSettings.pressAnimation = !mSettings.pressAnimation;
            mAnimationSettingsDirty = true;
            mMousePressed = false;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(100, 320, 300, 20)))
        {
            mDraggingSpeed = true;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(500, 525, 300, 20)))
        {
            mDraggingAnimSlider = 0;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(500, 555, 185, 30)))
        {
            mSelectedAnimColor = 0;
            mMousePressed = false;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(100, 375, 300, 20)))
        {
            mDraggingAnimSlider = 1;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(500, 210, 300, 30)))
        {
            mSettings.keyPressVisEnabled = !mSettings.keyPressVisEnabled;
            mAnimationSettingsDirty = true;
            mMousePressed = false;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(500, 275, 300, 20)))
        {
            mDraggingAnimSlider = 2;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(500, 330, 300, 20)))
        {
            mDraggingAnimSlider = 3;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(500, 385, 300, 20)))
        {
            mDraggingAnimSlider = 4;
        }
        if (isButtonHovered(mMousePos, sf::FloatRect(500, 415, 205, 30)))
        {
            mSelectedAnimColor = 1;
            mMousePressed = false;
        }

        for (int i = 0; i < 7; ++i)
        {
            sf::FloatRect btnBounds;
            if (i < 4)
                btnBounds = sf::FloatRect(100.f + static_cast<float>(i) * 85.f, 455.f, 80.f, 30.f);
            else
                btnBounds = sf::FloatRect(100.f + static_cast<float>(i - 4) * 85.f, 495.f, 80.f, 30.f);

            if (isButtonHovered(mMousePos, btnBounds))
            {
                mSettings.animationStyle = i;
                mSelectedAnimStyle = i;
                applyAnimationStyle(i);
                mAnimationSettingsDirty = true;
                mMousePressed = false;
                break;
            }
        }
    }
    else if (mDraggingSpeed)
    {
        float speed = (static_cast<float>(mMousePos.x) - 100.f) / 300.f;
        mSettings.animationScale = 0.5f + speed * 1.5f;
        mSettings.animationScale = std::clamp(mSettings.animationScale, 0.5f, 2.0f);
        mAnimationSettingsDirty = true;
    }
    else if (mDraggingAnimSlider >= 0)
    {
        if (mDraggingAnimSlider == 0)
        {
            float val = (static_cast<float>(mMousePos.x) - 500.f) / 300.f;
            val = std::clamp(val, 0.f, 1.f);
            mSettings.lightAnimScale = static_cast<int>(val * 200.f);
            mAnimationSettingsDirty = true;
        }
        else if (mDraggingAnimSlider == 1)
        {
            float val = (static_cast<float>(mMousePos.x) - 100.f) / 300.f;
            val = std::clamp(val, 0.f, 1.f);
            mSettings.pressAnimOffset = static_cast<int>(val * 20.f);
            mAnimationSettingsDirty = true;
        }
        else if (mDraggingAnimSlider == 2)
        {
            float val = (static_cast<float>(mMousePos.x) - 500.f) / 300.f;
            val = std::clamp(val, 0.f, 1.f);
            mSettings.keyPressVisSpeed = static_cast<int>(val * 100.f);
            mAnimationSettingsDirty = true;
        }
        else if (mDraggingAnimSlider == 3)
        {
            float val = (static_cast<float>(mMousePos.x) - 500.f) / 300.f;
            val = std::clamp(val, 0.f, 1.f);
            mSettings.keyPressVisRotation = static_cast<int>(val * 360.f);
            mAnimationSettingsDirty = true;
        }
        else if (mDraggingAnimSlider == 4)
        {
            float val = (static_cast<float>(mMousePos.x) - 500.f) / 300.f;
            val = std::clamp(val, 0.f, 1.f);
            mSettings.keyPressVisFadeDistance = static_cast<int>(val * 1000.f);
            mAnimationSettingsDirty = true;
        }

        if (!mMousePressed)
        {
            mDraggingAnimSlider = -1;
        }
    }

    if (mSelectedAnimColor >= 0 && mMousePressed)
    {
        sf::Color *target = (mSelectedAnimColor == 0) ? &mSettings.lightAnimColor : &mSettings.keyPressVisColor;
        sf::FloatRect sliderRects[3];
        const float barWidth = 200.f;
        const float barHeight = 12.f;
        const float spacing = 35.f;
        sf::Vector2f sliderPos(100.f, 580.f);
        for (int i = 0; i < 3; ++i)
        {
            sliderRects[i] = sf::FloatRect(sliderPos.x + 30.f, sliderPos.y + static_cast<float>(i) * spacing + 10.f, barWidth, barHeight);
        }
        handleColorSliders(*target, sliderPos, sliderRects);
    }
}

void StyleWizard::handleWindowSettingsStep()
{
    if (mMousePressed && isButtonHovered(mMousePos, sf::FloatRect(150, 210, 20, 20)))
    {
        mSettings.windowTitleBar = !mSettings.windowTitleBar;
        mMousePressed = false;
    }
    if (mMousePressed && isButtonHovered(mMousePos, sf::FloatRect(150, 260, 20, 20)))
    {
        mSettings.windowResizable = !mSettings.windowResizable;
        mMousePressed = false;
    }

    handleNumericSlider(mSettings.windowBonusSizeTop, -500, 500, sf::Vector2f(150, 330));
    handleNumericSlider(mSettings.windowBonusSizeBottom, -500, 500, sf::Vector2f(150, 380));
    handleNumericSlider(mSettings.windowBonusSizeLeft, -500, 500, sf::Vector2f(150, 430));
    handleNumericSlider(mSettings.windowBonusSizeRight, -500, 500, sf::Vector2f(150, 480));

    if (mMousePressed && isButtonHovered(mMousePos, sf::FloatRect(80, 730, 120, 40)))
    {
        mCurrentStep = Step::AnimationSettings;
        mMousePressed = false;
    }
    if (mMousePressed && isButtonHovered(mMousePos, sf::FloatRect(220, 730, 120, 40)))
    {
        closeWindow();
        mMousePressed = false;
    }
    if (mMousePressed && isButtonHovered(mMousePos, sf::FloatRect(360, 730, 120, 40)))
    {
        resetToOriginal();
        mMousePressed = false;
    }
    if (mMousePressed && isButtonHovered(mMousePos, sf::FloatRect(700, 730, 120, 40)))
    {
        mCurrentStep = Step::ReviewApply;
        mMousePressed = false;
    }
}

void StyleWizard::handleReviewApplyStep()
{
    if (mMousePressed)
    {
        sf::FloatRect applyButton(700, 730, 120, 40);
        if (isButtonHovered(mMousePos, applyButton))
        {
            saveConfig();
            mApplyRequested = true;
            mStatusMessage = "Settings saved successfully!";
            mStatusTimer = 3.f;
            mMousePressed = false;
        }

        sf::FloatRect backButton(80, 730, 120, 40);
        if (mMousePressed && isButtonHovered(mMousePos, sf::FloatRect(80, 730, 120, 40)))
        {
            mCurrentStep = Step::WindowSettings;
            mMousePressed = false;
        }

        sf::FloatRect cancelButton(220, 730, 120, 40);
        if (isButtonHovered(mMousePos, cancelButton))
        {
            closeWindow();
            mMousePressed = false;
        }

        sf::FloatRect resetButton(360, 730, 120, 40);
        if (isButtonHovered(mMousePos, resetButton))
        {
            resetToOriginal();
            mMousePressed = false;
        }
    }
}

 void StyleWizard::applyPreset(const std::string &presetName)
{
    for (const auto &preset : mPresets)
    {
        if (preset.name == presetName)
        {
            mSettings.buttonColor = preset.buttonColor;
            mSettings.textColor = preset.textColor;
            mSettings.backgroundColor = preset.backgroundColor;
            mSettings.borderColor = preset.borderColor;
            mSettings.lightAnimation = preset.lightAnimation;
            mSettings.pressAnimation = preset.pressAnimation;
            mSettings.animationScale = preset.animationScale;
            mSettings.lightAnimScale = preset.lightAnimScale;
            mSettings.lightAnimColor = preset.lightAnimColor;
            mSettings.pressAnimOffset = preset.pressAnimOffset;
            mSettings.keyPressVisEnabled = preset.keyPressVisEnabled;
            mSettings.keyPressVisSpeed = preset.keyPressVisSpeed;
            mSettings.keyPressVisRotation = preset.keyPressVisRotation;
            mSettings.keyPressVisFadeDistance = preset.keyPressVisFadeDistance;
            mSettings.keyPressVisColor = preset.keyPressVisColor;
            mSettings.keyPressVisBonusWidth = preset.keyPressVisBonusWidth;
            mSettings.keyPressVisFixedHeight = preset.keyPressVisFixedHeight;
            mSettings.buttonShape = preset.buttonShape;
            mSettings.animationStyle = preset.animationStyle;
            mSettings.textOutlineColor = preset.textOutlineColor;
            mSettings.textOutlineThickness = preset.textOutlineThickness;
            mSettings.textSize = preset.textSize;

            mSettings.windowTitleBar = Settings::WindowTitleBar;
            mSettings.windowResizable = Settings::WindowResizable;
            mSettings.windowBonusSizeTop = Settings::WindowBonusSizeTop;
            mSettings.windowBonusSizeBottom = Settings::WindowBonusSizeBottom;
            mSettings.windowBonusSizeLeft = Settings::WindowBonusSizeLeft;
            mSettings.windowBonusSizeRight = Settings::WindowBonusSizeRight;

            mSelectedButtonShape = mSettings.buttonShape;
            mSelectedAnimStyle = mSettings.animationStyle;
            break;
        }
    }
}

void StyleWizard::applyAnimationStyle(int style)
{
    switch (style)
    {
        case 0:
            mSettings.lightAnimation = true;
            mSettings.pressAnimation = false;
            mSettings.animationScale = 1.0f;
            mSettings.lightAnimScale = 100;
            mSettings.pressAnimOffset = 3;
            break;
        case 1:
            mSettings.lightAnimation = true;
            mSettings.pressAnimation = false;
            mSettings.animationScale = 1.5f;
            mSettings.lightAnimScale = 120;
            mSettings.pressAnimOffset = 3;
            break;
        case 2:
            mSettings.lightAnimation = false;
            mSettings.pressAnimation = true;
            mSettings.animationScale = 1.2f;
            mSettings.lightAnimScale = 100;
            mSettings.pressAnimOffset = 8;
            break;
        case 3:
            mSettings.lightAnimation = true;
            mSettings.pressAnimation = true;
            mSettings.animationScale = 0.8f;
            mSettings.lightAnimScale = 80;
            mSettings.pressAnimOffset = 2;
            break;
        case 4:
            mSettings.lightAnimation = true;
            mSettings.pressAnimation = false;
            mSettings.animationScale = 1.3f;
            mSettings.lightAnimScale = 150;
            mSettings.pressAnimOffset = 3;
            break;
        case 5:
            mSettings.lightAnimation = false;
            mSettings.pressAnimation = true;
            mSettings.animationScale = 1.0f;
            mSettings.lightAnimScale = 100;
            mSettings.pressAnimOffset = 12;
            break;
        case 6:
            mSettings.lightAnimation = true;
            mSettings.pressAnimation = false;
            mSettings.animationScale = 2.0f;
            mSettings.lightAnimScale = 180;
            mSettings.pressAnimOffset = 3;
            break;
    }
}

void StyleWizard::saveConfig()
{
    std::ifstream inFile("JKPS.cfg");
    std::vector<std::string> lines;
    std::string line;

    while (std::getline(inFile, line))
    {
        lines.push_back(line);
    }
    inFile.close();

    std::string currentSection;
    for (auto &l : lines)
    {
        if (l.size() > 2 && l.front() == '[' && l.back() == ']')
        {
            currentSection = l;
            continue;
        }

        if (currentSection == "[Button graphics]" && l.find("Texture color:") != std::string::npos)
        {
            l = "Texture color: " + std::to_string(static_cast<int>(mSettings.buttonColor.r)) + "," +
                std::to_string(static_cast<int>(mSettings.buttonColor.g)) + "," +
                std::to_string(static_cast<int>(mSettings.buttonColor.b)) + "," +
                std::to_string(static_cast<int>(mSettings.buttonColor.a));
        }
        else if (currentSection == "[Buttons text]" && l.find("Text color:") != std::string::npos)
        {
            l = "Text color: " + std::to_string(static_cast<int>(mSettings.textColor.r)) + "," +
                std::to_string(static_cast<int>(mSettings.textColor.g)) + "," +
                std::to_string(static_cast<int>(mSettings.textColor.b)) + "," +
                std::to_string(static_cast<int>(mSettings.textColor.a));
        }
        else if (currentSection == "[Main window]" && l.find("Background color:") != std::string::npos)
        {
            l = "Background color: " + std::to_string(static_cast<int>(mSettings.backgroundColor.r)) + "," +
                std::to_string(static_cast<int>(mSettings.backgroundColor.g)) + "," +
                std::to_string(static_cast<int>(mSettings.backgroundColor.b)) + "," +
                std::to_string(static_cast<int>(mSettings.backgroundColor.a));
        }
        else if (currentSection == "[Common parameters]" && l.find("Animation duration (frames):") != std::string::npos)
        {
            l = "Animation duration (frames): " + std::to_string(static_cast<int>(mSettings.animationScale * 5));
        }
        else if (currentSection == "[Press animation]" && l.find("Enabled:") != std::string::npos)
        {
            l = "Enabled: " + std::string(mSettings.pressAnimation ? "True" : "False");
        }
        else if (currentSection == "[Light animation]" && l.find("Enabled:") != std::string::npos)
        {
            l = "Enabled: " + std::string(mSettings.lightAnimation ? "True" : "False");
        }
        else if (currentSection == "[Light animation]" && l.find("Scale on click (%):") != std::string::npos)
        {
            l = "Scale on click (%): " + std::to_string(mSettings.lightAnimScale) + "," + std::to_string(mSettings.lightAnimScale);
        }
        else if (currentSection == "[Light animation]" && l.find("Color:") != std::string::npos)
        {
            l = "Color: " + std::to_string(static_cast<int>(mSettings.lightAnimColor.r)) + "," +
                std::to_string(static_cast<int>(mSettings.lightAnimColor.g)) + "," +
                std::to_string(static_cast<int>(mSettings.lightAnimColor.b)) + "," +
                std::to_string(static_cast<int>(mSettings.lightAnimColor.a));
        }
        else if (currentSection == "[Press animation]" && l.find("Offset:") != std::string::npos)
        {
            l = "Offset: " + std::to_string(mSettings.pressAnimOffset);
        }
        else if (currentSection == "[Key press visualization]" && l.find("Enabled:") != std::string::npos)
        {
            l = "Enabled: " + std::string(mSettings.keyPressVisEnabled ? "True" : "False");
        }
        else if (currentSection == "[Key press visualization]" && l.find("Speed:") != std::string::npos)
        {
            l = "Speed: " + std::to_string(mSettings.keyPressVisSpeed);
        }
        else if (currentSection == "[Key press visualization]" && l.find("Movement rotation:") != std::string::npos)
        {
            l = "Movement rotation: " + std::to_string(mSettings.keyPressVisRotation);
        }
        else if (currentSection == "[Key press visualization]" && l.find("Fade out distance:") != std::string::npos)
        {
            l = "Fade out distance: " + std::to_string(mSettings.keyPressVisFadeDistance);
        }
        else if (currentSection == "[Key press visualization]" && l.find("Color:") != std::string::npos)
        {
            l = "Color: " + std::to_string(static_cast<int>(mSettings.keyPressVisColor.r)) + "," +
                std::to_string(static_cast<int>(mSettings.keyPressVisColor.g)) + "," +
                std::to_string(static_cast<int>(mSettings.keyPressVisColor.b)) + "," +
                std::to_string(static_cast<int>(mSettings.keyPressVisColor.a));
        }
        else if (currentSection == "[Key press visualization]" && l.find("Bonus width scale (%):") != std::string::npos)
        {
            l = "Bonus width scale (%): " + std::to_string(mSettings.keyPressVisBonusWidth);
        }
        else if (currentSection == "[Key press visualization]" && l.find("Fixed height to replace holds:") != std::string::npos)
        {
            l = "Fixed height to replace holds: " + std::to_string(mSettings.keyPressVisFixedHeight);
        }
        else if (currentSection == "[Button graphics]" && l.find("Button shape:") != std::string::npos)
        {
            l = "Button shape: " + std::to_string(mSettings.buttonShape);
        }
        else if (currentSection == "[Button graphics]" && l.find("Border color:") != std::string::npos)
        {
            l = "Border color: " + std::to_string(static_cast<int>(mSettings.borderColor.r)) + "," +
                std::to_string(static_cast<int>(mSettings.borderColor.g)) + "," +
                std::to_string(static_cast<int>(mSettings.borderColor.b)) + "," +
                std::to_string(static_cast<int>(mSettings.borderColor.a));
        }
        else if (currentSection == "[Buttons text]" && l.find("Outline color:") != std::string::npos)
        {
            l = "Outline color: " + std::to_string(static_cast<int>(mSettings.textOutlineColor.r)) + "," +
                std::to_string(static_cast<int>(mSettings.textOutlineColor.g)) + "," +
                std::to_string(static_cast<int>(mSettings.textOutlineColor.b)) + "," +
                std::to_string(static_cast<int>(mSettings.textOutlineColor.a));
        }
        else if (currentSection == "[Buttons text]" && l.find("Outline thickness:") != std::string::npos)
        {
            l = "Outline thickness: " + std::to_string(mSettings.textOutlineThickness);
        }
        else if (currentSection == "[Buttons text]" && l.find("Character size:") != std::string::npos)
        {
            l = "Character size: " + std::to_string(mSettings.textSize);
        }
        else if (currentSection == "[Common parameters]" && l.find("Animation style:") != std::string::npos)
        {
            l = "Animation style: " + std::to_string(mSettings.animationStyle);
        }
    }

    std::ofstream outFile("JKPS.cfg");
    for (const auto &l : lines)
    {
        outFile << l << "\n";
    }
    outFile.close();
}

void StyleWizard::resetToOriginal()
{
    mSettings = mOriginalSettings;
    mSelectedButtonShape = mSettings.buttonShape;
    mSelectedAnimStyle = mSettings.animationStyle;
    mColorCustomizationDirty = false;
    mAnimationSettingsDirty = false;
    mStatusMessage = "Settings reset to original";
    mStatusTimer = 3.f;
}

void StyleWizard::loadConfig()
{
    std::ifstream inFile("JKPS.cfg");
    if (!inFile.is_open())
        return;

    std::string line;
    std::string currentSection;
    while (std::getline(inFile, line))
    {
        if (line.size() > 2 && line.front() == '[' && line.back() == ']')
        {
            currentSection = line;
            continue;
        }

        if (currentSection == "[Button graphics]" && line.find("Texture color:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                auto values = Utility::split(parts[1], ',');
                if (values.size() == 4)
                {
                    mSettings.buttonColor = sf::Color(
                        static_cast<sf::Uint8>(std::stoi(values[0])),
                        static_cast<sf::Uint8>(std::stoi(values[1])),
                        static_cast<sf::Uint8>(std::stoi(values[2])),
                        static_cast<sf::Uint8>(std::stoi(values[3]))
                    );
                    mOriginalSettings.buttonColor = mSettings.buttonColor;
                }
            }
        }
        else if (currentSection == "[Buttons text]" && line.find("Text color:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                auto values = Utility::split(parts[1], ',');
                if (values.size() == 4)
                {
                    mSettings.textColor = sf::Color(
                        static_cast<sf::Uint8>(std::stoi(values[0])),
                        static_cast<sf::Uint8>(std::stoi(values[1])),
                        static_cast<sf::Uint8>(std::stoi(values[2])),
                        static_cast<sf::Uint8>(std::stoi(values[3]))
                    );
                    mOriginalSettings.textColor = mSettings.textColor;
                }
            }
        }
        else if (currentSection == "[Main window]" && line.find("Background color:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                auto values = Utility::split(parts[1], ',');
                if (values.size() == 4)
                {
                    mSettings.backgroundColor = sf::Color(
                        static_cast<sf::Uint8>(std::stoi(values[0])),
                        static_cast<sf::Uint8>(std::stoi(values[1])),
                        static_cast<sf::Uint8>(std::stoi(values[2])),
                        static_cast<sf::Uint8>(std::stoi(values[3]))
                    );
                    mOriginalSettings.backgroundColor = mSettings.backgroundColor;
                }
            }
        }
        else if (currentSection == "[Common parameters]" && line.find("Animation duration (frames):") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                int frames = std::stoi(parts[1]);
                mSettings.animationScale = static_cast<float>(frames) / 5.0f;
                mOriginalSettings.animationScale = mSettings.animationScale;
            }
        }
        else if (currentSection == "[Press animation]" && line.find("Enabled:") != std::string::npos)
        {
            if (line.find("True") != std::string::npos)
                mSettings.pressAnimation = true;
            else if (line.find("False") != std::string::npos)
                mSettings.pressAnimation = false;
            mOriginalSettings.pressAnimation = mSettings.pressAnimation;
        }
        else if (currentSection == "[Light animation]" && line.find("Enabled:") != std::string::npos)
        {
            if (line.find("True") != std::string::npos)
                mSettings.lightAnimation = true;
            else if (line.find("False") != std::string::npos)
                mSettings.lightAnimation = false;
            mOriginalSettings.lightAnimation = mSettings.lightAnimation;
        }
        else if (currentSection == "[Light animation]" && line.find("Scale on click (%):") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                auto values = Utility::split(parts[1], ',');
                if (values.size() >= 1)
                {
                    mSettings.lightAnimScale = std::stoi(values[0]);
                    mOriginalSettings.lightAnimScale = mSettings.lightAnimScale;
                }
            }
        }
        else if (currentSection == "[Light animation]" && line.find("Color:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                auto values = Utility::split(parts[1], ',');
                if (values.size() == 4)
                {
                    mSettings.lightAnimColor = sf::Color(
                        static_cast<sf::Uint8>(std::stoi(values[0])),
                        static_cast<sf::Uint8>(std::stoi(values[1])),
                        static_cast<sf::Uint8>(std::stoi(values[2])),
                        static_cast<sf::Uint8>(std::stoi(values[3]))
                    );
                    mOriginalSettings.lightAnimColor = mSettings.lightAnimColor;
                }
            }
        }
        else if (currentSection == "[Press animation]" && line.find("Enabled:") != std::string::npos)
        {
            if (line.find("True") != std::string::npos)
                mSettings.pressAnimation = true;
            else if (line.find("False") != std::string::npos)
                mSettings.pressAnimation = false;
            mOriginalSettings.pressAnimation = mSettings.pressAnimation;
        }
        else if (currentSection == "[Press animation]" && line.find("Offset:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.pressAnimOffset = std::stoi(parts[1]);
                mOriginalSettings.pressAnimOffset = mSettings.pressAnimOffset;
            }
        }
        else if (currentSection == "[Key press visualization]" && line.find("Enabled:") != std::string::npos)
        {
            if (line.find("True") != std::string::npos)
                mSettings.keyPressVisEnabled = true;
            else if (line.find("False") != std::string::npos)
                mSettings.keyPressVisEnabled = false;
            mOriginalSettings.keyPressVisEnabled = mSettings.keyPressVisEnabled;
        }
        else if (currentSection == "[Key press visualization]" && line.find("Speed:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.keyPressVisSpeed = std::stoi(parts[1]);
                mOriginalSettings.keyPressVisSpeed = mSettings.keyPressVisSpeed;
            }
        }
        else if (currentSection == "[Key press visualization]" && line.find("Movement rotation:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.keyPressVisRotation = std::stoi(parts[1]);
                mOriginalSettings.keyPressVisRotation = mSettings.keyPressVisRotation;
            }
        }
        else if (currentSection == "[Key press visualization]" && line.find("Fade out distance:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.keyPressVisFadeDistance = std::stoi(parts[1]);
                mOriginalSettings.keyPressVisFadeDistance = mSettings.keyPressVisFadeDistance;
            }
        }
        else if (currentSection == "[Key press visualization]" && line.find("Color:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                auto values = Utility::split(parts[1], ',');
                if (values.size() == 4)
                {
                    mSettings.keyPressVisColor = sf::Color(
                        static_cast<sf::Uint8>(std::stoi(values[0])),
                        static_cast<sf::Uint8>(std::stoi(values[1])),
                        static_cast<sf::Uint8>(std::stoi(values[2])),
                        static_cast<sf::Uint8>(std::stoi(values[3]))
                    );
                    mOriginalSettings.keyPressVisColor = mSettings.keyPressVisColor;
                }
            }
        }
        else if (currentSection == "[Key press visualization]" && line.find("Bonus width scale (%):") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.keyPressVisBonusWidth = std::stoi(parts[1]);
                mOriginalSettings.keyPressVisBonusWidth = mSettings.keyPressVisBonusWidth;
            }
        }
        else if (currentSection == "[Key press visualization]" && line.find("Fixed height to replace holds:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.keyPressVisFixedHeight = std::stoi(parts[1]);
                mOriginalSettings.keyPressVisFixedHeight = mSettings.keyPressVisFixedHeight;
            }
        }
        else if (currentSection == "[Button graphics]" && line.find("Button shape:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.buttonShape = std::stoi(parts[1]);
                mOriginalSettings.buttonShape = mSettings.buttonShape;
                mSelectedButtonShape = mSettings.buttonShape;
            }
        }
        else if (currentSection == "[Button graphics]" && line.find("Border color:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                auto values = Utility::split(parts[1], ',');
                if (values.size() == 4)
                {
                    mSettings.borderColor = sf::Color(
                        static_cast<sf::Uint8>(std::stoi(values[0])),
                        static_cast<sf::Uint8>(std::stoi(values[1])),
                        static_cast<sf::Uint8>(std::stoi(values[2])),
                        static_cast<sf::Uint8>(std::stoi(values[3]))
                    );
                    mOriginalSettings.borderColor = mSettings.borderColor;
                }
            }
        }
        else if (currentSection == "[Buttons text]" && line.find("Outline color:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                auto values = Utility::split(parts[1], ',');
                if (values.size() == 4)
                {
                    mSettings.textOutlineColor = sf::Color(
                        static_cast<sf::Uint8>(std::stoi(values[0])),
                        static_cast<sf::Uint8>(std::stoi(values[1])),
                        static_cast<sf::Uint8>(std::stoi(values[2])),
                        static_cast<sf::Uint8>(std::stoi(values[3]))
                    );
                    mOriginalSettings.textOutlineColor = mSettings.textOutlineColor;
                }
            }
        }
        else if (currentSection == "[Buttons text]" && line.find("Outline thickness:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.textOutlineThickness = std::stoi(parts[1]);
                mOriginalSettings.textOutlineThickness = mSettings.textOutlineThickness;
            }
        }
        else if (currentSection == "[Buttons text]" && line.find("Character size:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.textSize = std::stoi(parts[1]);
                mOriginalSettings.textSize = mSettings.textSize;
            }
        }
        else if (currentSection == "[Common parameters]" && line.find("Animation style:") != std::string::npos)
        {
            auto parts = Utility::split(line, ':');
            if (parts.size() == 2)
            {
                mSettings.animationStyle = std::stoi(parts[1]);
                mOriginalSettings.animationStyle = mSettings.animationStyle;
                mSelectedAnimStyle = mSettings.animationStyle;
            }
        }
    }
    inFile.close();
}

bool StyleWizard::isButtonHovered(sf::Vector2i mousePos, const sf::FloatRect &bounds) const
{
    return bounds.contains(sf::Vector2f(mousePos));
}

void StyleWizard::drawButton(const std::string &text, sf::Vector2f position, sf::Vector2f size, bool hovered, int shape)
{
    sf::Color baseColor = hovered ? sf::Color(70, 130, 180, 255) : sf::Color(50, 50, 50, 255);
    sf::Color outlineColor = hovered ? sf::Color(100, 180, 255, 255) : sf::Color(80, 80, 80, 255);

    if (shape == 0)
    {
        sf::RectangleShape button;
        button.setSize(size);
        button.setPosition(position);
        button.setFillColor(baseColor);
        button.setOutlineThickness(1.5f);
        button.setOutlineColor(outlineColor);
        mWindow.draw(button);
    }
    else if (shape == 1)
    {
        sf::RectangleShape button;
        button.setSize(sf::Vector2f(size.x - 4.f, size.y - 4.f));
        button.setPosition(position + sf::Vector2f(2.f, 2.f));
        button.setFillColor(baseColor);
        button.setOutlineThickness(1.5f);
        button.setOutlineColor(outlineColor);
        mWindow.draw(button);
    }
    else if (shape == 2)
    {
        float radius = size.y / 2.f;
        sf::RectangleShape button;
        button.setSize(sf::Vector2f(size.x, size.y));
        button.setPosition(position);
        button.setFillColor(baseColor);
        button.setOutlineThickness(1.5f);
        button.setOutlineColor(outlineColor);
        mWindow.draw(button);

        sf::CircleShape leftCap(radius);
        leftCap.setPosition(position.x - radius, position.y);
        leftCap.setFillColor(baseColor);
        mWindow.draw(leftCap);

        sf::CircleShape rightCap(radius);
        rightCap.setPosition(position.x + size.x - radius, position.y);
        rightCap.setFillColor(baseColor);
        mWindow.draw(rightCap);
    }
    else if (shape == 3)
    {
        float radius = std::min(size.x, size.y) / 2.f;
        sf::CircleShape button(radius);
        button.setPosition(position.x + size.x / 2.f - radius, position.y + size.y / 2.f - radius);
        button.setFillColor(baseColor);
        button.setOutlineThickness(1.5f);
        button.setOutlineColor(outlineColor);
        mWindow.draw(button);
    }
    else if (shape == 4)
    {
        sf::ConvexShape diamond;
        diamond.setPointCount(4);
        diamond.setPoint(0, sf::Vector2f(position.x + size.x / 2.f, position.y));
        diamond.setPoint(1, sf::Vector2f(position.x + size.x, position.y + size.y / 2.f));
        diamond.setPoint(2, sf::Vector2f(position.x + size.x / 2.f, position.y + size.y));
        diamond.setPoint(3, sf::Vector2f(position.x, position.y + size.y / 2.f));
        diamond.setFillColor(baseColor);
        diamond.setOutlineThickness(1.5f);
        diamond.setOutlineColor(outlineColor);
        mWindow.draw(diamond);
    }
    else if (shape == 5)
    {
        sf::ConvexShape star;
        star.setPointCount(10);
        float cx = position.x + size.x / 2.f;
        float cy = position.y + size.y / 2.f;
        float outerR = std::min(size.x, size.y) / 2.f;
        float innerR = outerR * 0.4f;
        for (std::size_t i = 0; i < 5; ++i)
        {
            float angle = -3.14159f / 2.f + static_cast<float>(i) * 2.f * 3.14159f / 5.f;
            star.setPoint(i, sf::Vector2f(cx + outerR * cosf(angle), cy + outerR * sinf(angle)));
        }
        for (std::size_t i = 0; i < 5; ++i)
        {
            float angle = -3.14159f / 2.f + (static_cast<float>(i) + 0.5f) * 2.f * 3.14159f / 5.f;
            star.setPoint(i + 5, sf::Vector2f(cx + innerR * cosf(angle), cy + innerR * sinf(angle)));
        }
        star.setFillColor(baseColor);
        star.setOutlineThickness(1.5f);
        star.setOutlineColor(outlineColor);
        mWindow.draw(star);
    }
    else if (shape == 6)
    {
        sf::ConvexShape hex;
        hex.setPointCount(6);
        float cx = position.x + size.x / 2.f;
        float cy = position.y + size.y / 2.f;
        float radius = std::min(size.x, size.y) / 2.f;
        for (std::size_t i = 0; i < 6; ++i)
        {
            float angle = static_cast<float>(i) * 3.14159f / 3.f;
            hex.setPoint(i, sf::Vector2f(cx + radius * cosf(angle), cy + radius * sinf(angle)));
        }
        hex.setFillColor(baseColor);
        hex.setOutlineThickness(1.5f);
        hex.setOutlineColor(outlineColor);
        mWindow.draw(hex);
    }

    sf::Text buttonText;
    buttonText.setFont(getFont());
    buttonText.setString(text);
    buttonText.setCharacterSize(18);
    buttonText.setFillColor(sf::Color::White);
    buttonText.setPosition(position.x + size.x / 2 - buttonText.getLocalBounds().width / 2,
                          position.y + size.y / 2 - buttonText.getLocalBounds().height / 2);
    mWindow.draw(buttonText);
}

const sf::Font &StyleWizard::getFont() const
{
    return mFontHolder->get(Fonts::ButtonValue);
}

void StyleWizard::drawToggle(const std::string &label, bool value, sf::Vector2f position)
{
    sf::RectangleShape box;
    box.setSize(sf::Vector2f(24.f, 24.f));
    box.setPosition(position);
    box.setFillColor(value ? sf::Color(100, 180, 100, 255) : sf::Color(60, 60, 60, 255));
    box.setOutlineThickness(2.f);
    box.setOutlineColor(value ? sf::Color(150, 220, 150, 255) : sf::Color(100, 100, 100, 255));
    mWindow.draw(box);

    if (value)
    {
        sf::RectangleShape inner;
        inner.setSize(sf::Vector2f(12.f, 12.f));
        inner.setPosition(position.x + 6.f, position.y + 6.f);
        inner.setFillColor(sf::Color::White);
        mWindow.draw(inner);
    }

    sf::Text labelText;
    labelText.setFont(getFont());
    labelText.setString(label);
    labelText.setCharacterSize(20);
    labelText.setFillColor(sf::Color::White);
    labelText.setPosition(position.x + 34, position.y - 2);
    mWindow.draw(labelText);
}

void StyleWizard::drawColorSwatch(sf::Color color, sf::Vector2f position, float size)
{
    sf::RectangleShape swatch;
    swatch.setSize(sf::Vector2f(size, size));
    swatch.setPosition(position);
    swatch.setFillColor(color);
    swatch.setOutlineThickness(1.f);
    swatch.setOutlineColor(sf::Color::White);
    mWindow.draw(swatch);
}

void StyleWizard::drawStatusMessage()
{
    if (mStatusTimer > 0.f && !mStatusMessage.empty())
    {
        sf::Text status;
        status.setFont(getFont());
        status.setString(mStatusMessage);
        status.setCharacterSize(20);
        status.setFillColor(sf::Color(100, 255, 100, 255));
        status.setPosition(450.f - status.getLocalBounds().width / 2.f, 680.f);
        mWindow.draw(status);
    }
}

void StyleWizard::drawStepIndicator()
{
    const std::vector<std::string> labels = {
        "Welcome",
        "Presets",
        "Colors",
        "Animations",
        "Window Settings",
        "Review"
    };

    const float totalWidth = 600.f;
    const float startX = 450.f - totalWidth / 2.f;
    const float yPos = 30.f;
    const int totalSteps = 6;
    const float stepWidth = totalWidth / (totalSteps - 1);

    int currentStepIdx = 0;
    if (mCurrentStep == Step::Welcome) currentStepIdx = 0;
    else if (mCurrentStep == Step::PresetSelection) currentStepIdx = 1;
    else if (mCurrentStep == Step::ColorCustomization) currentStepIdx = 2;
    else if (mCurrentStep == Step::AnimationSettings) currentStepIdx = 3;
    else if (mCurrentStep == Step::WindowSettings) currentStepIdx = 4;
    else if (mCurrentStep == Step::ReviewApply) currentStepIdx = 5;

    for (std::size_t i = 0; i < static_cast<std::size_t>(totalSteps); ++i)
    {
        sf::CircleShape dot(6.f);
        dot.setPosition(startX + static_cast<float>(i) * stepWidth - 6.f, yPos);
        dot.setFillColor(static_cast<int>(i) == currentStepIdx ? sf::Color(100, 180, 255, 255) : sf::Color(80, 80, 80, 255));
        mWindow.draw(dot);

        sf::Text label;
        label.setFont(getFont());
        label.setString(labels[i]);
        label.setCharacterSize(14);
        label.setFillColor(static_cast<int>(i) == currentStepIdx ? sf::Color::White : sf::Color(120, 120, 120));
        label.setPosition(startX + static_cast<float>(i) * stepWidth - label.getLocalBounds().width / 2.f, yPos + 14.f);
        mWindow.draw(label);
    }
}

void StyleWizard::drawPreviewButton(sf::Color buttonColor, sf::Color textColor, sf::Vector2f position)
{
    sf::Vector2f size(120.f, 50.f);
    sf::Color baseColor = buttonColor;
    sf::Color outlineColor = mSettings.borderColor;

    if (mSettings.buttonShape == 0)
    {
        sf::RectangleShape preview;
        preview.setSize(size);
        preview.setPosition(position);
        preview.setFillColor(baseColor);
        preview.setOutlineThickness(2.f);
        preview.setOutlineColor(outlineColor);
        mWindow.draw(preview);
    }
    else if (mSettings.buttonShape == 1)
    {
        sf::RectangleShape preview;
        preview.setSize(sf::Vector2f(size.x - 4.f, size.y - 4.f));
        preview.setPosition(position + sf::Vector2f(2.f, 2.f));
        preview.setFillColor(baseColor);
        preview.setOutlineThickness(2.f);
        preview.setOutlineColor(outlineColor);
        mWindow.draw(preview);
    }
    else if (mSettings.buttonShape == 2)
    {
        float radius = size.y / 2.f;
        sf::RectangleShape preview;
        preview.setSize(size);
        preview.setPosition(position);
        preview.setFillColor(baseColor);
        preview.setOutlineThickness(2.f);
        preview.setOutlineColor(outlineColor);
        mWindow.draw(preview);

        sf::CircleShape leftCap(radius);
        leftCap.setPosition(position.x - radius, position.y);
        leftCap.setFillColor(baseColor);
        mWindow.draw(leftCap);

        sf::CircleShape rightCap(radius);
        rightCap.setPosition(position.x + size.x - radius, position.y);
        rightCap.setFillColor(baseColor);
        mWindow.draw(rightCap);
    }
    else if (mSettings.buttonShape == 3)
    {
        float radius = std::min(size.x, size.y) / 2.f;
        sf::CircleShape preview(radius);
        preview.setPosition(position.x + size.x / 2.f - radius, position.y + size.y / 2.f - radius);
        preview.setFillColor(baseColor);
        preview.setOutlineThickness(2.f);
        preview.setOutlineColor(outlineColor);
        mWindow.draw(preview);
    }
    else if (mSettings.buttonShape == 4) // Diamond
    {
        sf::ConvexShape preview;
        preview.setPointCount(4);
        preview.setPoint(0, sf::Vector2f(position.x + size.x / 2.f, position.y));
        preview.setPoint(1, sf::Vector2f(position.x + size.x, position.y + size.y / 2.f));
        preview.setPoint(2, sf::Vector2f(position.x + size.x / 2.f, position.y + size.y));
        preview.setPoint(3, sf::Vector2f(position.x, position.y + size.y / 2.f));
        preview.setFillColor(baseColor);
        preview.setOutlineThickness(2.f);
        preview.setOutlineColor(outlineColor);
        mWindow.draw(preview);
    }
    else if (mSettings.buttonShape == 5) // Star
    {
        float cx = position.x + size.x / 2.f;
        float cy = position.y + size.y / 2.f;
        float outerR = std::min(size.x, size.y) / 2.f;
        float innerR = outerR * 0.4f;
        sf::VertexArray preview(sf::TriangleFan, 12);
        preview[0].position = {cx, cy};
        preview[0].color = baseColor;
        for (std::size_t i = 0; i < 5; ++i)
        {
            float angle = -3.14159f / 2.f + static_cast<float>(i) * 2.f * 3.14159f / 5.f;
            const auto outerIdx = i * 2 + 1;
            const auto innerIdx = i * 2 + 2;
            preview[outerIdx].position = {cx + outerR * cosf(angle), cy + outerR * sinf(angle)};
            preview[outerIdx].color = baseColor;
            float innerAngle = angle + 3.14159f / 5.f;
            preview[innerIdx].position = {cx + innerR * cosf(innerAngle), cy + innerR * sinf(innerAngle)};
            preview[innerIdx].color = baseColor;
        }
        mWindow.draw(preview);

        sf::VertexArray outline(sf::LineStrip, 11);
        for (std::size_t i = 0; i < 5; ++i)
        {
            float angle = -3.14159f / 2.f + static_cast<float>(i) * 2.f * 3.14159f / 5.f;
            const auto outerIdx = i * 2;
            const auto innerIdx = i * 2 + 1;
            outline[outerIdx].position = {cx + outerR * cosf(angle), cy + outerR * sinf(angle)};
            outline[outerIdx].color = outlineColor;
            float innerAngle = angle + 3.14159f / 5.f;
            outline[innerIdx].position = {cx + innerR * cosf(innerAngle), cy + innerR * sinf(innerAngle)};
            outline[innerIdx].color = outlineColor;
        }
        outline[10].position = outline[0].position;
        outline[10].color = outlineColor;
        mWindow.draw(outline);
    }
    else if (mSettings.buttonShape == 6) // Hexagon
    {
        sf::ConvexShape preview;
        preview.setPointCount(6);
        float cx = position.x + size.x / 2.f;
        float cy = position.y + size.y / 2.f;
        float radius = std::min(size.x, size.y) / 2.f;
        for (std::size_t i = 0; i < 6; ++i)
        {
            float angle = static_cast<float>(i) * 3.14159f / 3.f;
            preview.setPoint(i, sf::Vector2f(cx + radius * cosf(angle), cy + radius * sinf(angle)));
        }
        preview.setFillColor(baseColor);
        preview.setOutlineThickness(2.f);
        preview.setOutlineColor(outlineColor);
        mWindow.draw(preview);
    }
    else if (mSettings.buttonShape == 7) // Triangle
    {
        sf::ConvexShape preview;
        preview.setPointCount(3);
        float cx = position.x + size.x / 2.f;
        float cy = position.y + size.y / 2.f;
        float radius = std::min(size.x, size.y) / 2.f;
        preview.setPoint(0, sf::Vector2f(cx, cy - radius));
        preview.setPoint(1, sf::Vector2f(cx + radius * 0.866f, cy + radius * 0.5f));
        preview.setPoint(2, sf::Vector2f(cx - radius * 0.866f, cy + radius * 0.5f));
        preview.setFillColor(baseColor);
        preview.setOutlineThickness(2.f);
        preview.setOutlineColor(outlineColor);
        mWindow.draw(preview);
    }
    else if (mSettings.buttonShape == 8) // Octagon
    {
        sf::ConvexShape preview;
        preview.setPointCount(8);
        float cx = position.x + size.x / 2.f;
        float cy = position.y + size.y / 2.f;
        float radius = std::min(size.x, size.y) / 2.f;
        for (std::size_t i = 0; i < 8; ++i)
        {
            float angle = static_cast<float>(i) * 3.14159f / 4.f + 3.14159f / 8.f;
            preview.setPoint(i, sf::Vector2f(cx + radius * cosf(angle), cy + radius * sinf(angle)));
        }
        preview.setFillColor(baseColor);
        preview.setOutlineThickness(2.f);
        preview.setOutlineColor(outlineColor);
        mWindow.draw(preview);
    }

    sf::Text previewText;
    previewText.setFont(getFont());
    previewText.setString("Preview");
    previewText.setCharacterSize(static_cast<unsigned int>(mSettings.textSize));
    previewText.setFillColor(textColor);
    previewText.setOutlineThickness(static_cast<float>(mSettings.textOutlineThickness));
    previewText.setOutlineColor(mSettings.textOutlineColor);
    previewText.setPosition(position.x + size.x / 2 - previewText.getLocalBounds().width / 2,
                            position.y + size.y / 2 - previewText.getLocalBounds().height / 2);
    mWindow.draw(previewText);
}

void StyleWizard::drawAnimStylePreview(int style, sf::Vector2f position, float scale)
{
    static sf::Clock clock;
    const float t = clock.getElapsedTime().asSeconds();
    const float pulseBase = 1.f + (scale - 1.f) * 0.12f;
    const float pulse = pulseBase + std::sin(t * 5.f) * 0.04f;
    const float bounce = std::fabs(std::sin(t * 4.f)) * 8.f;
    const float wave = std::sin(t * 3.f) * 10.f;

    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(300.f, 56.f));
    panel.setPosition(position);
    panel.setFillColor(sf::Color(35, 35, 35, 230));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(100, 180, 255, 180));
    mWindow.draw(panel);

    const char *styleName =
        style == 0 ? "Default" :
        style == 1 ? "Pulse" :
        style == 2 ? "Bounce" :
        style == 3 ? "Smooth" :
        style == 4 ? "Wave" :
        style == 5 ? "Spin" : "Glow";

    sf::Text label;
    label.setFont(getFont());
    label.setString(std::string("Style: ") + styleName);
    label.setCharacterSize(16);
    label.setFillColor(sf::Color::White);
    label.setPosition(position.x + 12.f, position.y + 7.f);
    mWindow.draw(label);

    sf::RectangleShape backdrop;
    backdrop.setSize(sf::Vector2f(112.f * pulse, 28.f * pulse));
    backdrop.setOrigin(backdrop.getSize() / 2.f);
    backdrop.setPosition(position.x + 200.f + wave * 0.2f, position.y + 30.f + bounce * 0.25f);
    backdrop.setFillColor(sf::Color(mSettings.buttonColor.r, mSettings.buttonColor.g, mSettings.buttonColor.b, 70));
    backdrop.setOutlineThickness(2.f);
    backdrop.setOutlineColor(mSettings.borderColor);
    mWindow.draw(backdrop);

    sf::CircleShape dot(8.f + (style == 1 ? 3.f * (pulse - 1.f) : 0.f));
    dot.setOrigin(dot.getRadius(), dot.getRadius());
    dot.setPosition(position.x + 200.f + wave, position.y + 30.f + bounce);
    dot.setFillColor(mSettings.lightAnimation ? mSettings.lightAnimColor : mSettings.buttonColor);
    mWindow.draw(dot);

    if (mSettings.pressAnimation)
    {
        sf::RectangleShape pressBar;
        pressBar.setSize(sf::Vector2f(28.f, 4.f));
        pressBar.setPosition(position.x + 155.f, position.y + 37.f - std::fabs(std::sin(t * 4.f)) * static_cast<float>(mSettings.pressAnimOffset));
        pressBar.setFillColor(sf::Color(180, 100, 100, 220));
        mWindow.draw(pressBar);
    }

    sf::Text hint;
    hint.setFont(getFont());
    std::string hintText;
    if (mSettings.lightAnimation)
        hintText += "Light";
    if (mSettings.lightAnimation && mSettings.pressAnimation)
        hintText += " + ";
    if (mSettings.pressAnimation)
        hintText += "Press";
    if (hintText.empty())
        hintText = "No animations enabled";
    hint.setString(hintText);
    hint.setCharacterSize(13);
    hint.setFillColor(sf::Color(180, 180, 180));
    hint.setPosition(position.x + 12.f, position.y + 31.f);
    mWindow.draw(hint);
}

void StyleWizard::drawColorSliders(sf::Color &color, sf::Vector2f position)
{
    const float barWidth = 200.f;
    const float barHeight = 12.f;
    const float spacing = 35.f;

    struct Channel { const char *label; sf::Uint8 &ref; };
    Channel channels[] = {
        { "R", color.r },
        { "G", color.g },
        { "B", color.b }
    };

    for (int i = 0; i < 3; ++i)
    {
        sf::Text label;
        label.setFont(getFont());
        label.setString(channels[i].label);
        label.setCharacterSize(16);
        label.setFillColor(sf::Color::White);
        label.setPosition(position.x, position.y + static_cast<float>(i) * spacing);
        mWindow.draw(label);

        sf::RectangleShape bg;
        bg.setSize(sf::Vector2f(barWidth, barHeight));
        bg.setPosition(position.x + 30, position.y + static_cast<float>(i) * spacing + 10);
        bg.setFillColor(sf::Color(60, 60, 60));
        mWindow.draw(bg);

        float fillWidth = barWidth * channels[i].ref / 255.f;
        sf::RectangleShape fill;
        fill.setSize(sf::Vector2f(fillWidth, barHeight));
        fill.setPosition(position.x + 30, position.y + static_cast<float>(i) * spacing + 10);
        fill.setFillColor(i == 0 ? sf::Color(255, 80, 80, 200) : i == 1 ? sf::Color(80, 255, 80, 200) : sf::Color(80, 80, 255, 200));
        mWindow.draw(fill);

        sf::Text value;
        value.setFont(getFont());
        value.setString(std::to_string(static_cast<int>(channels[i].ref)));
        value.setCharacterSize(14);
        value.setFillColor(sf::Color(200, 200, 200));
        value.setPosition(position.x + 30 + barWidth + 10, position.y + static_cast<float>(i) * spacing + 8);
        mWindow.draw(value);
    }
}

void StyleWizard::handleColorSliders(sf::Color &color, sf::Vector2f position, sf::FloatRect sliderBounds[3])
{
    const float barWidth = 200.f;

    if (mMousePressed && mDraggingSlider >= 0 && mDraggingSlider < 3)
    {
        float relX = static_cast<float>(mMousePos.x) - (position.x + 30.f);
        float val = relX / barWidth;
        val = std::clamp(val, 0.f, 1.f);
        sf::Uint8 newVal = static_cast<sf::Uint8>(val * 255.f);

        if (mDraggingSlider == 0) color.r = newVal;
        else if (mDraggingSlider == 1) color.g = newVal;
        else color.b = newVal;

        mColorCustomizationDirty = true;
    }

        if (!mMousePressed)
    {
        mDraggingSlider = -1;
        for (int i = 0; i < 3; ++i)
        {
            if (sliderBounds[i].contains(sf::Vector2f(mMousePos)))
            {
                mDraggingSlider = i;
                break;
            }
        }
    }
}

bool StyleWizard::resetApplyRequest()
{
    bool req = mApplyRequested;
    mApplyRequested = false;
    return req;
}

void StyleWizard::drawNumericSlider(const std::string &label, int &value, int minVal, int maxVal, sf::Vector2f position)
{
    sf::Text txtLabel;
    txtLabel.setFont(getFont());
    txtLabel.setCharacterSize(14);
    txtLabel.setFillColor(sf::Color::White);
    txtLabel.setString(label);
    txtLabel.setPosition(position);
    mWindow.draw(txtLabel);

    const float barWidth = 150.f;
    const float barHeight = 8.f;
    sf::RectangleShape bg(sf::Vector2f(barWidth, barHeight));
    bg.setPosition(position.x + 150.f, position.y + 4.f);
    bg.setFillColor(sf::Color(60, 60, 60));
    mWindow.draw(bg);

    float pct = static_cast<float>(value - minVal) / static_cast<float>(maxVal - minVal);
    sf::RectangleShape fill(sf::Vector2f(barWidth * pct, barHeight));
    fill.setPosition(position.x + 150.f, position.y + 4.f);
    fill.setFillColor(sf::Color(120, 180, 255));
    mWindow.draw(fill);

    sf::CircleShape handle(6.f);
    handle.setOrigin(6.f, 6.f);
    handle.setPosition(position.x + 150.f + barWidth * pct, position.y + 8.f);
    handle.setFillColor(sf::Color::White);
    mWindow.draw(handle);

    sf::Text txtVal;
    txtVal.setFont(getFont());
    txtVal.setCharacterSize(14);
    txtVal.setFillColor(sf::Color::White);
    txtVal.setString(std::to_string(value));
    txtVal.setPosition(position.x + 150.f + barWidth + 15.f, position.y);
    mWindow.draw(txtVal);
}

void StyleWizard::handleNumericSlider(int &value, int minVal, int maxVal, sf::Vector2f position)
{
    const float barWidth = 150.f;
    sf::FloatRect bounds(position.x + 150.f, position.y, barWidth, 16.f);
    if (mMousePressed)
    {
        if (bounds.contains(static_cast<sf::Vector2f>(mMousePos)))
        {
            float relX = static_cast<float>(mMousePos.x) - (position.x + 150.f);
            float pct = std::max(0.f, std::min(1.f, relX / barWidth));
            value = minVal + static_cast<int>(pct * static_cast<float>(maxVal - minVal));
        }
    }
}
