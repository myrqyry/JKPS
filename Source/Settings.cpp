#include "../Headers/Settings.hpp"
#include "../Headers/DefaultFiles.hpp"


namespace Settings
{

// [Statistics text]
float StatisticsTextDistance;
sf::Vector2f StatisticsTextPosition;
sf::Vector2f StatisticsTextValuePosition;
bool StatisticsTextCenterOrigin;
std::string StatisticsTextFontPath;
sf::Color StatisticsTextColor;
unsigned StatisticsTextCharacterSize;
unsigned StatisticsTextOutlineThickness;
sf::Color StatisticsTextOutlineColor;
bool StatisticsTextBold;
bool StatisticsTextItalic;
bool ShowStatisticsText;
bool ShowStatisticsKPS;
bool ShowStatisticsMaxKPS;
bool ShowStatisticsTotal;
bool ShowStatisticsBPM;

bool StatisticsTextAdvancedMode;
StatisticsContainer<sf::Vector2f> StatisticsTextAdvPosition;
StatisticsContainer<sf::Vector2f> StatisticsTextAdvValuePosition;
StatisticsContainer<bool> StatisticsTextAdvCenterOrigin;
StatisticsContainer<sf::Color> StatisticsTextAdvColor;
StatisticsContainer<unsigned> StatisticsTextAdvCharacter;
StatisticsContainer<bool> StatisticsTextAdvBold;
StatisticsContainer<bool> StatisticsTextAdvItalic;
std::string StatisticsKPSText;
std::string StatisticsKPS2Text;
std::string StatisticsTotalText;
std::string StatisticsBPMText;

// [Button text]
std::string ButtonTextFontPath;
sf::Color ButtonTextColor;
unsigned ButtonTextCharacterSize;
unsigned ButtonTextOutlineThickness;
sf::Color ButtonTextOutlineColor;
sf::Vector2f ButtonTextPosition;
bool ButtonTextBoundsToggle;
sf::Vector2f ButtonTextBounds;
bool ButtonTextIgnoreBtnMovement;
bool ButtonTextBold;
bool ButtonTextItalic;
bool ButtonTextShowVisualKeys;
sf::Vector2f ButtonTextVisualKeysTextPosition;
bool ButtonTextShowTotal;
sf::Vector2f ButtonTextTotalTextPosition;
bool ButtonTextShowKPS;
sf::Vector2f ButtonTextKPSTextPosition;
bool ButtonTextShowBPM;
sf::Vector2f ButtonTextBPMTextPosition;

// [Buttons text advanced settings]
bool ButtonTextSepPosAdvancedMode;
Container<sf::Vector2f> ButtonTextAdvVisualKeysTextPosition;
Container<sf::Vector2f> ButtonTextAdvTotalTextPosition;
Container<sf::Vector2f> ButtonTextAdvKPSTextPosition;
Container<sf::Vector2f> ButtonTextAdvBPMTextPosition;

bool ButtonTextAdvancedMode;
Container<sf::Color> ButtonTextAdvColor;
Container<unsigned> ButtonTextAdvCharacterSize;
Container<unsigned> ButtonTextAdvOutlineThickness;
Container<sf::Color> ButtonTextAdvOutlineColor;
Container<sf::Vector2f> ButtonsTextAdvPosition;
Container<sf::Vector2f> ButtonTextAdvBounds;
Container<bool> ButtonTextAdvIgnoreBtnMovement;
Container<bool> ButtonTextAdvBold;
Container<bool> ButtonTextAdvItalic;

// [Button graphics]
float GfxButtonDistance;
std::string GfxButtonTexturePath;
sf::Vector2u GfxButtonTextureSize;
sf::Color GfxButtonTextureColor;
sf::Color GfxButtonBorderColor;
int ButtonShape = 0;
bool GfxButtonAdvancedMode;
Container<sf::Vector2f> GfxButtonsBtnPositions;
Container<sf::Vector2f> GfxButtonsSizes;
Container<sf::Color> GfxButtonsColor;

// [Button graphics scaling]
float GfxButtonScaleFactor;
float GfxButtonAnimationScaleFactor;
float GfxButtonTextScaleFactor;
float GfxButtonMinScale;
float GfxButtonMaxScale;
Container<float> GfxButtonAdvScaleFactor;
Container<float> GfxButtonAdvAnimationScaleFactor;
Container<float> GfxButtonAdvTextScaleFactor;

// [Theme system]
int CurrentThemeIndex;

// [Overlay settings]
bool IsOverlayApplication;
float OverlayScaleFactor;
float GlobalScaleFactor;

// [Animation graphics]
bool LightAnimation;
bool PressAnimation;
std::string AnimationTexturePath;
unsigned AnimationFrames;
sf::Vector2f AnimationScale;
sf::Color AnimationColor;
float AnimationOffset;

// [Background]
std::string BackgroundTexturePath;
sf::Color BackgroundColor;
bool ScaleBackground;

// Non config parameters
bool isGreenscreenSet;

// [Main window]
namespace Window
{
bool WindowTitleBar;
bool WindowResizable = true;
unsigned RenderUpdateFrequency;
int WindowBonusSizeTop;
int WindowBonusSizeBottom;
int WindowBonusSizeLeft;
int WindowBonusSizeRight;
}

// [KPS window]
bool KPSWindowEnabledFromStart;
sf::Vector2u KPSWindowSize;
sf::Color KPSBackgroundColor;
sf::Color KPSTextColor;
sf::Color KPSNumberColor;
std::string KPSWindowTextFontPath;
std::string KPSWindowNumberFontPath;
unsigned KPSTextSize;
unsigned KPSNumberSize;
float KPSWindowTopPadding;
float KPSWindowDistanceBetween;

// [Key press visualization]
bool KeyPressVisToggle;
float KeyPressVisSpeed;
float KeyPressVisRotation;
float KeyPressVisFadeLineLen;
sf::Vector2f KeyPressVisOrig;
sf::Color KeyPressVisColor;
float KeyPressWidthScale;
float KeyPressFixedHeight;

// [Key press visualization advanced settings]
bool KeyPressVisAdvSettingsMode;
Container<float> KeyPressVisAdvSpeed;
Container<float> KeyPressVisAdvRotation;
Container<float> KeyPressVisAdvFadeLineLen;
Container<sf::Vector2f> KeyPressVisAdvOrig;
Container<sf::Color> KeyPressVisAdvColor;
Container<float> KeyPressAdvWidthScale;
Container<float> KeyPressAdvFixedHeight;

// [Other]
namespace Other
{
bool SaveStats;
bool ShowOppOnAlt;
unsigned ButtonPressMultiplier;
bool ReduceMotion = false;
}

// Default assets
unsigned char* StatisticsDefaultFont = MainProgramFont;
unsigned char* KeyCountersDefaultFont = MainProgramFont;
unsigned char* DefaultButtonTexture = ButtonTexture;
unsigned char* DefaultAnimationTexture = AnimationTexture;
unsigned char* DefaultBackgroundTexture = BackgroundTexture;
unsigned char* DefaultGreenscreenBackgroundTexture = GreenscreenTexture;
unsigned char* DefaultKPSWindowFont = KPSWindowDefaultFont;
unsigned char* KeyPressVisTexture = KeyPressVisTextureH;

// Hot keys
sf::Keyboard::Key KeyToIncreaseKeys = sf::Keyboard::Equal;
sf::Keyboard::Key AltKeyToIncreaseKeys = sf::Keyboard::Add;
sf::Keyboard::Key KeyToDecreaseKeys = sf::Keyboard::Dash;
sf::Keyboard::Key AltKeyToDecreaseKeys = sf::Keyboard::Subtract;
sf::Keyboard::Key KeyToIncreaseButtons = sf::Keyboard::Period;
sf::Keyboard::Key KeyToDecreaseButtons = sf::Keyboard::Comma;
sf::Keyboard::Key KeyToReset = sf::Keyboard::X;
sf::Keyboard::Key KeyExit = sf::Keyboard::W;
sf::Keyboard::Key KeyToOpenKPSWindow = sf::Keyboard::K;
sf::Keyboard::Key KeyToOpenMenuWindow = sf::Keyboard::A;
sf::Keyboard::Key KeyToOpenGraphWindow = sf::Keyboard::G;
sf::Keyboard::Key KeyToOpenStyleWizard = sf::Keyboard::S;

// Saved parameters
float MaxKPS;
unsigned Total;
Container<unsigned> KeysTotal;

// [UI design tokens — Material Design 3 Expressive]
// Dark semantic palette adapted for JKPS's blue identity.
// Each surface layer sits one step higher in the elevation stack.
namespace UiTokens
{
// --- Surface colors ---
sf::Color Surface                 = sf::Color(18, 18, 22);
sf::Color SurfaceDim              = sf::Color(15, 15, 19);
sf::Color SurfaceBright           = sf::Color(48, 48, 54);
sf::Color SurfaceContainerLowest  = sf::Color(12, 12, 16);
sf::Color SurfaceContainerLow     = sf::Color(24, 24, 28);
sf::Color SurfaceContainer        = sf::Color(32, 32, 38);
sf::Color SurfaceContainerHigh    = sf::Color(40, 40, 46);
sf::Color SurfaceContainerHighest = sf::Color(48, 48, 54);
sf::Color OnSurface               = sf::Color(226, 226, 232);
sf::Color OnSurfaceVariant        = sf::Color(194, 194, 200);

// --- Primary (blue) ---
sf::Color Primary                 = sf::Color(130, 200, 255);
sf::Color OnPrimary               = sf::Color(0, 35, 65);
sf::Color PrimaryContainer        = sf::Color(0, 75, 140);
sf::Color OnPrimaryContainer      = sf::Color(200, 230, 255);

// --- Secondary (muted blue) ---
sf::Color Secondary               = sf::Color(160, 200, 220);
sf::Color OnSecondary             = sf::Color(20, 32, 42);

// --- Outlines ---
sf::Color Outline                 = sf::Color(140, 140, 148);
sf::Color OutlineVariant          = sf::Color(66, 66, 74);

// --- Error ---
sf::Color Error                   = sf::Color(255, 180, 160);
sf::Color OnError                 = sf::Color(60, 15, 0);

// --- Focus ---
sf::Color Focus                   = sf::Color(255, 210, 0);

// --- Typography ---
unsigned TextSizeHeader           = 22u;
unsigned TextSizeBody             = 15u;
unsigned TextSizeSmall            = 12u;

// --- Spacing (4 px base unit) ---
float SpacingXs                   = 4.f;
float SpacingSm                   = 8.f;
float SpacingMd                   = 12.f;
float SpacingLg                   = 16.f;
float SpacingXl                   = 24.f;

// --- Corner radius ---
float RadiusSm                    = 8.f;
float RadiusMd                    = 12.f;
float RadiusLg                    = 16.f;
float RadiusXl                    = 20.f;
}

} // !namespace Settings
