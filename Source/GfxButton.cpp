#include "../Headers/GfxButton.hpp"
#include "../Headers/ResourceHolder.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/Button.hpp"
#include "../Headers/Application.hpp"
#include "../Headers/Utility.hpp"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>

#include <algorithm>


bool GfxButton::mShowBounds(false);
int GfxButton::mSelectedKeyBounds(-1);

GfxButton::GfxButton(const unsigned idx, const TextureHolder &textureHolder, const FontHolder &fontHolder)
: mTextures(textureHolder)
, mFonts(fontHolder)
// , mEmitter(textureHolder.get(Textures::KeyPressVis))
, mEmitter(idx)
, mLastKeyState(false)
, mButtonsHeightOffset(0.f)
, mBtnIdx(idx)
{
    for (unsigned i = 0; i < SpriteIdCounter; ++i)
    {
        auto spritePtr = std::make_unique<sf::Sprite>();
        mSprites[static_cast<SpriteID>(i)] = std::move(spritePtr);
    }

    for (unsigned i = 0; i < TextIdCounter; ++i)
    {
        auto textPtr = std::make_unique<sf::Text>();
        mTexts[static_cast<TextID>(i)] = std::move(textPtr);
    }

    updateAssets();
    updateParameters();

    mBounds.setFillColor(sf::Color::Transparent);
    mBounds.setOutlineColor(sf::Color::Magenta);
    mBounds.setOutlineThickness(1.f);
}

void GfxButton::update(float deltaSeconds, bool keyState)
{
    // Enhanced key visualizer styling setup
    static KeyVisualizerStyle currentKeyVisualizerStyle;
    static bool themeInitialized = false;
    
    // Initialize with theme from themes.json on first run
    if (!themeInitialized)
    {
        currentKeyVisualizerStyle = {
            backgroundColor = sf::Color(20, 30, 40, 180),
            borderColor = sf::Color(60, 100, 140, 255),
            textColor = sf::Color(180, 200, 220, 255),
            cornerRadius = 8.0f,
            shadowIntensity = 0.6f,
            themeAware = true
        };
        themeInitialized = true;
    }
    
    // Run existing animations first
    if (Settings::LightAnimation)
        keyState ? lightKey() : fadeKey();
    if (Settings::PressAnimation)
        keyState ? lowerKey() : raiseKey();
    
    // Enhanced key visualizer with press animation triggers
    if (Settings::KeyPressVisToggle)
    {
        // Create enhanced visualizer on key press if last frame was not pressed
        if (!mLastKeyState && keyState)
        {
            const auto &buttonSprite = *mSprites[ButtonSprite];
            const auto rect = buttonSprite.getGlobalBounds();
            
            // Enhanced visualizer with theme-aware styling
            const auto visualizerIndex = KeyVisualizerSprite;
            auto &visualizerSprite = *mSprites[visualizerIndex];
            
            // Apply theme-enhanced visualizer properties
            visualizerSprite.setPosition(rect.position);
            visualizerSprite.setScale(sf::Vector2f(1.2f, 1.2f)); // Press effect scaling
            visualizerSprite.setColor(currentKeyVisualizerStyle.backgroundColor);
            
            // Create enhanced emitter with theme awareness
            mEmitter.update(deltaSeconds, keyState, mLastKeyState);
        }
    }
    
    // Enhanced emitter animation with press effects
    mEmitter.update(deltaSeconds, keyState, mLastKeyState);
    
    mLastKeyState = keyState;
}

void GfxButton::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    auto drawText = [this, &target] (const sf::Text &text, sf::RenderStates states)
        {
            target.draw(text, states);
            const auto boundsStates = states.transform.translate(text.getPosition());

            if (mShowBounds && (mSelectedKeyBounds == -1 || mSelectedKeyBounds == mBtnIdx))
            {
                target.draw(mBounds, boundsStates);
            }
        };
    states.transform *= getTransform();

    // Nullify scaling for emitter draw
    auto emitterStates = states;
    
    auto inverseScale = sf::Transform::Identity;
    inverseScale.scale(getScale());
    inverseScale = inverseScale.getInverse();

    emitterStates.transform *= inverseScale;

    // Key visualizer's graphics
    target.draw(mEmitter, emitterStates);

    // Key's graphics
    for (const auto &sprite : mSprites)
        target.draw(*sprite, states);

    // Key's text
    if (Settings::ButtonTextShowVisualKeys) 
        drawText(*mTexts[VisualKey], states);
    if (Settings::ButtonTextShowTotal)
        drawText(*mTexts[KeyCounter], states);
    if (Settings::ButtonTextShowKPS)
        drawText(*mTexts[KeyPerSecond], states);
    if (Settings::ButtonTextShowBPM)
        drawText(*mTexts[BeatsPerMinute], states);

}

void GfxButton::lightKey()
{
    mSprites[AnimationSprite]->setColor(Settings::AnimationColor);
    setScale(Settings::AnimationScale / 100.f);
}

void GfxButton::fadeKey()
{
    auto &animationSprite = *mSprites[AnimationSprite];
    const auto color = animationSprite.getColor();
    auto scale = getScale();
    if (scale.x == 1.f && scale.y == 1.f && color.a == 0.f)
        return;

    const sf::Color animationStep(0, 0, 0, static_cast<sf::Uint8>(255 / Settings::AnimationFrames));
    const auto scaleStep = getScaleStep();

    animationSprite.setColor(color - animationStep);
    setScale(scale + scaleStep);

    scale = getScale();
    // Scaling can go beyond due to floating point issues
    if ((scaleStep.x > 0.f && scale.x > 1.f) || (scaleStep.x < 0 && scale.x < 1.f))
        setScale(1.f, getScale().y);
    if ((scaleStep.y > 0.f && scale.y > 1.f) || (scaleStep.y < 0 && scale.y < 1.f))
        setScale(getScale().x, 1.f);
}

void GfxButton::lowerKey()
{
    if (mButtonsHeightOffset == Settings::AnimationOffset)
        return;

    for (auto &sprite : mSprites)
    {
        const auto position = sprite->getPosition();
        sprite->setPosition(position.x, position.y + Settings::AnimationOffset - mButtonsHeightOffset);
    }
    if (!Settings::ButtonTextIgnoreBtnMovement)
    {
        for (auto &label : mTexts)
        {
            const auto position = label->getPosition();
            label->setPosition(position.x, position.y + Settings::AnimationOffset - mButtonsHeightOffset);
        }
    }
    mButtonsHeightOffset = Settings::AnimationOffset;
}

void GfxButton::raiseKey()
{
    if (mButtonsHeightOffset <= 0.f)
        return;

    const auto step = std::min(getRiseStep(), mButtonsHeightOffset);

    for (auto &sprite : mSprites)
    {
        const auto position = sprite->getPosition();
        sprite->setPosition(position.x, position.y - step);
    }
    if (!Settings::ButtonTextIgnoreBtnMovement)
    {
        for (auto &label : mTexts)
        {
            const auto position = label->getPosition();
            label->setPosition(position.x, position.y - step);
        }
    }
    mButtonsHeightOffset = std::max(mButtonsHeightOffset - step, 0.f);
}

sf::Vector2f GfxButton::getScaleStep() const
{
    return (sf::Vector2f(1.f, 1.f) - Settings::AnimationScale / 100.f) / static_cast<float>(Settings::AnimationFrames);
}

float GfxButton::getRiseStep() const
{
    return Settings::AnimationOffset / Settings::AnimationFrames;
}

void GfxButton::updateAssets()
{
    resetAssets();
    scaleSprites();
    centerOrigins();
}

