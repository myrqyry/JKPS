#include "../Headers/Button.hpp"
#include "../Headers/ResourceHolder.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/StringHelper.hpp"


unsigned Button::mSize(0u);

Button::Button(unsigned idx, LogKey &key, const TextureHolder &textureHolder, const FontHolder &fontHolder)
: LogButton(idx, key)
, GfxButton(idx, textureHolder, fontHolder)
, mTextLayoutDirty(true)
, mCachedCharSize(0u)
{
    LogButton::mTotal = Settings::KeysTotal[mBtnIdx];
}

void Button::update(float deltaSeconds)
{
    GfxButton::update(deltaSeconds, LogButton::isButtonPressed());
    setTextStrings();
    controlBounds();
}

void Button::processInput()
{
    processRealtimeInput();
}

void Button::reset()
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::ButtonTextAdvancedMode;
    const auto chSz = !advMode ? Settings::ButtonTextCharacterSize : Settings::ButtonTextAdvCharacterSize[mBtnIdx];

    LogButton::reset();
    setTextStrings();
    // text could be too large, but if everything was reset then original ch size must be set and controlled
    for (auto &text : mTexts)
    {
        text->setCharacterSize(chSz);
        GfxButton::keepInBounds(*text);
    }
    GfxButton::centerOrigins();
}

void Button::setTextStrings()
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::ButtonTextAdvancedMode;
    const auto chSz = !advMode ? Settings::ButtonTextCharacterSize : Settings::ButtonTextAdvCharacterSize[mBtnIdx];

    const auto lAlt = Settings::ShowOppOnAlt && sf::Keyboard::isKeyPressed(sf::Keyboard::LAlt);
    const auto sepValAdvMode = isInSupportedRange && Settings::ButtonTextSepPosAdvancedMode;

    // Only recompute layout when a displayed string or the character size
    // actually changes; otherwise the bounds/origin pass is redundant.
    if (chSz != mCachedCharSize)
        mTextLayoutDirty = true;

    auto assign = [this](TextID id, const std::string &str)
    {
        if (mTexts[id]->getString().toAnsiString() != str)
        {
            mTexts[id]->setString(str);
            mTextLayoutDirty = true;
        }
    };

    if (Settings::ButtonTextShowVisualKeys) 
    {
        if ((!lAlt) || (lAlt && sepValAdvMode && Settings::ButtonTextShowTotal))
            assign(VisualKey, LogButton::mKey.visualStr);
        else
            assign(VisualKey, std::to_string(LogButton::mTotal));
    }
    if (Settings::ButtonTextShowTotal)
    {
        if ((!lAlt) || (lAlt && sepValAdvMode && Settings::ButtonTextShowVisualKeys))
            assign(KeyCounter, std::to_string(LogButton::mTotal));
        else
            assign(KeyCounter, LogButton::mKey.visualStr);
    }
    if (Settings::ButtonTextShowKPS)
    {
        // static_cast is required since KPS w/floating point is not still done
        assign(KeyPerSecond, std::to_string(static_cast<unsigned>(mKeysPerSecond)));
    }
    if (Settings::ButtonTextShowBPM)
    {
        assign(BeatsPerMinute, std::to_string(static_cast<unsigned>(LogButton::getLocalBeatsPerMinute())));
    }
}

void Button::controlBounds()
{
    const auto isInSupportedRange = mBtnIdx < Settings::SupportedAdvancedKeysNumber;
    const auto advMode = isInSupportedRange && Settings::ButtonTextAdvancedMode;
    const auto chSz = !advMode ? Settings::ButtonTextCharacterSize : Settings::ButtonTextAdvCharacterSize[mBtnIdx];

    if (LogButton::mKey.resetChangedState())
        mTextLayoutDirty = true;

    if (!mTextLayoutDirty)
        return;

    for (auto &text : mTexts)
    {
        text->setCharacterSize(chSz);
        GfxButton::keepInBounds(*text);
    }
    GfxButton::centerOrigins();

    mCachedCharSize = chSz;
    mTextLayoutDirty = false;
}

