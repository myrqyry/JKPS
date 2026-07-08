#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "ResourceIdentifiers.hpp"

#include <array>
#include <memory>



class GfxButton : public sf::Drawable, public sf::Transformable
{
    protected:
        enum SpriteID
        {
            ButtonSprite,
            AnimationSprite,
            PressEffectSprite,
            ShadowSprite,
            KeyVisualizerSprite,
            SpriteIdCounter,
        };
        enum TextID
        {
            VisualKey,
            KeyCounter,
            KeyPerSecond,
            BeatsPerMinute,
            TextIdCounter,
            Nothing
        };
        
        // Enhanced press animation states
        enum PressState
        {
            Normal,
            Pressed,
            Released,
            Transitioning
        };


    public:
        GfxButton(const unsigned idx, const TextureHolder& textureHolder, const FontHolder& fontHolder);

        void update(float deltaSeconds, bool keyState);
        virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        void updateAssets();
        void updateParameters();

        static void setShowBounds(bool flag, int idx = -1);
        static float getWidth(unsigned idx);
        static float getHeight(unsigned idx);
        static TextID getTextIdToDisplay();

        virtual ~GfxButton();


protected:
        // Enhanced press animation system
        struct PressAnimation
        {
            sf::Vector2f scale;
            sf::Color color;
            sf::Vector2f position;
            float time;
            PressState state;
        };
        
        // Theme integration structure
        struct ThemeColors
        {
            sf::Color normalBorder;
            sf::Color pressedBorder;
            sf::Color shadow;
            sf::Color pressColor;
            sf::Color gradientTop;
            sf::Color gradientBottom;
        };
        
        // Responsive sizing structure
        struct ResponsiveSize
        {
            float baseWidth;
            float baseHeight;
            float scaleFactor;
            float minScale;
            float maxScale;
        };
        
        // Enhanced key visualizer styling
        struct KeyVisualizerStyle
        {
            sf::Color backgroundColor;
            sf::Color borderColor;
            sf::Color textColor;
            float cornerRadius;
            float shadowIntensity;
            bool themeAware;
        };

        // Theme management
        struct ThemeInfo
        {
            std::string themeName;
            sf::Color normalBorder;
            sf::Color pressedBorder;
            sf::Color shadow;
            sf::Color pressColor;
            sf::Color gradientTop;
            sf::Color gradientBottom;
        };

        // Enhanced styling functions
        void applyEnhancedButtonStyling(unsigned idx, bool advGfxMode, bool advTextMode, bool sepValAdvMode);
        void applyEnhancedGradientToSprite(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &baseColor, bool advMode, unsigned idx);
        void applyEnhancedPressBorder(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &borderColor, const sf::Color &shadowColor, const sf::Color &pressColor, bool advMode, unsigned idx);
        void applyEnhancedScaling(unsigned idx, bool advMode);
        void applyEnhancedGradientOverlay(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &pressColor, unsigned idx);
        
        // Enhanced gradient and color functions
        EnhancedGradientColors getEnhancedGradientColors(unsigned idx) const;
        GradientColors getEnhancedOverlayButtonColor(unsigned idx, bool advMode);
        sf::Color getEnhancedButtonBorderColor(unsigned idx, bool advMode);
        sf::Color getEnhancedShadowColor(unsigned idx);
        sf::Color getEnhancedPressColor(unsigned idx, bool advMode);
        
        // Responsive sizing functions
        void applyResponsiveScaling(unsigned idx, float &baseScale, float &animationScale);
        void applyResponsiveTextScaling(unsigned idx);
        float getEnvironmentScaleFactor();
        float getWindowContentScale();
        
        // Theme integration functions
        void applyThemeBasedGradient(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &pressColor, unsigned idx);
        ThemeInfo getCurrentTheme();
        void initializeThemeSystem();
        
        // Global theme storage
        static std::vector<ThemeInfo> settingsThemes;
        
        // Theme integration structure
        struct ThemeColors
        {
            sf::Color normalBorder;
            sf::Color pressedBorder;
            sf::Color shadow;
            sf::Color pressColor;
            sf::Color gradientTop;
            sf::Color gradientBottom;
        };
        
        // Responsive sizing structure
        struct ResponsiveSize
        {
            float baseWidth;
            float baseHeight;
            float scaleFactor;
            float minScale;
            float maxScale;
        };
        
        // Enhanced key visualizer styling
        struct KeyVisualizerStyle
        {
            sf::Color backgroundColor;
            sf::Color borderColor;
            sf::Color textColor;
            float cornerRadius;
            float shadowIntensity;
            bool themeAware;
        };
        
        // Theme integration structure
        struct ThemeColors
        {
            sf::Color normalBorder;
            sf::Color pressedBorder;
            sf::Color shadow;
            sf::Color pressColor;
            sf::Color gradientTop;
            sf::Color gradientBottom;
        };
        
