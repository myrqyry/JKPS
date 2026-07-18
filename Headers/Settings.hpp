#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Graphics/Color.hpp>

#include "GfxStatisticsLine.hpp"

#include <string>
#include <array>


namespace Settings
{
    const size_t SupportedAdvancedKeysNumber = 20ul;
    const size_t OldSupportedAdvancedKeysNumber = 15ul;
    template <typename T>
    using Container = std::array<T, SupportedAdvancedKeysNumber>;
    template <typename T>
    using StatisticsContainer = std::array<T, GfxStatisticsLine::StatisticsIdCounter>;

    // [Statistics text]
    extern float StatisticsTextDistance;
    extern sf::Vector2f StatisticsTextPosition;
    extern sf::Vector2f StatisticsTextValuePosition;
    extern bool StatisticsTextCenterOrigin;
    extern std::string StatisticsTextFontPath;
    extern sf::Color StatisticsTextColor;
    extern unsigned StatisticsTextCharacterSize;
    extern unsigned StatisticsTextOutlineThickness;
    extern sf::Color StatisticsTextOutlineColor;
    extern bool StatisticsTextBold;
    extern bool StatisticsTextItalic;
    extern bool ShowStatisticsText;
	extern bool ShowStatisticsKPS;
	extern bool ShowStatisticsMaxKPS;
    extern bool ShowStatisticsTotal;
    extern bool ShowStatisticsBPM;

    extern bool StatisticsTextAdvancedMode;
    extern StatisticsContainer<sf::Vector2f> StatisticsTextAdvPosition;
    extern StatisticsContainer<sf::Vector2f> StatisticsTextAdvValuePosition;
    extern StatisticsContainer<bool> StatisticsTextAdvCenterOrigin;
    extern StatisticsContainer<sf::Color> StatisticsTextAdvColor;
    extern StatisticsContainer<unsigned> StatisticsTextAdvCharacter;
    extern StatisticsContainer<bool> StatisticsTextAdvBold;
    extern StatisticsContainer<bool> StatisticsTextAdvItalic;
    extern std::string StatisticsKPSText;
    extern std::string StatisticsKPS2Text;
    extern std::string StatisticsTotalText;
    extern std::string StatisticsBPMText;

    // [Button text]
    extern std::string ButtonTextFontPath;
    extern sf::Color ButtonTextColor;
    extern unsigned ButtonTextCharacterSize;
    extern unsigned ButtonTextOutlineThickness;
    extern sf::Color ButtonTextOutlineColor;
    extern sf::Vector2f ButtonTextPosition;
    extern bool ButtonTextBoundsToggle;
    extern sf::Vector2f ButtonTextBounds;
    extern bool ButtonTextIgnoreBtnMovement;
    extern bool ButtonTextBold;
    extern bool ButtonTextItalic;
    extern bool ButtonTextShowVisualKeys;
    extern sf::Vector2f ButtonTextVisualKeysTextPosition;
    extern bool ButtonTextShowTotal;
    extern sf::Vector2f ButtonTextTotalTextPosition;
    extern bool ButtonTextShowKPS;
    extern sf::Vector2f ButtonTextKPSTextPosition;
    extern bool ButtonTextShowBPM;
    extern sf::Vector2f ButtonTextBPMTextPosition;

    // [Buttons text advanced settings]
    extern bool ButtonTextSepPosAdvancedMode;
    extern Container<sf::Vector2f> ButtonTextAdvVisualKeysTextPosition;
    extern Container<sf::Vector2f> ButtonTextAdvTotalTextPosition;
    extern Container<sf::Vector2f> ButtonTextAdvKPSTextPosition;
    extern Container<sf::Vector2f> ButtonTextAdvBPMTextPosition;
    
    extern bool ButtonTextAdvancedMode;
    extern Container<sf::Color> ButtonTextAdvColor;
    extern Container<unsigned> ButtonTextAdvCharacterSize;
    extern Container<unsigned> ButtonTextAdvOutlineThickness;
    extern Container<sf::Color> ButtonTextAdvOutlineColor;
    extern Container<sf::Vector2f> ButtonsTextAdvPosition;
    extern Container<sf::Vector2f> ButtonTextAdvBounds;
    extern Container<bool> ButtonTextAdvBold;
    extern Container<bool> ButtonTextAdvItalic;

    // [Button graphics]
    extern float GfxButtonDistance;
    extern std::string GfxButtonTexturePath;
    extern sf::Vector2u GfxButtonTextureSize;
    extern sf::Color GfxButtonTextureColor;
    extern sf::Color GfxButtonBorderColor;
    extern int ButtonShape;

