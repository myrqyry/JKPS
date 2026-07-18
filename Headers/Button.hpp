#pragma once

#include "GfxButton.hpp"
#include "LogButton.hpp"
#include "ResourceIdentifiers.hpp"
#include "LogicalParameter.hpp"


class Button : public LogButton, public GfxButton
{
    public:
        Button(unsigned idx, LogKey &key, const TextureHolder &textureHolder, const FontHolder &fontHolder);

        void update(float deltaSeconds);
        void processInput();

        void reset();

        LogKey *getLogKey();

        unsigned getIdx() const;
        static unsigned size();
        static void setCount(unsigned count);
        static bool parameterIdMatches(LogicalParameter::ID id);

        ~Button();


    private:
        void setTextStrings();
        void controlBounds();


    private:
        static unsigned mSize;

        using GfxButton::mBtnIdx;

        // Layout caching: text bounds/origins are only recomputed when the
        // displayed strings, character size, or key state actually change,
        // instead of every frame.
        bool mTextLayoutDirty;
        unsigned mCachedCharSize;
};