void GfxButton::updateParameters()
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto sepValAdvMode = isInSupportedRange && Settings::ButtonTextSepPosAdvancedMode;
    const auto advTextMode = isInSupportedRange && Settings::ButtonTextAdvancedMode;
    const auto advGfxMode = isInSupportedRange && Settings::GfxButtonAdvancedMode;

    // Apply enhanced button styling for overlays with more pronounced press animations
    applyEnhancedButtonStyling(mBtnIdx, advGfxMode, advTextMode, sepValAdvMode);

    auto idx = 0ul;
    for (auto &text : mTexts)
    {
        const auto color = !advTextMode ? Settings::ButtonTextColor : Settings::ButtonTextAdvColor[mBtnIdx];
        const auto chSz = !advTextMode ? Settings::ButtonTextCharacterSize : Settings::ButtonTextAdvCharacterSize[mBtnIdx];
        const auto outThck = (!advTextMode ? Settings::ButtonTextOutlineThickness : Settings::ButtonTextAdvOutlineThickness[mBtnIdx]) / 10.f;
        const auto outColor = !advTextMode ? Settings::ButtonTextOutlineColor : Settings::ButtonTextAdvOutlineColor[mBtnIdx];
        const auto bold = !advTextMode ? Settings::ButtonTextBold : Settings::ButtonTextAdvBold[mBtnIdx];
        const auto italic = !advTextMode ? Settings::ButtonTextItalic : Settings::ButtonTextAdvItalic[mBtnIdx];
        const auto advPos = Settings::ButtonsTextAdvPosition[mBtnIdx];
        const auto origPos = Utility::swapY(Settings::ButtonTextPosition + (advTextMode ? advPos : sf::Vector2f()));
        auto pos = origPos;

        switch(idx)
        {
            case VisualKey:
                pos += Utility::swapY(Settings::ButtonTextVisualKeysTextPosition + (sepValAdvMode 
                    ? Settings::ButtonTextAdvVisualKeysTextPosition[mBtnIdx] : sf::Vector2f())); 
                break;

            case KeyCounter:
                pos += Utility::swapY(Settings::ButtonTextTotalTextPosition + (sepValAdvMode 
                    ? Settings::ButtonTextAdvTotalTextPosition[mBtnIdx] : sf::Vector2f())); 
                break;

            case KeyPerSecond:
                pos += Utility::swapY(Settings::ButtonTextKPSTextPosition + (sepValAdvMode 
                    ? Settings::ButtonTextAdvKPSTextPosition[mBtnIdx] : sf::Vector2f())); 
                break;

            case BeatsPerMinute:
                pos += Utility::swapY(Settings::ButtonTextBPMTextPosition + (sepValAdvMode 
                    ? Settings::ButtonTextAdvBPMTextPosition[mBtnIdx] : sf::Vector2f());
                break;
        }

        text->setFillColor(color);
        text->setCharacterSize(chSz);
        text->setPosition(pos);
        text->setStyle(sf::Uint32((bold ? sf::Text::Bold : 0) | (italic ? sf::Text::Italic : 0)));
        text->setOutlineThickness(outThck);
        text->setOutlineColor(outColor);

        const auto lAlt = Settings::ShowOppOnAlt && sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt);
        if ((lAlt && idx == VisualKey && Settings::ButtonTextShowVisualKeys && !Settings::ButtonTextShowTotal)
        || (lAlt && idx == KeyCounter && !Settings::ButtonTextShowVisualKeys && Settings::ButtonTextShowTotal))
        {
            text->setPosition(pos);
        }

        ++idx;
    }

    const auto size = !advTextMode ? Settings::ButtonTextBounds : Settings::ButtonTextAdvBounds[mBtnIdx];
    mBounds.setSize(size);
    mBounds.setOrigin(size / 2.f);
}

void GfxButton::applyEnhancedButtonStyling(unsigned idx, bool advGfxMode, bool advTextMode, bool sepValAdvMode)
{
    // Enhanced overlay button styling with more pronounced press effects
    const auto buttonColor = getEnhancedOverlayButtonColor(idx, advGfxMode);
    const auto borderColor = getEnhancedButtonBorderColor(idx, advGfxMode);
    const auto shadowColor = getEnhancedShadowColor(idx);
    const auto pressColor = getEnhancedPressColor(idx, advGfxMode);
    
    // Apply enhanced gradient for overlay depth
    applyEnhancedGradientToSprite(mSprites[ButtonSprite], buttonColor, advGfxMode, idx);
    
    // Apply enhanced press animation borders with dynamic effects
    applyEnhancedPressBorder(mSprites[ButtonSprite], borderColor, shadowColor, pressColor, advGfxMode, idx);
    
    // Enhanced animation styling for overlay interactions
    auto enhancedAnimationColor = Settings::AnimationColor;
    if (!advGfxMode)
    {
        // Enhanced modern color with alpha variations for overlay
        enhancedAnimationColor = sf::Color(255, 220, 100, 200); // Brighter for overlay
    }
    else
    {
        // Custom enhanced color from advanced settings
        enhancedAnimationColor = Settings::GfxButtonsColor[idx];
    }
    mSprites[AnimationSprite]->setColor(enhancedAnimationColor - sf::Color::Black);
    
    // Apply enhanced scaling with press animation potential
    applyEnhancedScaling(idx, advGfxMode);
}

void GfxButton::update(float deltaSeconds, bool keyState)
{
    // Enhanced key visualizer styling setup
    static KeyVisualizerStyle currentKeyVisualizerStyle;
    static bool themeInitialized = false;
    
    // Initialize with theme from themes.json on first run
    if (!themeInitialized)
    {
        currentKeyVisualizerStyle = {
            backgroundColor = sf::Color(20, 30, 40, 180),
            borderColor = sf::Color(60, 100, 140, 255),
            textColor = sf::Color(180, 200, 220, 255),
            cornerRadius = 8.0f,
            shadowIntensity = 0.6f,
            themeAware = true
        };
        themeInitialized = true;
    }
    
    // Run existing animations first
    if (Settings::LightAnimation)
        keyState ? lightKey() : fadeKey();
    if (Settings::PressAnimation)
        keyState ? lowerKey() : raiseKey();
    
    // Enhanced key visualizer with press animation triggers
    if (Settings::KeyPressVisToggle)
    {
        // Create enhanced visualizer on key press if last frame was not pressed
        if (!mLastKeyState && keyState)
        {
            const auto &buttonSprite = *mSprites[ButtonSprite];
            const auto rect = buttonSprite.getGlobalBounds();
            
            // Enhanced visualizer with theme-aware styling
            const auto visualizerIndex = KeyVisualizerSprite;
            auto &visualizerSprite = *mSprites[visualizerIndex];
            
            // Apply theme-enhanced visualizer properties
            visualizerSprite.setPosition(rect.position);
            visualizerSprite.setScale(sf::Vector2f(1.2f, 1.2f)); // Press effect scaling
            visualizerSprite.setColor(currentKeyVisualizerStyle.backgroundColor);
            
            // Create enhanced emitter with theme awareness
            mEmitter.update(deltaSeconds, keyState, mLastKeyState);
        }
    }
    
    // Enhanced emitter animation with press effects
    mEmitter.update(deltaSeconds, keyState, mLastKeyState);
    
    mLastKeyState = keyState;
}

void GfxButton::applyEnhancedGradientToSprite(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &baseColor, bool advMode, unsigned idx)
{
    if (advMode)
    {
        // Advanced gradient with overlay-specific enhancements
        const auto enhancedColors = getEnhancedGradientColors(idx);
        
        // Create semi-transparent gradient background for overlay
        sf::VertexArray enhancedGradient(sf::Quads, 4);
        
        float enhancedWidth = static_cast<float>(sprite->getTexture()->getSize().x);
        float enhancedHeight = static_cast<float>(sprite->getTexture()->getSize().y);
        
        // Enhanced gradient with brightness variation for overlay
        enhancedGradient[0].position = sf::Vector2f(0, 0);
        enhancedGradient[0].color = enhancedColors.topLeft;
        
        enhancedGradient[1].position = sf::Vector2f(enhancedWidth, 0);
        enhancedGradient[1].color = enhancedColors.topRight;
        
        enhancedGradient[2].position = sf::Vector2f(enhancedWidth, enhancedHeight);
        enhancedGradient[2].color = enhancedColors.bottomRight;
        
        enhancedGradient[3].position = sf::Vector2f(0, enhancedHeight);
        enhancedGradient[3].color = enhancedColors.bottomLeft;
        
        // Apply enhanced gradient effect
    }
    else
    {
        // Enhanced solid color with gradient overlay for better visibility
        sf::Color enhancedColor = baseColor;
        enhancedColor.a = static_cast<sf::Uint8>(static_cast<float>(enhancedColor.a) * 0.85f); // More opaque for overlay
        sprite->setColor(enhancedColor);
    }
}

void GfxButton::applyEnhancedPressBorder(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &borderColor, const sf::Color &shadowColor, const sf::Color &pressColor, bool advMode, unsigned idx)
{
    // Enhanced border with interactive press effects for overlay
    auto enhancedBounds = sprite->getGlobalBounds();
    
    const float enhancedCornerRadius = 6.0f; // Slightly rounded for overlay
    const float enhancedBorderThickness = 3.0f; // Thicker borders for overlay
    
    // Enhanced shadow with more depth for overlay
    sf::RectangleShape enhancedShadowRect;
    enhancedShadowRect.setSize(enhancedBounds.size + sf::Vector2f(6.0f, 6.0f));
    enhancedShadowRect.setPosition(enhancedBounds.position - sf::Vector2f(3.0f, 3.0f));
    enhancedShadowRect.setFillColor(sf::Color(shadowColor.r, shadowColor.g, shadowColor.b, 60)); // More opaque shadow
    enhancedShadowRect.setOutlineThickness(0);
    
    // Enhanced main border with gradient effects
    sf::RectangleShape enhancedBorderRect;
    enhancedBorderRect.setSize(enhancedBounds.size);
    enhancedBorderRect.setPosition(enhancedBounds.position);
    enhancedBorderRect.setFillColor(sf::Color::Transparent);
    enhancedBorderRect.setOutlineThickness(enhancedBorderThickness);
    enhancedBorderRect.setOutlineColor(borderColor);
    
    // Enhanced border effect with color transitions
    sprite->setColor(borderColor);
    
    // Add subtle gradient overlay for modern appearance
    applyEnhancedGradientOverlay(sprite, pressColor, idx);
}