LogKey *Button::getLogKey()
{
    return &(LogButton::mKey);
}

Button::~Button()
{
}

unsigned Button::getIdx() const
{
    return mBtnIdx;
}

unsigned Button::size()
{
    return mSize;
}

void Button::setCount(unsigned count)
{
    mSize = count;
}

bool Button::parameterIdMatches(LogicalParameter::ID id)
{
    const auto idU = static_cast<unsigned>(id);
    
    const auto textSepValAdvParms = 4ul;
    const auto firstTextSepValAdvPar = static_cast<unsigned>(LogicalParameter::ID::BtnTextAdvVisPosition);
    const auto lastTextSepValAdvPar = firstTextSepValAdvPar + textSepValAdvParms * Settings::SupportedAdvancedKeysNumber - 1u;

    const auto textAdvParms = 8ul;
    const auto firstTextAdvPar = static_cast<unsigned>(LogicalParameter::ID::BtnTextAdvClr);
    const auto lastTextAdvPar = firstTextAdvPar + textAdvParms * Settings::SupportedAdvancedKeysNumber - 1u;

    const auto gfxBtnAdvParms = 3ul;
    const auto firstGfxBtnAdvPar = static_cast<unsigned>(LogicalParameter::ID::BtnGfxBtnPos);
    const auto lastGfxBtnAdvPar = firstGfxBtnAdvPar + gfxBtnAdvParms * Settings::SupportedAdvancedKeysNumber - 1u;

    return 
        id == LogicalParameter::ID::BtnTextClr ||
        id == LogicalParameter::ID::BtnTextChSz ||
        id == LogicalParameter::ID::BtnTextOutThck ||
        id == LogicalParameter::ID::BtnTextOutClr ||
        id == LogicalParameter::ID::BtnTextPosition ||
        id == LogicalParameter::ID::BtnTextBounds ||
        id == LogicalParameter::ID::BtnTextBold ||
        id == LogicalParameter::ID::BtnTextItal ||
        id == LogicalParameter::ID::BtnTextShowVisKeys ||
        id == LogicalParameter::ID::BtnTextShowTot ||
        id == LogicalParameter::ID::BtnTextShowKps ||
        id == LogicalParameter::ID::BtnTextSepPosAdvMode ||
        id == LogicalParameter::ID::BtnTextVisPosition ||
        id == LogicalParameter::ID::BtnTextTotPosition ||
        id == LogicalParameter::ID::BtnTextKPSPosition ||
        id == LogicalParameter::ID::BtnTextBPMPosition ||

        id == LogicalParameter::ID::BtnGfxDist ||
        id == LogicalParameter::ID::BtnGfxTxtr ||
        id == LogicalParameter::ID::BtnGfxTxtrSz ||
        id == LogicalParameter::ID::BtnGfxTxtrClr ||
        id == LogicalParameter::ID::BtnGfxBorderClr ||
        id == LogicalParameter::ID::BtnGfxShape ||

        id == LogicalParameter::ID::AnimGfxVel ||
        id == LogicalParameter::ID::AnimGfxScl ||
        id == LogicalParameter::ID::AnimGfxClr ||
        id == LogicalParameter::ID::AnimGfxOffset ||

        id == LogicalParameter::ID::MainWndwTop ||
        id == LogicalParameter::ID::MainWndwBot ||
        id == LogicalParameter::ID::MainWndwLft ||
        id == LogicalParameter::ID::MainWndwRght ||

        id == LogicalParameter::ID::BtnTextPosAdvMode ||
        id == LogicalParameter::ID::BtnGfxAdvMode ||
        (idU >= firstTextSepValAdvPar && idU <= lastTextSepValAdvPar) ||
        (idU >= firstTextAdvPar && idU <= lastTextAdvPar) ||
        (idU >= firstGfxBtnAdvPar && idU <= lastGfxBtnAdvPar);
}