    // [Button graphics advanced settings]
    extern bool GfxButtonAdvancedMode;
    extern Container<sf::Vector2f> GfxButtonsBtnPositions;
    extern Container<sf::Vector2f> GfxButtonsSizes;
    extern Container<sf::Color> GfxButtonsColor;

    // [Button graphics scaling]
    extern float GfxButtonScaleFactor;
    extern float GfxButtonAnimationScaleFactor;
    extern float GfxButtonTextScaleFactor;
    extern float GfxButtonMinScale;
    extern float GfxButtonMaxScale;
    extern Container<float> GfxButtonAdvScaleFactor;
    extern Container<float> GfxButtonAdvAnimationScaleFactor;
    extern Container<float> GfxButtonAdvTextScaleFactor;

    // [Theme system]
    extern int CurrentThemeIndex;

    // [Overlay settings]
    extern bool IsOverlayApplication;
    extern float OverlayScaleFactor;
    extern float GlobalScaleFactor;

    // [Animation graphics]
    extern bool LightAnimation;
    extern bool PressAnimation;
    extern std::string AnimationTexturePath;
    extern unsigned AnimationFrames;
    extern sf::Vector2f AnimationScale;
    extern sf::Color AnimationColor;
    extern float AnimationOffset;

    // [Background]
    extern std::string BackgroundTexturePath;
    extern sf::Color BackgroundColor;
    extern bool ScaleBackground;

    // Non config parameters
    extern bool isGreenscreenSet;

    // [Main window]
    namespace Window
    {
        extern bool WindowTitleBar;
        extern bool WindowResizable;
	extern unsigned RenderUpdateFrequency;
        extern int WindowBonusSizeTop;
        extern int WindowBonusSizeBottom;
        extern int WindowBonusSizeLeft;
        extern int WindowBonusSizeRight;
    }

    inline bool &WindowTitleBar = Window::WindowTitleBar;
    inline bool &WindowResizable = Window::WindowResizable;
	inline unsigned &RenderUpdateFrequency = Window::RenderUpdateFrequency;
    inline int &WindowBonusSizeTop = Window::WindowBonusSizeTop;
    inline int &WindowBonusSizeBottom = Window::WindowBonusSizeBottom;
    inline int &WindowBonusSizeLeft = Window::WindowBonusSizeLeft;
    inline int &WindowBonusSizeRight = Window::WindowBonusSizeRight;

    // [KPS window]
    extern bool KPSWindowEnabledFromStart;
    extern sf::Vector2u KPSWindowSize;
    extern sf::Color KPSBackgroundColor;
    extern sf::Color KPSTextColor;
    extern sf::Color KPSNumberColor;
    extern std::string KPSWindowTextFontPath;
    extern std::string KPSWindowNumberFontPath;
    extern float KPSWindowTopPadding;
    extern float KPSWindowDistanceBetween;
    extern unsigned KPSTextSize;
    extern unsigned KPSNumberSize;

    // [Key press visualization]
    extern bool KeyPressVisToggle;
    extern float KeyPressVisSpeed;
    extern float KeyPressVisRotation;
    extern float KeyPressVisFadeLineLen;
    extern sf::Vector2f KeyPressVisOrig;
    extern sf::Color KeyPressVisColor;
    extern float KeyPressWidthScale;
    extern float KeyPressFixedHeight;

    // [Key press visualization advanced settings]
    extern bool KeyPressVisAdvSettingsMode;
    extern Container<float> KeyPressVisAdvSpeed;
    extern Container<float> KeyPressVisAdvRotation;
    extern Container<float> KeyPressVisAdvFadeLineLen;
    extern Container<sf::Vector2f> KeyPressVisAdvOrig;
    extern Container<sf::Color> KeyPressVisAdvColor;
    extern Container<float> KeyPressAdvWidthScale;
    extern Container<float> KeyPressAdvFixedHeight;

    // [Other]
    namespace Other
    {
        extern bool SaveStats;
        extern bool ShowOppOnAlt;
        extern unsigned ButtonPressMultiplier;
        extern bool ReduceMotion;
    }

    inline bool &SaveStats = Other::SaveStats;
    inline bool &ShowOppOnAlt = Other::ShowOppOnAlt;
    inline unsigned &ButtonPressMultiplier = Other::ButtonPressMultiplier;
    inline bool &ReduceMotion = Other::ReduceMotion;