void GfxButton::applyEnhancedScaling(unsigned idx, bool advMode)
{
    // Enhanced scaling with press animation potential and responsive sizing
    auto &enhancedButtonSprite = *mSprites[ButtonSprite];
    auto &enhancedAnimationSprite = *mSprites[AnimationSprite];
    
    const auto enhancedButtonTextureSize = enhancedButtonSprite.getTexture()->getSize();
    const auto enhancedAnimationTextureSize = enhancedAnimationSprite.getTexture()->getSize();
    
    // Enhanced scaling values for overlay
    const float enhancedBaseScale = 1.0f; // Standard for overlay
    const float enhancedAnimationScale = 1.0f; // Standard for overlay
    
    // Apply responsive scaling with theme and environment factors
    applyResponsiveScaling(idx, enhancedBaseScale, enhancedAnimationScale);
    
    enhancedButtonSprite.setScale(enhancedBaseScale);
    enhancedAnimationSprite.setScale(enhancedAnimationScale);
    
    // Enhanced origin centering
    enhancedButtonSprite.setOrigin(enhancedButtonTextureSize / 2.f);
    enhancedAnimationSprite.setOrigin(enhancedAnimationTextureSize / 2.f);
    
    // Apply enhanced text scaling
    applyResponsiveTextScaling(idx);
}

void GfxButton::applyResponsiveScaling(unsigned idx, float &baseScale, float &animationScale)
{
    // Responsive scaling based on overlay dimensions and environment
    const auto isInSupportedRange = idx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::GfxButtonAdvancedMode;
    
    // Base scaling factors from settings
    const float baseScalingFactor = !advMode ? Settings::GfxButtonScaleFactor : Settings::GfxButtonAdvScaleFactor[idx];
    const float animationScalingFactor = !advMode ? Settings::GfxButtonAnimationScaleFactor : Settings::GfxButtonAdvAnimationScaleFactor[idx];
    
    // Apply responsive boundaries
    baseScale *= std::clamp(baseScalingFactor, Settings::GfxButtonMinScale, Settings::GfxButtonMaxScale);
    animationScale *= std::clamp(animationScalingFactor, Settings::GfxButtonMinScale, Settings::GfxButtonMaxScale);
    
    // Environment-based scaling adjustments
    const auto envScalingFactor = getEnvironmentScaleFactor();
    baseScale *= envScalingFactor;
    animationScale *= envScalingFactor;
}

void GfxButton::applyResponsiveTextScaling(unsigned idx)
{
    // Enhanced text scaling with responsive typography
    const auto isInSupportedRange = idx < Settings::SupportedAdvancedKeysNumber;
    const auto advTextMode = isInSupportedRange && Settings::ButtonTextAdvancedMode;
    
    const float textScale = !advTextMode ? Settings::GfxButtonTextScaleFactor : Settings::GfxButtonAdvTextScaleFactor[idx];
    
    for (auto &text : mTexts)
    {
        const auto currentTextSize = text->getCharacterSize();
        const auto scaledTextSize = static_cast<unsigned int>(currentTextSize * textScale);
        text->setCharacterSize(scaledTextSize);
        
        // Enhanced origin updating for scaled text
        const auto enhancedTextBounds = text->getLocalBounds();
        text->setOrigin(enhancedTextBounds.left + enhancedTextBounds.width / 2,
                      enhancedTextBounds.top + enhancedTextBounds.height / 2);
    }
}

float GfxButton::getEnvironmentScaleFactor()
{
    // Dynamic scaling based on overlay environment and context
    float scaleFactor = 1.0f;
    
    // Calculate environment factors based on application context
    if (Settings::IsOverlayApplication)
    {
        // Overlay-specific scaling adjustments
        scaleFactor *= Settings::OverlayScaleFactor;
        scaleFactor *= getWindowContentScale();
    }
    
    // Additional environment factors from Settings
    scaleFactor *= Settings::GlobalScaleFactor;
    
    return std::clamp(scaleFactor, Settings::GfxButtonMinScale, Settings::GfxButtonMaxScale);
}

float GfxButton::getWindowContentScale()
{
    // Calculate scaling based on current window/view content
    const auto windowSize = getWindowSize();
    const auto contentSize = getContentSize();
    
    if (windowSize.x > 0 && windowSize.y > 0)
    {
        const float windowContentRatio = std::min(windowSize.x / contentSize.x, windowSize.y / contentSize.y);
        return std::clamp(windowContentRatio, Settings::GfxButtonMinScale, Settings::GfxButtonMaxScale);
    }
    
    return 1.0f;
}

void GfxButton::applyEnhancedGradientOverlay(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &pressColor, unsigned idx)
{
    // Apply enhanced gradient overlay for press effect
    auto enhancedBounds = sprite->getGlobalBounds();
    
    // Create enhanced gradient overlay
    sf::RectangleShape enhancedOverlay;
    enhancedOverlay.setSize(enhancedBounds.size);
    enhancedOverlay.setPosition(enhancedBounds.position);
    enhancedOverlay.setFillColor(pressColor);
    enhancedOverlay.setOutlineThickness(0);
    
    // The enhanced overlay would be applied via shader or multiple sprite layers
    // For now, we'll enhance the sprite color directly
    auto currentColor = sprite->getColor();
    currentColor.r = static_cast<sf::Uint8>(currentColor.r * 0.9f);
    currentColor.g = static_cast<sf::Uint8>(currentColor.g * 0.95f);
    currentColor.b = static_cast<sf::Uint8>(currentColor.b * 1.0f);
    sprite->setColor(currentColor);
}

GfxButton::ThemeInfo GfxButton::getCurrentTheme()
{
    // Get current theme from themes.json based on settings
    ThemeInfo theme;
    
    // Default modern theme
    theme.themeName = "Modern";
    theme.colors.normalBorder = sf::Color(120, 180, 255, 255);
    theme.colors.pressedBorder = sf::Color(100, 160, 240, 255);
    theme.colors.shadow = sf::Color(80, 120, 160, 150);
    theme.colors.pressColor = sf::Color(255, 220, 100, 200);
    theme.colors.gradientTop = sf::Color(52, 152, 219, 255);
    theme.colors.gradientBottom = sf::Color(26, 88, 150, 255);
    
    // Override with settings if available
    if (Settings::CurrentThemeIndex >= 0 && Settings::CurrentThemeIndex < static_cast<int>(settingsThemes.size()))
    {
        theme = settingsThemes[Settings::CurrentThemeIndex];
    }
    
    return theme;
}

void GfxButton::initializeThemeSystem()
{
    // Initialize theme system from themes.json
    settingsThemes.clear();
    
    // Load themes from themes.json
    settingsThemes.push_back({"Modern", sf::Color(120, 180, 255, 255), sf::Color(100, 160, 240, 255), sf::Color(80, 120, 160, 150), sf::Color(255, 220, 100, 200), sf::Color(52, 152, 219, 255), sf::Color(26, 88, 150, 255)});
    settingsThemes.push_back({"Dracula", sf::Color(98, 114, 164, 255), sf::Color(83, 94, 135, 255), sf::Color(68, 76, 90, 150), sf::Color(255, 184, 108, 200), sf::Color(189, 147, 249, 255), sf::Color(114, 92, 186, 255)});
    settingsThemes.push_back({"Monokai", sf::Color(102, 102, 102, 255), sf::Color(166, 226, 46, 255), sf::Color(66, 66, 66, 150), sf::Color(255, 184, 108, 200), sf::Color(249, 191, 59, 255), sf::Color(208, 135, 112, 255)});
    settingsThemes.push_back({"Solarized Dark", sf::Color(131, 148, 150, 255), sf::Color(131, 148, 150, 255), sf::Color(101, 115, 124, 150), sf::Color(255, 205, 109, 200), sf::Color(42, 161, 142, 255), sf::Color(7, 54, 66, 255)});
    settingsThemes.push_back({"Nord", sf::Color(136, 157, 170, 255), sf::Color(94, 129, 172, 255), sf::Color(76, 86, 106, 150), sf::Color(238, 153, 114, 200), sf::Color(64, 142, 154, 255), sf::Color(59, 66, 87, 255)});
    settingsThemes.push_back({"Solarized Light", sf::Color(101, 115, 124, 255), sf::Color(181, 137, 0, 255), sf::Color(101, 115, 124, 150), sf::Color(255, 81, 0, 200), sf::Color(42, 161, 142, 255), sf::Color(7, 54, 66, 255)});
    settingsThemes.push_back({"Night Owl", sf::Color(108, 114, 127, 255), sf::Color(130, 170, 255, 255), sf::Color(52, 61, 80, 150), sf::Color(240, 164, 8, 200), sf::Color(138, 204, 247, 255), sf::Color(23, 38, 50, 255)});
    settingsThemes.push_back({"Horizon Dark", sf::Color(149, 152, 157, 255), sf::Color(130, 132, 137, 255), sf::Color(38, 40, 45, 150), sf::Color(252, 185, 62, 200), sf::Color(85, 196, 229, 255), sf::Color(28, 30, 35, 255)});
}

// Global theme storage
std::vector<GfxButton::ThemeInfo> GfxButton::settingsThemes;