        // Responsive sizing structure
        struct ResponsiveSize
        {
            float baseWidth;
            float baseHeight;
            float scaleFactor;
            float minScale;
            float maxScale;
        };
        
        // Enhanced key visualizer styling
        struct KeyVisualizerStyle
        {
            sf::Color backgroundColor;
            sf::Color borderColor;
            sf::Color textColor;
            float cornerRadius;
            float shadowIntensity;
            bool themeAware;
        };

        // Enhanced gradient colors structure
        struct EnhancedGradientColors
        {
            sf::Color topLeft;
            sf::Color topRight;
            sf::Color bottomRight;
            sf::Color bottomLeft;
        };

        // Theme management
        struct ThemeInfo
        {
            std::string themeName;
            sf::Color normalBorder;
            sf::Color pressedBorder;
            sf::Color shadow;
            sf::Color pressColor;
            sf::Color gradientTop;
            sf::Color gradientBottom;
        };

        // Enhanced styling functions
        void applyEnhancedButtonStyling(unsigned idx, bool advGfxMode, bool advTextMode, bool sepValAdvMode);
        void applyEnhancedGradientToSprite(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &baseColor, bool advMode, unsigned idx);
        void applyEnhancedPressBorder(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &borderColor, const sf::Color &shadowColor, const sf::Color &pressColor, bool advMode, unsigned idx);
        void applyEnhancedScaling(unsigned idx, bool advMode);
        void applyEnhancedGradientOverlay(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &pressColor, unsigned idx);
        
        // Enhanced gradient and color functions
        EnhancedGradientColors getEnhancedGradientColors(unsigned idx) const;
        GradientColors getEnhancedOverlayButtonColor(unsigned idx, bool advMode);
        sf::Color getEnhancedButtonBorderColor(unsigned idx, bool advMode);
        sf::Color getEnhancedShadowColor(unsigned idx);
        sf::Color getEnhancedPressColor(unsigned idx, bool advMode);
        
        // Responsive sizing functions
        void applyResponsiveScaling(unsigned idx, float &baseScale, float &animationScale);
        void applyResponsiveTextScaling(unsigned idx);
        float getEnvironmentScaleFactor();
        float getWindowContentScale();
        
        // Theme integration functions
        void applyThemeBasedGradient(std::unique_ptr<sf::Sprite> &sprite, const sf::Color &pressColor, unsigned idx);
        ThemeInfo getCurrentTheme();
        void initializeThemeSystem();
        
        // Global theme storage
        static std::vector<ThemeInfo> settingsThemes;
        void resetAssets();
        void scaleSprites();

        // Light animation
        void lightKey();
        void fadeKey();

        // Press animation
        void lowerKey();
        void raiseKey();

        sf::Vector2f getScaleStep() const;
        float getRiseStep() const;


    protected:
        std::array<std::unique_ptr<sf::Sprite>, SpriteID::SpriteIdCounter> mSprites;
        std::array<std::unique_ptr<sf::Text>, TextID::TextIdCounter> mTexts;


    private:
        class RectEmitter : public sf::Drawable
        {
            public:
                // RectEmitter(const sf::Texture &texture);
                RectEmitter(unsigned btnIdx);

                void update(float deltaSeconds, bool keyState, bool prevKeyState);
                void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

                void setPosition(sf::Vector2f position);
                void create(float deltaSeconds, sf::Vector2f buttonSize);

                void scaleTexture(sf::Vector2f buttonSize);


            private:
                sf::Transform getPressRectTransform(sf::Transform transform) const;
                float getVertexProgress(size_t vertexNumber, float vertexHeight) const;
                sf::Color getVertexColor(const sf::VertexArray &vertexArray, size_t vertexIndex) const;
                void pushVertecies(sf::VertexArray &vertexArray, sf::Vertex *toPush, size_t offset, sf::Vector2f buttonSize);

				static float getConstantSpeedScale();

            private:
                const unsigned mBtnIdx;
                // const sf::Texture &mTexture;
                sf::Vector2f mTextureScale;
                sf::VertexArray mMiddleVertecies;
                sf::VertexArray mTopVertecies;
                sf::VertexArray mBottomVertecies;
                std::vector<size_t> mAvailableRectIndices;
                std::vector<size_t> mUsedRectIndices;
                std::vector<sf::Text> mTexts;
                sf::Vector2f mEmitterPosition;
                sf::Vector2f mLastRectSize;
        };


    private:
        const TextureHolder &mTextures;
        const FontHolder &mFonts;
        RectEmitter mEmitter;
        sf::RectangleShape mBounds;
        static int mSelectedKeyBounds;

        bool mLastKeyState;

        float mButtonsHeightOffset;

        const unsigned mBtnIdx;

        static bool mShowBounds;
};