    // Default assets
    extern unsigned char* KeyCountersDefaultFont;
    extern unsigned char* StatisticsDefaultFont;
    extern unsigned char* DefaultButtonTexture;
    extern unsigned char* DefaultAnimationTexture;
    extern unsigned char* DefaultBackgroundTexture;
    extern unsigned char* DefaultGreenscreenBackgroundTexture;
    extern unsigned char* DefaultKPSWindowFont;
    extern unsigned char* KeyPressVisTexture;

    // Hot keys
    extern sf::Keyboard::Key KeyToIncreaseKeys;
    extern sf::Keyboard::Key AltKeyToIncreaseKeys;
    extern sf::Keyboard::Key KeyToDecreaseKeys;
    extern sf::Keyboard::Key AltKeyToDecreaseKeys;
    extern sf::Keyboard::Key KeyToIncreaseButtons;
    extern sf::Keyboard::Key KeyToDecreaseButtons;
    extern sf::Keyboard::Key KeyToReset;
    extern sf::Keyboard::Key KeyExit;
    extern sf::Keyboard::Key KeyToOpenKPSWindow;
    extern sf::Keyboard::Key KeyToOpenMenuWindow;
    extern sf::Keyboard::Key KeyToOpenGraphWindow;
    extern sf::Keyboard::Key KeyToOpenStyleWizard;

    // Saved parameters
    extern float MaxKPS;
    extern unsigned Total;
    extern Container<unsigned> KeysTotal;

    // [UI design tokens]
    // Material Design 3 Expressive semantic palette and spacing system.
    // Inspired by the M3 dark scheme adapted for JKPS's blue identity.
    // Layout code should prefer these tokens so styling stays coherent.
    namespace UiTokens
    {
        // --- Surface colors (M3 dark elevation system) ---
        extern sf::Color Surface;
        extern sf::Color SurfaceDim;
        extern sf::Color SurfaceBright;
        extern sf::Color SurfaceContainerLowest;
        extern sf::Color SurfaceContainerLow;
        extern sf::Color SurfaceContainer;
        extern sf::Color SurfaceContainerHigh;
        extern sf::Color SurfaceContainerHighest;
        extern sf::Color OnSurface;
        extern sf::Color OnSurfaceVariant;

        // --- Primary colors (blue accent) ---
        extern sf::Color Primary;
        extern sf::Color OnPrimary;
        extern sf::Color PrimaryContainer;
        extern sf::Color OnPrimaryContainer;

        // --- Secondary colors (muted blue) ---
        extern sf::Color Secondary;
        extern sf::Color OnSecondary;

        // --- Outline colors ---
        extern sf::Color Outline;
        extern sf::Color OutlineVariant;

        // --- Error colors ---
        extern sf::Color Error;
        extern sf::Color OnError;

        // --- Focus indicator ---
        extern sf::Color Focus;

        // --- Typography (px) ---
        extern unsigned TextSizeHeader;
        extern unsigned TextSizeBody;
        extern unsigned TextSizeSmall;

        // --- Spacing (4px base unit) ---
        extern float SpacingXs;
        extern float SpacingSm;
        extern float SpacingMd;
        extern float SpacingLg;
        extern float SpacingXl;

        // --- Corner radius ---
        extern float RadiusSm;
        extern float RadiusMd;
        extern float RadiusLg;
        extern float RadiusXl;

        // --- Backward-compatible aliases ---
        // Legacy names redirect to equivalent M3E tokens so existing
        // code compiles without changes while the codebase migrates.
        inline float      &BaseSpacing                     = SpacingMd;
        inline unsigned   &PrimaryTextSize                 = TextSizeBody;
        inline unsigned   &SecondaryTextSize               = TextSizeSmall;
        inline sf::Color  &SurfaceColor                    = Surface;
        inline sf::Color  &AccentColor                     = Primary;
        inline sf::Color  &BorderColor                     = OutlineVariant;
        inline sf::Color  &CustomizationSurface            = Surface;
        inline sf::Color  &CustomizationSurfaceVariant     = SurfaceContainer;
        inline sf::Color  &CustomizationAccent             = Primary;
        inline sf::Color  &CustomizationOutline            = OutlineVariant;
        inline sf::Color  &CustomizationFocus              = Focus;
        inline unsigned   &CustomizationHeaderTextSize     = TextSizeHeader;
        inline unsigned   &CustomizationBodyTextSize       = TextSizeBody;
        inline float      &CustomizationSpacing            = SpacingMd;
        inline float      &CustomizationRadius             = RadiusMd;
    }
}