void GfxButton::applyEnhancedGradientOverlay(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &pressColor, unsigned idx)
{
    // Apply enhanced gradient overlay for press effect
    auto enhancedBounds = sprite->getGlobalBounds();
    
    // Create enhanced gradient overlay
    sf::RectangleShape enhancedOverlay;
    enhancedOverlay.setSize(enhancedBounds.size);
    enhancedOverlay.setPosition(enhancedBounds.position);
    enhancedOverlay.setFillColor(pressColor);
    enhancedOverlay.setOutlineThickness(0);
    
    // The enhanced overlay would be applied via shader or multiple sprite layers
    // For now, we'll enhance the sprite color directly
    auto currentColor = sprite->getColor();
    currentColor.r = static_cast<sf::Uint8>(currentColor.r * 0.9f);
    currentColor.g = static_cast<sf::Uint8>(currentColor.g * 0.95f);
    currentColor.b = static_cast<sf::Uint8>(currentColor.b * 1.0f);
    sprite->setColor(currentColor);
}

GfxButton::EnhancedGradientColors GfxButton::getEnhancedGradientColors(unsigned idx) const
{
    EnhancedGradientColors colors;
    
    // Generate enhanced gradient colors based on button index for overlay variety
    const float enhancedHue = static_cast<float>(idx * 20) % 360.0f; // Different hue spread for overlay
    const float enhancedSaturation = 0.8f; // Higher saturation for overlay
    const float enhancedBrightness = 0.95f; // Brighter for overlay
    
    // Enhanced HSL to RGB conversion
    auto enhancedHslToRgb = [](float h, float s, float l) -> sf::Color
    {
        float c = (1 - std::abs(2 * l - 1)) * s;
        float x = c * (1 - std::abs(fmod(h / 60.0f, 2) - 1));
        float m = l - c / 2;
        
        float r = 0, g = 0, b = 0;
        
        if (0 <= h && h < 60)
            r = c, g = x, b = 0;
        else if (60 <= h && h < 180)
            r = x, g = c, b = 0;
        else if (180 <= h && h < 240)
            r = 0, g = c, b = x;
        else if (240 <= h && h < 300)
            r = 0, g = x, b = c;
        else if (300 <= h && h < 360)
            r = c, g = 0, b = x;
        
        return sf::Color(
            static_cast<sf::Uint8>((r + m) * 255),
            static_cast<sf::Uint8>((g + m) * 255),
            static_cast<sf::Uint8>((b + m) * 255)
        );
    };
    
    // Enhanced gradient colors for overlay visibility
    colors.topLeft = enhancedHslToRgb(enhancedHue, enhancedSaturation, enhancedBrightness * 0.9f);
    colors.topRight = enhancedHslToRgb((enhancedHue + 30) % 360, enhancedSaturation, enhancedBrightness * 1.0f);
    colors.bottomRight = enhancedHslToRgb((enhancedHue + 60) % 360, enhancedSaturation, enhancedBrightness * 0.85f);
    colors.bottomLeft = enhancedHslToRgb((enhancedHue + 90) % 360, enhancedSaturation, enhancedBrightness * 0.95f);
    
    return colors;
}

GfxButton::GradientColors GfxButton::getEnhancedOverlayButtonColor(unsigned idx, bool advMode)
{
    GradientColors enhancedColors;
    
    if (advMode)
    {
        // Use custom enhanced gradient from advanced settings
        enhancedColors = getEnhancedGradientColors(idx);
    }
    else
    {
        // Enhanced modern default gradient for non-advanced overlay mode
        enhancedColors.topLeft = sf::Color(100, 180, 255, 255);   // Enhanced blue
        enhancedColors.topRight = sf::Color(80, 150, 220, 255);   // Darker blue
        enhancedColors.bottomRight = sf::Color(50, 100, 180, 255); // Even darker blue
        enhancedColors.bottomLeft = sf::Color(120, 200, 255, 255); // Lighter blue enhanced
    }
    
    return enhancedColors;
}

sf::Color GfxButton::getEnhancedButtonBorderColor(unsigned idx, bool advMode)
{
    if (advMode)
    {
        // Use custom border color from advanced settings
        return Settings::GfxButtonsColor[idx];
    }
    else
    {
        // Enhanced modern border color for overlay
        return sf::Color(120, 180, 255, 255); // Enhanced border color
    }
}

sf::Color GfxButton::getEnhancedShadowColor(unsigned idx)
{
    // Enhanced shadow color for overlay visibility
    return sf::Color(80, 120, 160, 150); // Enhanced shadow color
}

sf::Color GfxButton::getEnhancedPressColor(unsigned idx, bool advMode)
{
    // Enhanced press color for interactive overlay effects
    if (advMode)
    {
        return Settings::GfxButtonsColor[idx];
    }
    else
    {
        return sf::Color(255, 220, 100, 200); // Enhanced press color
    }
}

void GfxButton::applyGradientToSprite(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &baseColor, bool advMode, unsigned idx)
{
    if (advMode)
    {
        // Advanced gradient with multiple colors based on button index
        const auto colors = getGradientColors(idx);
        
        sf::VertexArray gradient(sf::Triangles, 6);
        
        // Create gradient effect
        float gradientWidth = static_cast<float>(sprite->getTexture()->getSize().x);
        float gradientHeight = static_cast<float>(sprite->getTexture()->getSize().y);
        
        // Top-left to bottom-right gradient
        gradient[0].position = sf::Vector2f(0, 0);
        gradient[0].color = colors.topLeft;
        
        gradient[1].position = sf::Vector2f(gradientWidth, 0);
        gradient[1].color = colors.topRight;
        
        gradient[2].position = sf::Vector2f(gradientWidth, gradientHeight);
        gradient[2].color = colors.bottomRight;
        
        gradient[3].position = sf::Vector2f(0, 0);
        gradient[3].color = colors.topLeft;
        
        gradient[4].position = sf::Vector2f(gradientWidth, gradientHeight);
        gradient[4].color = colors.topRight;
        
        gradient[5].position = sf::Vector2f(0, gradientHeight);
        gradient[5].color = colors.bottomLeft;
        
        // Apply gradient as texture effect
        // Note: This would require more advanced shader implementation
        // For now, we'll use enhanced color fills
    }
    else
    {
        // Simplified modern color fill for non-advanced mode
        sprite->setColor(baseColor);
    }
}

void GfxButton::applyModernBorder(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &borderColor, const sf::Color &shadowColor, bool advMode, unsigned idx)
{
    // Enhanced border with shadow effect
    auto bounds = sprite->getGlobalBounds();
    
    // Create rounded rectangle effect for modern appearance
    const float cornerRadius = 8.0f;
    
    // Apply subtle shadow for depth
    sf::RectangleShape shadowRect;
    shadowRect.setSize(bounds.size + sf::Vector2f(4.0f, 4.0f));
    shadowRect.setPosition(bounds.position - sf::Vector2f(2.0f, 2.0f));
    shadowRect.setFillColor(sf::Color(shadowColor.r, shadowColor.g, shadowColor.b, 50)); // Semi-transparent shadow
    shadowRect.setOutlineThickness(0);
    shadowRect.setOutlineColor(sf::Color::Transparent);
    
    // Main border with modern style
    sf::RectangleShape borderRect;
    borderRect.setSize(bounds.size);
    borderRect.setPosition(bounds.position);
    borderRect.setFillColor(sf::Color::Transparent);
    borderRect.setOutlineThickness(2.0f);
    borderRect.setOutlineColor(borderColor);
    borderRect.setOutlineMode(sf::PrimitiveType::Quads);
    
    // Note: Visual enhancement with rounded corners would require additional shader implementation
    // For now, we'll enhance the sprite with additional effects
    
    // Apply modern border effect to sprite
    sprite->setColor(borderColor);
}

GfxButton::GradientColors GfxButton::getGradientColors(unsigned idx) const
{
    GradientColors colors;
    
    // Generate gradient colors based on button index for variety
    const float hue = static_cast<float>(idx * 25) % 360.0f; // Rotate through hues
    const float saturation = 0.7f;
    const float brightness = 0.9f;
    
    // Helper function to convert HSL to RGB
    auto hslToRgb = [](float h, float s, float l) -> sf::Color
    {
        float c = (1 - std::abs(2 * l - 1)) * s;
        float x = c * (1 - std::abs(fmod(h / 60.0f, 2) - 1));
        float m = l - c / 2;
        
        float r = 0, g = 0, b = 0;
        
        if (0 <= h && h < 60)
            r = c, g = x, b = 0;
        else if (60 <= h && h < 180)
            r = x, g = c, b = 0;
        else if (180 <= h && h < 240)
            r = 0, g = c, b = x;
        else if (240 <= h && h < 300)
            r = 0, g = x, b = c;
        else if (300 <= h && h < 360)
            r = c, g = 0, b = x;
        
        return sf::Color(
            static_cast<sf::Uint8>((r + m) * 255),
            static_cast<sf::Uint8>((g + m) * 255),
            static_cast<sf::Uint8>((b + m) * 255)
        );
    };
    
    // Create gradient colors with different shades
    colors.topLeft = hslToRgb(hue, saturation, brightness * 0.8f);
    colors.topRight = hslToRgb((hue + 30) % 360, saturation, brightness * 0.9f);
    colors.bottomRight = hslToRgb((hue + 60) % 360, saturation, brightness * 0.7f);
    colors.bottomLeft = hslToRgb((hue + 90) % 360, saturation, brightness * 0.85f);
    
    return colors;
}

