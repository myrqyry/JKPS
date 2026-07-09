#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Font.hpp>

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <fstream>
#include <map>

#include "ResourceIdentifiers.hpp"

class StyleWizard
{
    public:
        enum class Step
        {
            Welcome,
            PresetSelection,
            ColorCustomization,
            AnimationSettings,
            WindowSettings,
            ReviewApply
        };

        StyleWizard(const FontHolder &fontHolder, const TextureHolder &textureHolder);
        ~StyleWizard();

        void openWindow(sf::Vector2i position);
        void closeWindow();
        bool isWindowOpen() const;
        bool resetApplyRequest();

        void processInput();
        void render();

        void setFont(const sf::Font &font);
        void setTexture(const sf::Texture &texture);

    private:
        void handleWelcomeStep();
        void handlePresetSelectionStep();
        void handleColorCustomizationStep();
        void handleAnimationSettingsStep();
        void handleWindowSettingsStep();
        void handleReviewApplyStep();

        void drawWelcomeStep();
        void drawPresetSelectionStep();
        void drawColorCustomizationStep();
        void drawAnimationSettingsStep();
        void drawWindowSettingsStep();
        void drawReviewApplyStep();

        void applyPreset(const std::string &presetName);
        void applyAnimationStyle(int style);
        void saveConfig();
        void loadConfig();
        void resetToOriginal();

        bool isButtonHovered(sf::Vector2i mousePos, const sf::FloatRect &bounds) const;
        void drawButton(const std::string &text, sf::Vector2f position, sf::Vector2f size, bool hovered, int shape = 0);
        void drawToggle(const std::string &label, bool value, sf::Vector2f position);
        void drawColorSwatch(sf::Color color, sf::Vector2f position, float size);
        void drawStatusMessage();
        void drawStepIndicator();
        void drawPreviewButton(sf::Color buttonColor, sf::Color textColor, sf::Vector2f position);
        void drawColorSliders(sf::Color &color, sf::Vector2f position);
        void handleColorSliders(sf::Color &color, sf::Vector2f position, sf::FloatRect sliderBounds[3]);
        void drawNumericSlider(const std::string &label, int &value, int minVal, int maxVal, sf::Vector2f position);
        void handleNumericSlider(int &value, int minVal, int maxVal, sf::Vector2f position);
        void drawShapePreview(int shape, sf::Vector2f position, sf::Vector2f size, sf::Color color);
        void drawAnimStylePreview(int style, sf::Vector2f position, float scale);

        const sf::Font &getFont() const;

        struct Preset
        {
            std::string name;
            std::string description;
            sf::Color buttonColor;
            sf::Color textColor;
            sf::Color backgroundColor;
            sf::Color borderColor;
            bool lightAnimation;
            bool pressAnimation;
            float animationScale;
            int lightAnimScale;
            sf::Color lightAnimColor;
            int pressAnimOffset;
            bool keyPressVisEnabled;
            int keyPressVisSpeed;
            int keyPressVisRotation;
            int keyPressVisFadeDistance;
            sf::Color keyPressVisColor;
            int keyPressVisBonusWidth;
            int keyPressVisFixedHeight;
            int buttonShape;
            int animationStyle;

            sf::Color textOutlineColor;
            int textOutlineThickness;
            int textSize;
        };

        struct WizardSettings
        {
            sf::Color buttonColor;
            sf::Color textColor;
            sf::Color backgroundColor;
            sf::Color borderColor;
            bool lightAnimation;
            bool pressAnimation;
            float animationScale;
            int lightAnimScale;
            sf::Color lightAnimColor;
            int pressAnimOffset;
            bool keyPressVisEnabled;
            int keyPressVisSpeed;
            int keyPressVisRotation;
            int keyPressVisFadeDistance;
            sf::Color keyPressVisColor;
            int keyPressVisBonusWidth;
            int keyPressVisFixedHeight;
            int buttonShape;
            int animationStyle;

            sf::Color textOutlineColor;
            int textOutlineThickness;
            int textSize;

            bool windowTitleBar;
            bool windowResizable;
            int windowBonusSizeTop;
            int windowBonusSizeBottom;
            int windowBonusSizeLeft;
            int windowBonusSizeRight;
        };

    private:
        sf::RenderWindow mWindow;
        sf::View mView;
        Step mCurrentStep;
        const FontHolder *mFontHolder;
        const TextureHolder *mTextureHolder;

        sf::Font mFont;
        sf::Texture mButtonTexture;

        std::vector<Preset> mPresets;
        int mSelectedPreset;

        WizardSettings mSettings;
        WizardSettings mOriginalSettings;

        std::array<std::unique_ptr<sf::Text>, 20> mPresetTexts;
        std::array<sf::FloatRect, 20> mPresetBounds;

        sf::Vector2i mMousePos;
        bool mMousePressed;

        std::string mStatusMessage;
        float mStatusTimer;

        bool mColorCustomizationDirty;
        bool mAnimationSettingsDirty;
        bool mDraggingSpeed;
        int mSelectedColorChannel;
        int mDraggingSlider;
        int mDraggingAnimSlider;
        int mSelectedAnimColor;
        int mSelectedButtonShape;
        int mSelectedAnimStyle;
        bool mApplyRequested;
};
