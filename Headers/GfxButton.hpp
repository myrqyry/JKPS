#pragma once

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

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
        void keepInBounds(sf::Text &text);
        void centerOrigins();
        void drawVectorShape(sf::RenderTarget &target, sf::RenderStates states, int shape, sf::Vector2f position, sf::Vector2f size, sf::Color fillColor, sf::Color outlineColor) const;


    private:
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

        const TextureHolder &mTextures;
        const FontHolder &mFonts;
        const unsigned mBtnIdx;

        float mButtonsHeightOffset;


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
                // Cache-friendly active-set: a flat flag per pool slot instead of a
                // growing/shrinking index vector. Iteration stays uniform and no
                // per-frame allocation occurs while particles live and die.
                std::vector<bool> mActive;
                std::vector<size_t> mUsedRectIndices;
                std::vector<sf::Text> mTexts;
                sf::Vector2f mEmitterPosition;
                sf::Vector2f mLastRectSize;
        };


    private:
        RectEmitter mEmitter;
        sf::RectangleShape mBounds;
        static int mSelectedKeyBounds;

        mutable std::unique_ptr<sf::Shape> mCachedShape;
        mutable sf::VertexArray mCachedStarFill{sf::TriangleFan, 11};
        mutable sf::VertexArray mCachedStarOutline{sf::LineStrip, 11};
        mutable int mCachedShapeType = -1;
        mutable sf::Vector2f mCachedShapeSize;
        mutable sf::Vector2f mCachedShapePos;

        bool mLastKeyState;

        static bool mShowBounds;
};