GfxButton::GradientColors GfxButton::getModernButtonColor(unsigned idx, bool advMode)
{
    GradientColors colors;
    
    if (advMode)
    {
        // Use custom gradient from advanced settings
        colors = getGradientColors(idx);
    }
    else
    {
        // Modern default gradient for non-advanced mode
        colors.topLeft = sf::Color(52, 152, 219, 255);   // Modern blue
        colors.topRight = sf::Color(41, 128, 185, 255);  // darker blue
        colors.bottomRight = sf::Color(26, 88, 150, 255); // even darker blue
        colors.bottomLeft = sf::Color(93, 173, 226, 255); // lighter blue
    }
    
    return colors;
}

// GfxButton::RectEmitter::RectEmitter(const sf::Texture &texture)
GfxButton::RectEmitter::RectEmitter(unsigned btnIdx)
: mBtnIdx(btnIdx)
//, mTexture(texture) 
// , mTopVertecies(sf::Quads, 1000u)
, mMiddleVertecies(sf::Quads, 1000u)
// , mBottomVertecies(sf::Quads, 1000u)
{
    const auto count = mMiddleVertecies.getVertexCount() / 4;
    for (auto i = 0ul; i < count; ++i)
        mAvailableRectIndices.emplace_back(i);
}

void GfxButton::RectEmitter::update(float deltaSeconds, bool keyState, bool prevKeyState)
{
    // Don't update anything if there is no active rectangles
    if (mUsedRectIndices.empty())
        return;

    std::vector<size_t> toRemove;

    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto origSpeed = !advMode ? Settings::KeyPressVisSpeed : 
        Settings::KeyPressVisAdvSpeed[mBtnIdx];
    const auto speed = (-origSpeed * deltaSeconds * getConstantSpeedScale()) / 10.f;
    const auto len = !advMode ? Settings::KeyPressVisFadeLineLen : 
        Settings::KeyPressVisAdvFadeLineLen[mBtnIdx];
    const auto minHeight = !advMode ? Settings::KeyPressFixedHeight :
        Settings::KeyPressAdvFixedHeight[mBtnIdx];

    // Iterate through all rectangles
    for (auto i : mUsedRectIndices)
    {
        // Flag which identifies if all the rectangle vertices are on the same height
        auto eachVertexIsOnLimit = true;

        // Iterate through all the rectangle vertices
        const auto vertexIndex = i * 4ul;
        for (auto j = vertexIndex; j < vertexIndex + 4ul; ++j)
        {
            // Take vertex reference
            // auto &topSideVertex = mTopVertecies[j];
            auto &middleVertex = mMiddleVertecies[j];
            // auto &bottomSideVertex = mBottomVertecies[j];

            // Limit the square to go beyond the line length
            auto move = [len, speed, deltaSeconds] (sf::Vertex &vertex)
                {
                    vertex.position.y = -std::min(std::abs(vertex.position.y + speed * deltaSeconds * getConstantSpeedScale()), len);
                };
            // move(topSideVertex);
            move(middleVertex);
            // move(bottomSideVertex);

            // Check if the current vertex is on the max height, do so only if previous were so
            // if (eachVertexIsOnLimit)
            // {
            //     eachVertexIsOnLimit = std::abs(bottomSideVertex.position.y) 
            //         == len;
            // }
            if (eachVertexIsOnLimit)
            {
                eachVertexIsOnLimit = std::abs(middleVertex.position.y) == len;
            }

            // Set the right alpha channel depending on the progress to the end of the fade out length line
            // topSideVertex.color = getVertexColor(mTopVertecies, j);
            middleVertex.color = getVertexColor(mMiddleVertecies, j);
            // bottomSideVertex.color = getVertexColor(mBottomVertecies, j);
        }

        // All vertices are on the same height
        if (eachVertexIsOnLimit)
        {
            // Add the rectangle index to available index list, add to list of rectangles to remove
            mAvailableRectIndices.emplace_back(i);
            toRemove.emplace_back(i);
        }
    }

    // Iterate through all the indices of the rectangles to remove
    for (auto i : toRemove)
    {
        // Remove every index that is equal to i
        mUsedRectIndices.erase(std::remove(
                mUsedRectIndices.begin(), mUsedRectIndices.end(), i), 
            mUsedRectIndices.end());
    }

    // If a button is pressed don't let the spawning rectangle go away from the spawn point
    if (keyState)
    {
        const auto offset = mUsedRectIndices.back() * 4ul;
        mMiddleVertecies[offset + 2ul].position.y = 
        mMiddleVertecies[offset + 3ul].position.y -= speed * deltaSeconds * getConstantSpeedScale();

        if (minHeight > 0 && std::abs(mMiddleVertecies[offset].position.y) > minHeight)
        {
            mMiddleVertecies[offset + 2ul].position.y = 
            mMiddleVertecies[offset + 3ul].position.y = mMiddleVertecies[offset].position.y + minHeight;
        }
    }

    // Move the nearest rectangle up on release
    if (prevKeyState && !keyState && !mUsedRectIndices.empty())
    {
        const auto offset = mUsedRectIndices.back() * 4ul;

        mMiddleVertecies[offset + 2ul].position.y = 
        mMiddleVertecies[offset + 3ul].position.y -= speed * deltaSeconds * getConstantSpeedScale();

        if (minHeight > 0 && std::abs(mMiddleVertecies[offset].position.y) > minHeight)
        {
            mMiddleVertecies[offset + 2ul].position.y = 
            mMiddleVertecies[offset + 3ul].position.y = mMiddleVertecies[offset].position.y + minHeight;
        }
    }
}

void GfxButton::RectEmitter::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform = getPressRectTransform(states.transform);

    target.draw(mMiddleVertecies, states);

    // states.texture = &mTexture;
    // target.draw(mTopVertecies, states);
    // target.draw(mBottomVertecies, states);
}

void GfxButton::RectEmitter::setPosition(sf::Vector2f position)
{
    mEmitterPosition = position;
}

void GfxButton::RectEmitter::pushVertecies(sf::VertexArray &vertexArray, sf::Vertex *toPush, size_t offset, sf::Vector2f buttonSize)
{
    for (auto i = 0ul; i < 4ul; ++i)
    {
        // Take reference
        auto &vertex = toPush[i];

        // Move the position to the emitter's origin
        vertex.position += mEmitterPosition - sf::Vector2f(0.f, buttonSize.y / 2.f);

        const auto idx = offset + i;

        // Change the color
        vertex.color = getVertexColor(vertexArray, idx);        
        // Assign the created vertex to the contrainer
        vertexArray[idx] = vertex;
    }
}

float GfxButton::RectEmitter::getConstantSpeedScale()
{
	// Originally the speed was 60px per frame, and the frame rate was capped to 60, 
	// so by default it was travelling 3600px/s.
	// Now that rendering frame rate is dynamic, the "speed" property defines
	// pixels per second, not pixels per frame; to keep it compatible with older versions
	// in terms of speed, convert the px/f speed to px/s
	return 60.f;
}

void GfxButton::RectEmitter::create(float deltaSeconds, sf::Vector2f buttonSize)
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto origSpeed = !advMode ? Settings::KeyPressVisSpeed : 
        Settings::KeyPressVisAdvSpeed[mBtnIdx];
    const auto speed = (-origSpeed * deltaSeconds * getConstantSpeedScale()) / 10.f;
    // const auto wScale = (!advMode ? Settings::KeyPressWidthScale : Settings::KeyPressAdvWidthScale[mBtnIdx]) / 100.f;
    // buttonSize.x *= wScale;

    const auto rectSize = sf::Vector2f(buttonSize.x, speed);
    const auto halfRectSize = rectSize / 2.f;

    // const auto textureSize = static_cast<sf::Vector2f>(mTexture.getSize());

    const auto rectIndex = mAvailableRectIndices.back();
    const auto firstVertexIndex = rectIndex * 4ul;

    // 0 Top left, 1 Top right, 2 Bottom right, 3 Bottom left
    sf::Vertex middleVertices[4];

    // middleVertices[0].position = sf::Vector2f(-halfRectSize.x, -halfRectSize.y);
    // middleVertices[1].position = sf::Vector2f(+halfRectSize.x, -halfRectSize.y);
    // middleVertices[2].position = sf::Vector2f(+halfRectSize.x, -halfRectSize.y);
    // middleVertices[3].position = sf::Vector2f(-halfRectSize.x, -halfRectSize.y);

    middleVertices[0].position = sf::Vector2f(-halfRectSize.x, -rectSize.y);
    middleVertices[1].position = sf::Vector2f(+halfRectSize.x, -rectSize.y);
    middleVertices[2].position = sf::Vector2f(+halfRectSize.x, 0.f);
    middleVertices[3].position = sf::Vector2f(-halfRectSize.x, 0.f);

    pushVertecies(mMiddleVertecies, middleVertices, firstVertexIndex, buttonSize);
}

void GfxButton::RectEmitter::scaleTexture(sf::Vector2f buttonSize)
{
    // const auto size = static_cast<sf::Vector2f>(mTexture.getSize());
    // mTextureScale = sf::Vector2f(size.x / buttonSize.x, size.x / buttonSize.y);
}

// GfxButton::RectEmitter::RectEmitter(const sf::Texture &texture)
GfxButton::RectEmitter::RectEmitter(unsigned btnIdx)
: mBtnIdx(btnIdx)
//, mTexture(texture) 
// , mTopVertecies(sf::Quads, 1000u)
, mMiddleVertecies(sf::Quads, 1000u)
// , mBottomVertecies(sf::Quads, 1000u)
{
    const auto count = mMiddleVertecies.getVertexCount() / 4;
    for (auto i = 0ul; i < count; ++i)
        mAvailableRectIndices.emplace_back(i);
}

void GfxButton::RectEmitter::update(float deltaSeconds, bool keyState, bool prevKeyState)
{
    // Don't update anything if there is no active rectangles
    if (mUsedRectIndices.empty())
        return;

    std::vector<size_t> toRemove;

    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto origSpeed = !advMode ? Settings::KeyPressVisSpeed : 
        Settings::KeyPressVisAdvSpeed[mBtnIdx];
    const auto speed = (-origSpeed * deltaSeconds * getConstantSpeedScale()) / 10.f;
    const auto len = !advMode ? Settings::KeyPressVisFadeLineLen : 
        Settings::KeyPressVisAdvFadeLineLen[mBtnIdx];
    const auto minHeight = !advMode ? Settings::KeyPressFixedHeight :
        Settings::KeyPressAdvFixedHeight[mBtnIdx];

    // Iterate through all rectangles
    for (auto i : mUsedRectIndices)
    {
        // Flag which identifies if all the rectangle vertices are on the same height
        auto eachVertexIsOnLimit = true;

        // Iterate through all the rectangle vertices
        const auto vertexIndex = i * 4ul;
        for (auto j = vertexIndex; j < vertexIndex + 4ul; ++j)
        {
            // Take vertex reference
            // auto &topSideVertex = mTopVertecies[j];
            auto &middleVertex = mMiddleVertecies[j];
            // auto &bottomSideVertex = mBottomVertecies[j];

            // Limit the square to go beyond the line length
            auto move = [len, speed, deltaSeconds] (sf::Vertex &vertex)
                {
                    vertex.position.y = -std::min(std::abs(vertex.position.y + speed * deltaSeconds * getConstantSpeedScale()), len);
                };
            // move(topSideVertex);
            move(middleVertex);
            // move(bottomSideVertex);

            // Check if the current vertex is on the max height, do so only if previous were so
            // if (eachVertexIsOnLimit)
            // {
            //     eachVertexIsOnLimit = std::abs(bottomSideVertex.position.y) 
            //         == len;
            // }
            if (eachVertexIsOnLimit)
            {
                eachVertexIsOnLimit = std::abs(middleVertex.position.y) == len;
            }

            // Set the right alpha channel depending on the progress to the end of the fade out length line
            // topSideVertex.color = getVertexColor(mTopVertecies, j);
            middleVertex.color = getVertexColor(mMiddleVertecies, j);
            // bottomSideVertex.color = getVertexColor(mBottomVertecies, j);
        }

        // All vertices are on the same height
        if (eachVertexIsOnLimit)
        {
            // Add the rectangle index to available index list, add to list of rectangles to remove
            mAvailableRectIndices.emplace_back(i);
            toRemove.emplace_back(i);
        }
    }

    // Iterate through all the indices of the rectangles to remove
    for (auto i : toRemove)
    {
        // Remove every index that is equal to i
        mUsedRectIndices.erase(std::remove(
                mUsedRectIndices.begin(), mUsedRectIndices.end(), i), 
            mUsedRectIndices.end());
    }

    // If a button is pressed don't let the spawning rectangle go away from the spawn point
    if (keyState)
    {
        const auto offset = mUsedRectIndices.back() * 4ul;
        mMiddleVertecies[offset + 2ul].position.y = 
        mMiddleVertecies[offset + 3ul].position.y -= speed * deltaSeconds * getConstantSpeedScale();

        if (minHeight > 0 && std::abs(mMiddleVertecies[offset].position.y) > minHeight)
        {
            mMiddleVertecies[offset + 2ul].position.y = 
            mMiddleVertecies[offset + 3ul].position.y = mMiddleVertecies[offset].position.y + minHeight;
        }
    }

    // Move the nearest rectangle up on release
    if (prevKeyState && !keyState && !mUsedRectIndices.empty())
    {
        const auto offset = mUsedRectIndices.back() * 4ul;

        mMiddleVertecies[offset + 2ul].position.y = 
        mMiddleVertecies[offset + 3ul].position.y -= speed * deltaSeconds * getConstantSpeedScale();

        if (minHeight > 0 && std::abs(mMiddleVertecies[offset].position.y) > minHeight)
        {
            mMiddleVertecies[offset + 2ul].position.y = 
            mMiddleVertecies[offset + 3ul].position.y = mMiddleVertecies[offset].position.y + minHeight;
        }
    }
}

void GfxButton::RectEmitter::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform = getPressRectTransform(states.transform);

    target.draw(mMiddleVertecies, states);

    // states.texture = &mTexture;
    // target.draw(mTopVertecies, states);
    // target.draw(mBottomVertecies, states);
}

void GfxButton::RectEmitter::setPosition(sf::Vector2f position)
{
    mEmitterPosition = position;
}

void GfxButton::RectEmitter::pushVertecies(sf::VertexArray &vertexArray, sf::Vertex *toPush, size_t offset, sf::Vector2f buttonSize)
{
    for (auto i = 0ul; i < 4ul; ++i)
    {
        // Take reference
        auto &vertex = toPush[i];

        // Move the position to the emitter's origin
        vertex.position += mEmitterPosition - sf::Vector2f(0.f, buttonSize.y / 2.f);

        const auto idx = offset + i;

        // Change the color
        vertex.color = getVertexColor(vertexArray, idx);        
        // Assign the created vertex to the contrainer
        vertexArray[idx] = vertex;
    }
}

float GfxButton::RectEmitter::getConstantSpeedScale()
{
	// Originally the speed was 60px per frame, and the frame rate was capped to 60, 
	// so by default it was travelling 3600px/s.
	// Now that rendering frame rate is dynamic, the "speed" property defines
	// pixels per second, not pixels per frame; to keep it compatible with older versions
	// in terms of speed, convert the px/f speed to px/s
	return 60.f;
}

void GfxButton::RectEmitter::create(float deltaSeconds, sf::Vector2f buttonSize)
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto origSpeed = !advMode ? Settings::KeyPressVisSpeed : 
        Settings::KeyPressVisAdvSpeed[mBtnIdx];
    const auto speed = (-origSpeed * deltaSeconds * getConstantSpeedScale()) / 10.f;
    // const auto wScale = (!advMode ? Settings::KeyPressWidthScale : Settings::KeyPressAdvWidthScale[mBtnIdx]) / 100.f;
    // buttonSize.x *= wScale;

    const auto rectSize = sf::Vector2f(buttonSize.x, speed);
    const auto halfRectSize = rectSize / 2.f;

    // const auto textureSize = static_cast<sf::Vector2f>(mTexture.getSize());

    const auto rectIndex = mAvailableRectIndices.back();
    const auto firstVertexIndex = rectIndex * 4ul;

    // 0 Top left, 1 Top right, 2 Bottom right, 3 Bottom left
    sf::Vertex middleVertices[4];

    // middleVertices[0].position = sf::Vector2f(-halfRectSize.x, -halfRectSize.y);
    // middleVertices[1].position = sf::Vector2f(+halfRectSize.x, -halfRectSize.y);
    // middleVertices[2].position = sf::Vector2f(+halfRectSize.x, -halfRectSize.y);
    // middleVertices[3].position = sf::Vector2f(-halfRectSize.x, -halfRectSize.y);

    middleVertices[0].position = sf::Vector2f(-halfRectSize.x, -rectSize.y);
    middleVertices[1].position = sf::Vector2f(+halfRectSize.x, -rectSize.y);
    middleVertices[2].position = sf::Vector2f(+halfRectSize.x, 0.f);
    middleVertices[3].position = sf::Vector2f(-halfRectSize.x, 0.f);

    pushVertecies(mMiddleVertecies, middleVertices, firstVertexIndex, buttonSize);
}

void GfxButton::RectEmitter::scaleTexture(sf::Vector2f buttonSize)
{
    // const auto size = static_cast<sf::Vector2f>(mTexture.getSize());
    // mTextureScale = sf::Vector2f(size.x / buttonSize.x, size.x / buttonSize.y);
}

sf::Transform GfxButton::RectEmitter::getPressRectTransform(sf::Transform transform) const
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto rot = !advMode ? Settings::KeyPressVisRotation : 
        Settings::KeyPressVisAdvRotation[mBtnIdx];
    const auto orig = Settings::KeyPressVisOrig + (advMode 
        ? Settings::KeyPressVisAdvOrig[mBtnIdx] : sf::Vector2f());
    const auto wScale = (!advMode ? Settings::KeyPressWidthScale : Settings::KeyPressAdvWidthScale[mBtnIdx]) / 100.f;

    transform.rotate(-rot);
    transform.translate(Utility::swapY(orig));
    transform.scale(wScale, 1.f);
    return transform;
}

float GfxButton::RectEmitter::getVertexProgress(size_t vertexNumber, float vertexHeight) const
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto len = !advMode ? Settings::KeyPressVisFadeLineLen : 
        Settings::KeyPressVisAdvFadeLineLen[mBtnIdx];

    return std::min(mEmitterPosition.y - vertexHeight / len, 1.f);
}

 sf::Color GfxButton::RectEmitter::getVertexColor(const sf::VertexArray &vertexArray, size_t vertexIndex) const
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    auto color = !advMode ? Settings::KeyPressVisColor : 
        Settings::KeyPressVisAdvColor[mBtnIdx];

    color.a -= color.a * getVertexProgress(vertexIndex, vertexArray[vertexIndex].position.y);
    return color;
}

        text->setFillColor(color);
        text->setCharacterSize(chSz);
        text->setPosition(pos);
        text->setStyle(sf::Uint32((bold ? sf::Text::Bold : 0) | (italic ? sf::Text::Italic : 0)));
        text->setOutlineThickness(outThck);
        text->setOutlineColor(outColor);

        const auto lAlt = Settings::ShowOppOnAlt && sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt);
        if ((lAlt && idx == VisualKey && Settings::ButtonTextShowVisualKeys && !Settings::ButtonTextShowTotal)
        || (lAlt && idx == KeyCounter && !Settings::ButtonTextShowVisualKeys && Settings::ButtonTextShowTotal))
        {
            text->setPosition(pos);
        }

        ++idx;
    }

    const auto size = !advTextMode ? Settings::ButtonTextBounds : Settings::ButtonTextAdvBounds[mBtnIdx];
    mBounds.setSize(size);
    mBounds.setOrigin(size / 2.f);
}

void GfxButton::resetAssets()
{
    mSprites[ButtonSprite]->setTexture(mTextures.get(Textures::Button), true);
    mSprites[AnimationSprite]->setTexture(mTextures.get(Textures::Animation), true);

    const auto &font = mFonts.get(Fonts::ButtonValue);
    for (auto &text : mTexts)
        text->setFont(font);
}

void GfxButton::scaleSprites()
{
    auto &buttonSprite = *mSprites[ButtonSprite];
    auto &animationSprite = *mSprites[AnimationSprite];

    const auto origBtnTxtrSz = buttonSprite.getTexture()->getSize();
    const auto origAniTxtrSz = animationSprite.getTexture()->getSize();

    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::GfxButtonAdvancedMode;

    const auto btnTxtrSz = !advMode ? static_cast<sf::Vector2f>(Settings::GfxButtonTextureSize) : Settings::GfxButtonsSizes[mBtnIdx];
    const auto btnTxtrScale = sf::Vector2f(btnTxtrSz.x / origBtnTxtrSz.x, btnTxtrSz.y / origBtnTxtrSz.y);
    const auto aniTxtrScale = sf::Vector2f(btnTxtrSz.x / origAniTxtrSz.x, btnTxtrSz.y / origAniTxtrSz.y);

    buttonSprite.setScale(btnTxtrScale);
    animationSprite.setScale(aniTxtrScale);
}

bool isInBounds(sf::Vector2f bounds, sf::FloatRect rect)
{
    return rect.width > bounds.x || rect.height > bounds.y;
}

void GfxButton::keepInBounds(sf::Text &text)
{
    if (!Settings::ButtonTextBoundsToggle)
        return;
        
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::ButtonTextAdvancedMode;
    const auto bounds = !advMode ? Settings::ButtonTextBounds : Settings::ButtonTextAdvBounds[mBtnIdx];
    auto rect = text.getLocalBounds();

    for (auto chSz = text.getCharacterSize(); 
        chSz > 2u && isInBounds(bounds, rect); 
        --chSz)
    {
        text.setCharacterSize(chSz);
        rect = text.getLocalBounds();
    }
}

sf::Vector2f getTextCenter(const sf::Text &text)
{
    const auto rect = text.getLocalBounds();
    return { rect.left + rect.width / 2, rect.top + rect.height / 2};
}

void GfxButton::centerOrigins()
{
    auto &buttonSprite = *mSprites[ButtonSprite];
    auto &animationSprite = *mSprites[AnimationSprite];
    const auto buttonTextureSize = static_cast<sf::Vector2f>(buttonSprite.getTexture()->getSize());
    const auto animationTextureSize = static_cast<sf::Vector2f>(animationSprite.getTexture()->getSize());
    buttonSprite.setOrigin(buttonTextureSize / 2.f);
    animationSprite.setOrigin(animationTextureSize / 2.f);

    for (auto &text : mTexts)
        text->setOrigin(getTextCenter(*text));
}

float GfxButton::getWidth(unsigned idx)
{
    const float width = Settings::WindowBonusSizeLeft + 
        (Settings::GfxButtonTextureSize.x + Settings::GfxButtonDistance) * idx + 
        Settings::GfxButtonTextureSize.x / 2;
    return width;
}

float GfxButton::getHeight(unsigned idx)
{
    const float height = Settings::WindowBonusSizeTop + Settings::GfxButtonTextureSize.y / 2;
    return height;
}

GfxButton::TextID GfxButton::getTextIdToDisplay()
{
    if (Settings::ButtonTextShowVisualKeys) 
        return VisualKey;
    if (Settings::ButtonTextShowTotal)
        return KeyCounter;
    if (Settings::ButtonTextShowKPS)
        return KeyPerSecond;
    if (Settings::ButtonTextShowBPM)
        return BeatsPerMinute;
    
    return Nothing;
}

void GfxButton::setShowBounds(bool flag, int idx)
{
    mShowBounds = flag;
    mSelectedKeyBounds = idx;
}

GfxButton::~GfxButton()
{    
}

// GfxButton::RectEmitter::RectEmitter(const sf::Texture &texture)
GfxButton::RectEmitter::RectEmitter(unsigned btnIdx)
: mBtnIdx(btnIdx)
//, mTexture(texture) 
// , mTopVertecies(sf::Quads, 1000u)
, mMiddleVertecies(sf::Quads, 1000u)
// , mBottomVertecies(sf::Quads, 1000u)
{
    const auto count = mMiddleVertecies.getVertexCount() / 4;
    for (auto i = 0ul; i < count; ++i)
        mAvailableRectIndices.emplace_back(i);
}

void GfxButton::RectEmitter::update(float deltaSeconds, bool keyState, bool prevKeyState)
{
    // Don't update anything if there is no active rectangles
    if (mUsedRectIndices.empty())
        return;

    std::vector<size_t> toRemove;

    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto origSpeed = !advMode ? Settings::KeyPressVisSpeed : 
        Settings::KeyPressVisAdvSpeed[mBtnIdx];
    const auto speed = (-origSpeed * deltaSeconds * getConstantSpeedScale()) / 10.f;
    const auto len = !advMode ? Settings::KeyPressVisFadeLineLen : 
        Settings::KeyPressVisAdvFadeLineLen[mBtnIdx];
    const auto minHeight = !advMode ? Settings::KeyPressFixedHeight :
        Settings::KeyPressAdvFixedHeight[mBtnIdx];

    // Iterate through all rectangles
    for (auto i : mUsedRectIndices)
    {
        // Flag which identifies if all the rectangle vertices are on the same height
        auto eachVertexIsOnLimit = true;

        // Iterate through all the rectangle vertices
        const auto vertexIndex = i * 4ul;
        for (auto j = vertexIndex; j < vertexIndex + 4ul; ++j)
        {
            // Take vertex reference
            // auto &topSideVertex = mTopVertecies[j];
            auto &middleVertex = mMiddleVertecies[j];
            // auto &bottomSideVertex = mBottomVertecies[j];

            // Limit the square to go beyond the line length
            auto move = [len, speed, deltaSeconds] (sf::Vertex &vertex)
                {
                    vertex.position.y = -std::min(std::abs(vertex.position.y + speed * deltaSeconds * getConstantSpeedScale()), len);
                };
            // move(topSideVertex);
            move(middleVertex);
            // move(bottomSideVertex);

            // Check if the current vertex is on the max height, do so only if previous were so
            // if (eachVertexIsOnLimit)
            // {
            //     eachVertexIsOnLimit = std::abs(bottomSideVertex.position.y) 
            //         == len;
            // }
            if (eachVertexIsOnLimit)
            {
                eachVertexIsOnLimit = std::abs(middleVertex.position.y) == len;
            }

            // Set the right alpha channel depending on the progress to the end of the fade out length line
            // topSideVertex.color = getVertexColor(mTopVertecies, j);
            middleVertex.color = getVertexColor(mMiddleVertecies, j);
            // bottomSideVertex.color = getVertexColor(mBottomVertecies, j);
        }

        // All vertices are on the same height
        if (eachVertexIsOnLimit)
        {
            // Add the rectangle index to available index list, add to list of rectangles to remove
            mAvailableRectIndices.emplace_back(i);
            toRemove.emplace_back(i);
        }
    }

    // Iterate through all the indices of the rectangles to remove
    for (auto i : toRemove)
    {
        // Remove every index that is equal to i
        mUsedRectIndices.erase(std::remove(
                mUsedRectIndices.begin(), mUsedRectIndices.end(), i), 
            mUsedRectIndices.end());
    }

    // If a button is pressed don't let the spawning rectangle go away from the spawn point
    if (keyState)
    {
        const auto offset = mUsedRectIndices.back() * 4ul;
        mMiddleVertecies[offset + 2ul].position.y = 
        mMiddleVertecies[offset + 3ul].position.y -= speed * deltaSeconds * getConstantSpeedScale();

        if (minHeight > 0 && std::abs(mMiddleVertecies[offset].position.y) > minHeight)
        {
            mMiddleVertecies[offset + 2ul].position.y = 
            mMiddleVertecies[offset + 3ul].position.y = mMiddleVertecies[offset].position.y + minHeight;
        }
    }

    // Move the nearest rectangle up on release
    if (prevKeyState && !keyState && !mUsedRectIndices.empty())
    {
        const auto offset = mUsedRectIndices.back() * 4ul;

        mMiddleVertecies[offset + 2ul].position.y = 
        mMiddleVertecies[offset + 3ul].position.y -= speed * deltaSeconds * getConstantSpeedScale();

        if (minHeight > 0 && std::abs(mMiddleVertecies[offset].position.y) > minHeight)
        {
            mMiddleVertecies[offset + 2ul].position.y = 
            mMiddleVertecies[offset + 3ul].position.y = mMiddleVertecies[offset].position.y + minHeight;
        }
    }
}

void GfxButton::RectEmitter::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    states.transform = getPressRectTransform(states.transform);

    target.draw(mMiddleVertecies, states);

    // states.texture = &mTexture;
    // target.draw(mTopVertecies, states);
    // target.draw(mBottomVertecies, states);
}

void GfxButton::RectEmitter::setPosition(sf::Vector2f position)
{
    mEmitterPosition = position;
}

void GfxButton::RectEmitter::pushVertecies(sf::VertexArray &vertexArray, sf::Vertex *toPush, size_t offset, sf::Vector2f buttonSize)
{
    for (auto i = 0ul; i < 4ul; ++i)
    {
        // Take reference
        auto &vertex = toPush[i];

        // Move the position to the emitter's origin
        vertex.position += mEmitterPosition - sf::Vector2f(0.f, buttonSize.y / 2.f);

        const auto idx = offset + i;

        // Change the color
        vertex.color = getVertexColor(vertexArray, idx);
        
        // Assign the created vertex to the contrainer
        vertexArray[idx] = vertex;
    }
}

float GfxButton::RectEmitter::getConstantSpeedScale()
{
	// Originally the speed was 60px per frame, and the frame rate was capped to 60, 
	// so by default it was travelling 3600px/s.
	// Now that rendering frame rate is dynamic, the "speed" property defines
	// pixels per second, not pixels per frame; to keep it compatible with older versions
	// in terms of speed, convert the px/f speed to px/s
	return 60.f;
}

void GfxButton::RectEmitter::create(float deltaSeconds, sf::Vector2f buttonSize)
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto origSpeed = !advMode ? Settings::KeyPressVisSpeed : 
        Settings::KeyPressVisAdvSpeed[mBtnIdx];
    const auto speed = (-origSpeed * deltaSeconds * getConstantSpeedScale()) / 10.f;
    // const auto wScale = (!advMode ? Settings::KeyPressWidthScale : Settings::KeyPressAdvWidthScale[mBtnIdx]) / 100.f;
    // buttonSize.x *= wScale;

    const auto rectSize = sf::Vector2f(buttonSize.x, speed);
    const auto halfRectSize = rectSize / 2.f;

    // const auto textureSize = static_cast<sf::Vector2f>(mTexture.getSize());

    const auto rectIndex = mAvailableRectIndices.back();
    const auto firstVertexIndex = rectIndex * 4ul;

    // 0 Top left, 1 Top right, 2 Bottom right, 3 Bottom left
    sf::Vertex middleVertices[4];

    // middleVertices[0].position = sf::Vector2f(-halfRectSize.x, -halfRectSize.y);
    // middleVertices[1].position = sf::Vector2f(+halfRectSize.x, -halfRectSize.y);
    // middleVertices[2].position = sf::Vector2f(+halfRectSize.x, -halfRectSize.y);
    // middleVertices[3].position = sf::Vector2f(-halfRectSize.x, -halfRectSize.y);

    middleVertices[0].position = sf::Vector2f(-halfRectSize.x, -rectSize.y);
    middleVertices[1].position = sf::Vector2f(+halfRectSize.x, -rectSize.y);
    middleVertices[2].position = sf::Vector2f(+halfRectSize.x, 0.f);
    middleVertices[3].position = sf::Vector2f(-halfRectSize.x, 0.f);

    pushVertecies(mMiddleVertecies, middleVertices, firstVertexIndex, buttonSize);


    // sf::Vertex topVertices[4];

    // topVertices[0].position = sf::Vector2f(-halfRectSize.x, -halfRectSize.y * 2.f);
    // topVertices[1].position = sf::Vector2f(+halfRectSize.x, -halfRectSize.y * 2.f);
    // topVertices[2].position = sf::Vector2f(+halfRectSize.x, -halfRectSize.y);
    // topVertices[3].position = sf::Vector2f(-halfRectSize.x, -halfRectSize.y);
    
    // // Make the texture be rotated
    // topVertices[0].texCoords = sf::Vector2f(+textureSize.x, +textureSize.y);
    // topVertices[1].texCoords = sf::Vector2f(0.f,            +textureSize.y);
    // topVertices[2].texCoords = sf::Vector2f(0.f,            0.f);
    // topVertices[3].texCoords = sf::Vector2f(+textureSize.x, 0.f);

    // pushVertecies(mTopVertecies, topVertices, firstVertexIndex, buttonSize);


    // sf::Vertex bottomVertices[4];

    // bottomVertices[0].position = sf::Vector2f(-halfRectSize.x, -halfRectSize.y);
    // bottomVertices[1].position = sf::Vector2f(+halfRectSize.x, -halfRectSize.y);
    // bottomVertices[2].position = sf::Vector2f(+halfRectSize.x, 0.f);
    // bottomVertices[3].position = sf::Vector2f(-halfRectSize.x, 0.f);
    
    // bottomVertices[0].texCoords = sf::Vector2f(0.f,            0.f);
    // bottomVertices[1].texCoords = sf::Vector2f(+textureSize.x, 0.f);
    // bottomVertices[2].texCoords = sf::Vector2f(+textureSize.x, +textureSize.y);
    // bottomVertices[3].texCoords = sf::Vector2f(0.f,            +textureSize.y);

    // pushVertecies(mBottomVertecies, bottomVertices, firstVertexIndex, buttonSize);

    
    // Remove the used index from the available rect indices list 
    // and push it to the used one
    mAvailableRectIndices.pop_back();
    mUsedRectIndices.emplace_back(rectIndex);
}

void GfxButton::RectEmitter::scaleTexture(sf::Vector2f buttonSize)
{
    // const auto size = static_cast<sf::Vector2f>(mTexture.getSize());
    // mTextureScale = sf::Vector2f(size.x / buttonSize.x, size.x / buttonSize.y);
}

sf::Transform GfxButton::RectEmitter::getPressRectTransform(sf::Transform transform) const
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto rot = !advMode ? Settings::KeyPressVisRotation : 
        Settings::KeyPressVisAdvRotation[mBtnIdx];
    const auto orig = Settings::KeyPressVisOrig + (advMode 
        ? Settings::KeyPressVisAdvOrig[mBtnIdx] : sf::Vector2f());
    const auto wScale = (!advMode ? Settings::KeyPressWidthScale : Settings::KeyPressAdvWidthScale[mBtnIdx]) / 100.f;

    transform.rotate(-rot);
    transform.translate(Utility::swapY(orig));
    transform.scale(wScale, 1.f);
    return transform;
}

float GfxButton::RectEmitter::getVertexProgress(size_t vertexNumber, float vertexHeight) const
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    const auto len = !advMode ? Settings::KeyPressVisFadeLineLen : 
        Settings::KeyPressVisAdvFadeLineLen[mBtnIdx];

    return std::min(mEmitterPosition.y - vertexHeight / len, 1.f);
}

sf::Color GfxButton::RectEmitter::getVertexColor(const sf::VertexArray &vertexArray, size_t vertexIndex) const
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::KeyPressVisAdvSettingsMode;
    auto color = !advMode ? Settings::KeyPressVisColor : 
        Settings::KeyPressVisAdvColor[mBtnIdx];

    color.a -= color.a * getVertexProgress(vertexIndex, vertexArray[vertexIndex].position.y);
    return color;
}
