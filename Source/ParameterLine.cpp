#include "../Headers/ParameterLine.hpp"
#include "../Headers/ResourceHolder.hpp"
#include "../Headers/ResourceIdentifiers.hpp"
#include "../Headers/StringHelper.hpp"
#include "../Headers/GfxButtonSelector.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/Menu.hpp"
#include "../Headers/GfxButton.hpp"
#include "../Headers/ConfigHelper.hpp"
#include "../Headers/Utility.hpp"

#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <string>
#include <cmath>
#include <mutex>
#include <cctype>
#include <optional>


sf::RectangleShape ParameterLine::mCursor(sf::Vector2f(1, 21));
std::shared_ptr<LogicalParameter> ParameterLine::mSelectedParameter(nullptr);
std::shared_ptr<ParameterLine> ParameterLine::mSelectedLine(nullptr);
std::shared_ptr<GfxParameter> ParameterLine::mSelectedValue(nullptr);
int ParameterLine::mSelectedValueIndex(-1);
Palette ParameterLine::mPalette(0);
bool ParameterLine::mRefresh(false);

ParameterLine::ParameterLine(
    std::shared_ptr<LogicalParameter> parameter,
    const FontHolder &fonts, 
    const TextureHolder &textures,
    sf::RenderWindow &window)
: mWindow(window)
, mType(parameter->mType)
, mRectLine(sf::Vector2f(785.f, 40.f))
, mParameter(parameter)
, mColorButtonP(nullptr)
, paramValWasChanged(false)
, mIsThRunning(false)
{
    mRectLine.setFillColor(pickColor(mType));
    mRectLine.setOutlineThickness(1.f);
    mRectLine.setOutlineColor(Settings::UiTokens::CustomizationOutline);
    mParameterName.setString(parameter->mParName);
    mParameterName.setFont(fonts.get(Fonts::Parameter));
    mParameterName.setFillColor(Settings::UiTokens::OnSurface);
    setCharacterSize(20u);

    buildButtons(parameter->getValStr(), fonts, textures);
    buildLimits(fonts);

    // If it wasn't still initialized
    if (mCursor.getPosition() == sf::Vector2f())
    {
        mCursor.setOutlineThickness(1);
        mCursor.setFillColor(Settings::UiTokens::CustomizationFocus);
        mCursor.setOutlineColor(sf::Color::Black);
        mCursor.setOrigin(mCursor.getSize() / 2.f);
        mCursor.setPosition(0, mRectLine.getSize().y / 2);
    }
}

bool ParameterLine::handleEvent(sf::Event event)
{
    // Don't handle events for hidden objects
    if (isHidden())
        return false;

    const auto isSelected = handleButtonsInteractionEvent(event);
    const auto wasModified = handleValueModEvent(event);
    return isSelected || wasModified;
}

// Keyboard events which modify values
bool ParameterLine::handleValueModEvent(sf::Event event)
{
    // // Allow mouse wheel affect the values on scroll
    // if (event.type == sf::Event::MouseWheelScrolled)
    // {
    //     // Create new event and swap it with the original one
    //     sf::Event e;
    //     e.type = sf::Event::KeyPressed;
    //     e.key.code = (event.mouseWheelScroll.delta > 0 
    //         ? sf::Keyboard::Up : sf::Keyboard::Down);
    //     event = e;
    // }

    if (event.type == sf::Event::KeyPressed)
    {
        if (mSelectedValue == nullptr || !isSelectedValHere()) 
            return false;

        auto str = static_cast<std::string>(mSelectedValue->mValText.getString());
        const auto key = event.key;
        const auto keyCode = key.code;
        const auto isStrType = mType == LogicalParameter::Type::String || mType == LogicalParameter::Type::StringPath;
        auto btnIdx = 0;
        if (mType == LogicalParameter::Type::Color
        ||  mType == LogicalParameter::Type::VectorU
        ||  mType == LogicalParameter::Type::VectorI
        ||  mType == LogicalParameter::Type::VectorF)
        {
            btnIdx = -1;
            auto size = mParameterValues.size();
            for (auto i = 0ul; i < size; ++i)
            {
                if (mParameterValues[i] == mSelectedValue)
                {
                    btnIdx = static_cast<int>(i);
                    break;
                }
            }
            assert(btnIdx != -1);
            if (str != "-0")
                str = mParameter->getValPt(btnIdx);
        }

        if (!isStrType
        && ((keyCode >= sf::Keyboard::Num0    && keyCode <= sf::Keyboard::Num9)
         || (keyCode >= sf::Keyboard::Numpad0 && keyCode <= sf::Keyboard::Numpad9)))
        {
            int n = 0;
            if (keyCode >= sf::Keyboard::Num0 &&  keyCode <= sf::Keyboard::Num9)
                n = (keyCode - sf::Keyboard::Num0);
            if (keyCode >= sf::Keyboard::Numpad0 && keyCode <= sf::Keyboard::Numpad9)
                n = (keyCode - sf::Keyboard::Numpad0);

            if (mSelectedValueIndex == 0 && (str[0] == '-' || n == 0))
                return true;

            const auto strSize = str.size();
            if ((strSize == 1ul && str[0] == '0') || (strSize == 2ul && !std::isdigit(str[0]) && str[1] == '0'))
            {
                str.back() = static_cast<char>(n + '0');
                if (mSelectedValueIndex == static_cast<int>(strSize - 1ul))
                    ++mSelectedValueIndex;
            }
            else
            {
                const auto prevVal = str;
                addChOnIdx(str, static_cast<unsigned>(mSelectedValueIndex), static_cast<char>(n + '0'));
                const auto check = stof(str); 
                str = std::to_string(static_cast<int>(check));

                if (checkLimits(check))
                {  
                    runThread(str, prevVal);
                    --mSelectedValueIndex;
                }
                ++mSelectedValueIndex;
            }
        }

        if (!isStrType && keyCode == sf::Keyboard::Up)
        {
            const auto prevVal = str;
            const auto check = stof(str) + 1.f; 
            str = std::to_string(static_cast<int>(check));

            if (checkLimits(check))
                runThread(str, prevVal);

            const auto strSize = str.size();
            const auto prevValSize = prevVal.size();
            mSelectedValueIndex += strSize == prevValSize ? 0 : strSize > prevValSize ? 1 : -1;
        }

        if (!isStrType && keyCode == sf::Keyboard::Down)
        {
            const auto prevVal = str;
            const auto check = stof(str) - 1.f; 
            str = std::to_string(static_cast<int>(check));

            if (checkLimits(check))
                runThread(str, prevVal);

            const auto strSize = str.size();
            const auto prevValSize = prevVal.size();
            mSelectedValueIndex += strSize == prevValSize ? 0 : strSize > prevValSize ? 1 : -1;
        }

        if (!isStrType && (keyCode == sf::Keyboard::Hyphen || keyCode == sf::Keyboard::Subtract))
        {
            if (mType == LogicalParameter::Type::Unsigned 
            ||  mType == LogicalParameter::Type::Color 
            ||  mType == LogicalParameter::Type::VectorU)
                return true;

            if (str[0] == '-')
            {
                rmChOnIdx(str, 0);
                --mSelectedValueIndex;
            } else
            {
                addChOnIdx(str, 0, '-');
                ++mSelectedValueIndex;
            }
        }

		bool hasRemovedSomething = false;
        if (keyCode == sf::Keyboard::Backspace)
        {
            if (mSelectedValueIndex != 0)
            {
                // "0" or "-0"
                if ((!isStrType && str.size() == 1)
                ||  (str.size() == 2 && str[0] == '-' && mSelectedValueIndex == 2))
                {
                    str = "0";
                    mSelectedValueIndex = 1;
                }
                else
                {
                    rmChOnIdx(str, static_cast<unsigned>(mSelectedValueIndex - 1));
                    if (!isStrType && std::stoi(str) == 0)
                    {
                        // ex.: -500, remove 5, result "-0", if 500, then "0"
                        str = (str[0] == '-' ? "-0" : "0");
                    }
                    --mSelectedValueIndex;
                }
            }

			hasRemovedSomething = true;
        }
        else if (keyCode == sf::Keyboard::Delete)
        {
            const auto strSize = str.size();

            if (static_cast<int>(strSize) > mSelectedValueIndex)
            {
                // "0" or "-0"
                if ((!isStrType && strSize == 1) 
                ||  (!isStrType && strSize == 2 && str[0] == '-' && mSelectedValueIndex == 0))
                {
                    str = "0";
                    mSelectedValueIndex = 0;
                }
                else
                {
                    rmChOnIdx(str, static_cast<unsigned>(mSelectedValueIndex));
                    if (!isStrType && std::stoi(str) == 0)
                    {
                        // ex.: -500, remove 5, result "-0", if 500, then "0"
                        str = (str[0] == '-' ? "-0" : "0");
                    }
                }
            }

			hasRemovedSomething = true;
        }
		if (keyCode == sf::Keyboard::Left)
        {
            if (mSelectedValueIndex > 0)
                --mSelectedValueIndex;
        }
		else if (keyCode == sf::Keyboard::Right)
        {
            if (static_cast<int>(str.size()) > mSelectedValueIndex)
                ++mSelectedValueIndex;
        }
        else if (keyCode == sf::Keyboard::Home)
        {
            mSelectedValueIndex = 0;
        }
        else if (keyCode == sf::Keyboard::End)
        {
            mSelectedValueIndex = static_cast<int>(str.size());
        }

        if (isStrType && GfxButtonSelector::isCharacter(keyCode))
        {
            const auto maxLength = 50ul;
            if (key.control && keyCode == sf::Keyboard::V)
            {
                const auto clipboardStr = std::string(sf::Clipboard::getString());
                const auto lhs = std::string(str.begin(), str.begin() + mSelectedValueIndex);
                const auto rhs = std::string(str.begin() + mSelectedValueIndex, str.end());
                const auto newStr = lhs + clipboardStr + rhs;

                if (maxLength >= newStr.length())
                {
                    str = newStr;
                    mSelectedValueIndex += static_cast<int>(clipboardStr.length());
                }
            }
            else 
            {
                if (maxLength >= str.length())
                {
                    addChOnIdx(str, static_cast<unsigned>(mSelectedValueIndex), enumKeyToStr(keyCode));
                    ++mSelectedValueIndex;
                }
            }
        }
		else if (hasRemovedSomething && (mType == LogicalParameter::Type::Int 
		|| mType == LogicalParameter::Type::Float || mType == LogicalParameter::Type::Unsigned))
		{
			// Re-apply through the parameter so it clamps the value string, then read back the clamped result
			const int prevLen = static_cast<int>(str.length());
			mParameter->setValStr(str, static_cast<unsigned>(btnIdx));
			ConfigHelper::readDigitParameter(*mParameter, str);
			const int newLen = static_cast<int>(str.length());
			if (newLen != prevLen)
			{
				mSelectedValueIndex++;
			}
		}

        mParameter->setValStr(str, static_cast<unsigned>(btnIdx));
        mSelectedValue->mValText.setString(str);
        paramValWasChanged = true;

        mSelectedValue->setupValPos();
        setCursorPos();

        // Doesn't work
        // If values were changed in menu, then they must be also changed on palette
        // if (mPalette.isWindowOpen())
        //     mPalette.setColorOnPalette(lineToColor(mSelectedLine));
        return true;
    }
    return false;
}

bool ParameterLine::handleButtonsInteractionEvent(sf::Event event)
{
    // Mouse/keyboard events to control buttons selection/deselection
    if (event.type == sf::Event::MouseButtonPressed 
    ||  event.type == sf::Event::KeyPressed)
    {
        const auto halfWindowSize = static_cast<sf::Vector2f>(mWindow.getSize()) / 2.f;
        const auto viewCenter = mWindow.getView().getCenter();
        const auto viewOffset = viewCenter - halfWindowSize;
        const auto relMousePos = sf::Mouse::getPosition(mWindow);
        const auto absMousePos = static_cast<sf::Vector2f>(relMousePos) + viewOffset;
        const auto button = event.mouseButton.button;
        const auto key = event.key.code;
        auto ret = false;

        // Check click on buttons
        for (const auto &elem : mParameterValues)
        {
            auto contains = elem->contains(absMousePos);
            if (contains)
                ret = true;

            if (event.type == sf::Event::MouseButtonPressed 
            && (button == sf::Mouse::Left && contains))
            {
                if (mType == LogicalParameter::Type::Bool)
                {
                    mParameter->setValStr(LogicalParameter::getInverseBool(mParameter->getBool()));
                    elem->mValText.setString(mParameter->getValStr());
                    elem->setInverseMark();
                    deselect();
                    ret = false;
                }
                // Refresh button has 0x0 rectangle shape 
                else if (mType == LogicalParameter::Type::StringPath && elem->mRect.getSize().x == 0.f)
                {
                    mRefresh = true;
                    deselect();
                    // return in order to don't select refresh button
                    ret = false; 
                }
                else if (mType != LogicalParameter::Type::Color && mPalette.isWindowOpen())
                    mPalette.closeWindow();
                else
                {
                    deselect();
                    select(elem);
                    ret = true;
                }
            }
            if (event.type == sf::Event::KeyPressed && key == sf::Keyboard::Enter)
            {
                // Refresh button has 0x0 rectangle shape 
                if (mType == LogicalParameter::Type::StringPath && elem->mRect.getSize().x == 0.f)
                {
                    mRefresh = true;
                    deselect();
                    // return in order to don't select refresh button
                    ret = false; 
                }
            }

            if ((event.type == sf::Event::KeyPressed && (key == sf::Keyboard::Escape || key == sf::Keyboard::Enter)) 
            ||  (event.type == sf::Event::MouseButtonPressed && button == sf::Mouse::Right))
                ret = false;

            if (event.type == sf::Event::KeyPressed && key == sf::Keyboard::Tab)
            {
                if (tabulation())
                {
                    // return in order to avoid further tabulation and segmentation fault
                    ret = true;
                    break;
                }
            }
        }

        if (event.type == sf::Event::MouseButtonPressed && button == sf::Mouse::Left 
        &&  mType == LogicalParameter::Type::Color)
            ret |= selectRgbCircle(button, absMousePos);

        return ret;
    }
    return false;
}

bool ParameterLine::tabulation()
{
    if (isItSelectedLine(shared_from_this()))
    {
        for (auto it = mParameterValues.begin(); it < mParameterValues.end(); ++it)
        {
            if (*it == mSelectedValue)
            {
                deselect();
                auto tabOn = std::shared_ptr<GfxParameter>();

                if (!sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
                    // tab forward
                    tabOn = (it + 1 != mParameterValues.end() ? *(it + 1) : mParameterValues[0]);
                else
                    // tab backward
                    tabOn = (it == mParameterValues.begin() ? mParameterValues.back() : *(it - 1));

                select(tabOn);
                return true;
            }
        }
    }
    return false;
}

bool ParameterLine::selectRgbCircle(sf::Mouse::Button /*button*/, sf::Vector2f mousePos)
{
    const auto circleOrigin = mColorButtonP->getOrigin();
    const auto circlePosition = mColorButtonP->getPosition();
    auto circleRect = sf::FloatRect(mColorButtonP->rgbCircleSprite.getGlobalBounds());
    const auto position = getPosition();
    circleRect.left = position.x + circlePosition.x - circleOrigin.x;
    circleRect.top = position.y + circlePosition.y - circleOrigin.y;

    if (circleRect.contains(mousePos))
    {
        // If there is already selected line, but it isn't this one
        if (mSelectedLine && !isItSelectedLine(shared_from_this()))
            deselect();
        // If nothing is selected
        if (!mSelectedLine)
            select(mParameterValues[0]);

        // mPalette.setColorOnPalette(lineToColor(mSelectedLine));
        mPalette.openWindow(mWindow.getPosition() + static_cast<sf::Vector2i>(mWindow.getSize() / 2U));
        return true;
    }
    return false;
}

void ParameterLine::processInput()
{
    // Don't act upon hidden objects
    if (isHidden())
        return;

    if (mType == LogicalParameter::Type::Color)
        mPalette.processInput();
}

void ParameterLine::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // Don't draw hidden objects
    if (isHidden())
        return;

    states.transform *= getTransform();

    target.draw(mRectLine, states);
    target.draw(mParameterName, states);
    target.draw(mLimits, states);
    for (const auto &elem : mParameterValues)
        target.draw(*elem, states);
    
    if (mSelectedValue && isSelectedValHere())
        target.draw(mCursor, states);

    if (mType == LogicalParameter::Type::Color)
    {
        target.draw(*mColorButtonP, states);
        if (mPalette.isWindowOpen() && this == mSelectedLine.get())
            mPalette.render();
    }
}

void ParameterLine::buildButtons(const std::string &valueStr, const FontHolder &fonts, const TextureHolder &textures)
{
    GfxParameter::mTextures = &textures;
    GfxParameter::mFonts = &fonts;

    auto val = std::shared_ptr<GfxParameter>(); 
    if (mType == LogicalParameter::Type::Bool)
    {
        val = std::make_shared<GfxParameter>(this, readValue(valueStr, 0) == "True");
        val->setPosition(sf::Vector2f(mRectLine.getSize().x - 
            GfxParameter::getPosX(), mRectLine.getSize().y / 2));
        mParameterValues.emplace_back(std::move(val));
        return;
    }

    if (mType == LogicalParameter::Type::String || mType == LogicalParameter::Type::StringPath)
    {
        val = std::make_shared<GfxParameter>(this, readValue(valueStr, 0), 0, sf::Vector2f(265.f, 25.f));
        val->setPosition(598.f, mRectLine.getSize().y / 2.f);

        mParameterValues.emplace_back(std::move(val));

        if (mType == LogicalParameter::Type::StringPath)
        {
            val = std::make_shared<GfxParameter>(this);
            val->setPosition(765.f, mRectLine.getSize().y / 2.f);

            mParameterValues.emplace_back(std::move(val));
        }
        return;
    }

    const auto count = readAmountOfParms(valueStr);
    for (auto i = 0ul; i < count; ++i)
    {
        val = std::make_shared<GfxParameter>(this, readValue(valueStr, static_cast<unsigned>(i)), static_cast<unsigned>(i));

        val->setPosition(val->getPosition() + sf::Vector2f(mRectLine.getSize().x - 
            GfxParameter::getPosX(), mRectLine.getSize().y / 2));
        
        if (mType == LogicalParameter::Type::Color && i == 0ul)
        {
            const auto dist = 65.f;
            mColorButtonP = std::make_unique<ColorButton>(textures.get(Textures::rgbCircle));
            mColorButtonP->setPosition(val->getPosition().x - dist, mRectLine.getSize().y / 2);
            mColorButtonP->setOrigin(static_cast<sf::Vector2f>(mColorButtonP->rgbCircleSprite.getTexture()->getSize()) / 2.f);
        }

        mParameterValues.emplace_back(std::move(val));
    }
}

void ParameterLine::buildLimits(const FontHolder &fonts)
{
    if (mParameter->mLowLimits != mParameter->mHighLimits)
    {
        mLimits.setFont(fonts.get(Fonts::Parameter));
        mLimits.setCharacterSize(10u);
        mLimits.setFillColor(Settings::UiTokens::OnSurfaceVariant);
        mLimits.setOrigin(mLimits.getLocalBounds().left, mLimits.getLocalBounds().top);
        mLimits.setPosition(mParameterName.getLocalBounds().left, mParameterName.getLocalBounds().top + 
            mParameterName.getLocalBounds().height + 4.f);
        mLimits.setString("Available values: " + std::to_string(static_cast<int>(mParameter->mLowLimits)) + 
            " - " + std::to_string(static_cast<int>(mParameter->mHighLimits)));
    }
}

void ParameterLine::select(std::shared_ptr<GfxParameter> ptr)
{
    mSelectedValue = ptr;
    mSelectedParameter = mParameter;
    mSelectedLine = shared_from_this();
    mSelectedValue->mRect.setFillColor(GfxParameter::defaultSelectedRectColor);
    mSelectedValueIndex = static_cast<int>(mSelectedValue->mValText.getString().getSize());
    setCursorPos();

    const auto &parName = mSelectedParameter->mParName;
    if (parName == "Text bounds")
        GfxButton::setShowBounds(true);
    if (const auto i = Utility::retrieveNumber(parName, "@. Text bounds"); i != -1)
    {
        GfxButton::setShowBounds(true, i - 1);
    }
} 

void ParameterLine::deselect()
{
    if (mSelectedValue == nullptr && mSelectedLine == nullptr)
        return;

    mSelectedValue->mRect.setFillColor(GfxParameter::defaultRectColor);

    // Hide always just in case
    GfxButton::setShowBounds(false);
    mSelectedParameter = nullptr;
    mSelectedLine = nullptr;
    mSelectedValue = nullptr;
    mSelectedValueIndex = -1;
}

bool ParameterLine::isSelectedValHere() const
{
    for (const auto &elem : mParameterValues)
        if (elem == mSelectedValue)
            return true;

    return false;
}

bool ParameterLine::isItSelectedLine(const std::shared_ptr<ParameterLine> val) const
{
    return mSelectedLine == val;
}

void ParameterLine::setCursorPos()
{
    static auto text = sf::Text();
    static auto chSz = sf::Vector2f();
    if (text.getFont() == nullptr)
    {
        text.setFont(*mSelectedValue->mValText.getFont());
        text.setString("0");
        chSz.x = text.getLocalBounds().width;
        chSz.y = text.getLocalBounds().height;
    }

    // Take absolute position of the center of the button, substract by half width - the cursor is on the left bound,
    // then find the width of the part on the text left, and add it - the cursor is on the text left,
    // then take space in X axes for each character, substract it by 2 times spacing between them, 
    // and multiply by current cursor index - the cursor is on the index left
    const auto x = mSelectedValue->getPosition().x - mSelectedValue->mRect.getSize().x / 2 + 
        (mSelectedValue->mRect.getSize().x - mSelectedValue->mValText.getLocalBounds().width) / 2 +
        static_cast<float>(mSelectedValueIndex) * (chSz.x - text.getLetterSpacing() * 2);
    const auto y = mCursor.getPosition().y;

    mCursor.setPosition(x, y);
}

sf::Color ParameterLine::pickColor(LogicalParameter::Type type) const
{
    switch(type)
    {
        case LogicalParameter::Type::Empty: return sf::Color::Transparent;
        case LogicalParameter::Type::Collection: return Settings::UiTokens::CustomizationSurfaceVariant;
        case LogicalParameter::Type::Hint: return sf::Color(150, 0, 0);
        default: return Settings::UiTokens::CustomizationSurface;
    }
}

sf::Color ParameterLine::lineToColor(const std::shared_ptr<ParameterLine> linePtr) const
{
    assert(linePtr->mType == LogicalParameter::Type::Color);

    const auto values = linePtr->mParameterValues;
    sf::Uint8 rgba[4];
    auto i = 0ul;
    for (const auto &value : values)
    {
        const auto str = std::string(value->mValText.getString());
        rgba[i] = static_cast<sf::Uint8>(std::stoi(str));
        ++i;
    }

    return { rgba[0], rgba[1], rgba[2], rgba[3] };
}

bool ParameterLine::checkLimits(float check) const
{
    return check < mParameter->mLowLimits || check > mParameter->mHighLimits;
}

void ParameterLine::runThread(std::string &curVal, const std::string &prevVal)
{
    if (!mIsThRunning)
    {
        mWarningTh = std::thread(ParameterLine::warningVisualization, &mIsThRunning);
        mWarningTh.detach();
    }
    curVal = prevVal;
}

void ParameterLine::warningVisualization(bool *isRunning)
{
    *isRunning = true;
    
    auto mtx = std::mutex();
    auto gfxPar = mSelectedValue;
    const auto red = sf::Color(170,0,0);
    auto clock = sf::Clock();
    auto elapsedTime = sf::Time::Zero;
    auto totalTime = sf::Time::Zero;
    const auto timeToChange = sf::milliseconds(500);

    mtx.lock();
    // gfxPar->mValText.setFillColor(sf::Color::Red);
    gfxPar->mRect.setFillColor(red);
    mtx.unlock();

    while (totalTime.asMilliseconds() < timeToChange.asMilliseconds() * 4)
    {
        elapsedTime += clock.restart();

        if (elapsedTime >= timeToChange)
        {
            elapsedTime -= timeToChange;
            totalTime += timeToChange;

            mtx.lock();

            // gfxPar->mValText.setFillColor(gfxPar->mValText.getFillColor() == sf::Color::White ? 
            //     sf::Color::Red : sf::Color::White);
            gfxPar->mRect.setFillColor(gfxPar->mRect.getFillColor() != red ? red :
                gfxPar == mSelectedValue ? GfxParameter::defaultSelectedRectColor : GfxParameter::defaultRectColor);

            mtx.unlock();
        }
    }
    mtx.lock();
    gfxPar->mRect.setFillColor(gfxPar == mSelectedValue ? 
        GfxParameter::defaultSelectedRectColor : GfxParameter::defaultRectColor);
    mtx.unlock();

    *isRunning = false;
}

const std::shared_ptr<LogicalParameter> ParameterLine::getParameter() const
{
    return mParameter;
}

bool ParameterLine::resetState()
{
    return paramValWasChanged && !(paramValWasChanged = false);
}

void ParameterLine::setCharacterSize(unsigned size)
{
    mParameterName.setCharacterSize(size);
    // Don't let text go beyound 1.8 of the button width
    while (
       mType != LogicalParameter::Type::Collection 
    && mType != LogicalParameter::Type::Empty 
    && mType != LogicalParameter::Type::Hint
    && mParameterName.getLocalBounds().width >= mRectLine.getSize().x / 1.8)
    {
        mParameterName.setCharacterSize(mParameterName.getCharacterSize() - 1);
    }
}

void ParameterLine::setColor(sf::Color color)
{
    assert(mSelectedLine->mType == LogicalParameter::Type::Color);

    if (mSelectedLine->mType == LogicalParameter::Type::Color)
    {
        // Change Parameter
        mSelectedLine->mParameter->setColor(color);
        mSelectedLine->paramValWasChanged = true;

        // Change GraphicalParameter text
        mSelectedLine->mParameterValues[0]->mValText.setString(std::to_string(static_cast<unsigned>(color.r)));
        mSelectedLine->mParameterValues[1]->mValText.setString(std::to_string(static_cast<unsigned>(color.g)));
        mSelectedLine->mParameterValues[2]->mValText.setString(std::to_string(static_cast<unsigned>(color.b)));
        mSelectedLine->mParameterValues[0]->setupValPos();
        mSelectedLine->mParameterValues[1]->setupValPos();
        mSelectedLine->mParameterValues[2]->setupValPos();

        // Change cursor position
        for (auto &elem : mSelectedLine->mParameterValues)
        {
            if (elem == mSelectedValue)
            {
                mSelectedValueIndex = static_cast<int>(elem->mValText.getString().getSize());
                setCursorPos();
            }
        }
    }
}

LogicalParameter::Type ParameterLine::getType() const
{
    return mType;
}

bool ParameterLine::isHidden() const
{
    return getPosition() == sf::Vector2f(-1000.f, -1000.f);
}

bool ParameterLine::isEmpty(ParameterLine::ID id)
{
    return
        // id == ParameterLine::ID::StatTextMty ||
        // id == ParameterLine::ID::StatTextAdvMty ||
        id == ParameterLine::ID::StatTextAdvStrMty ||
        // id == ParameterLine::ID::BtnTextMty ||
        id == ParameterLine::ID::BtnTextAdvMty ||
        // id == ParameterLine::ID::BtnGfxMty ||
        id == ParameterLine::ID::BtnGfxAdvMty ||
        // id == ParameterLine::ID::AnimGfxLightMty ||
        id == ParameterLine::ID::AnimGfxPressMty ||
        id == ParameterLine::ID::MainWndwMty ||
        id == ParameterLine::ID::KPSWndwMty ||
        // id == ParameterLine::ID::KeyPressVisMty ||
        id == ParameterLine::ID::KeyPressVisAdvModeMty ||
        id == ParameterLine::ID::OtherMty ||
        // id == ParameterLine::ID::InfoMty ||
        id == ParameterLine::ID::LastLine;
}

bool ParameterLine::isToSkip(ParameterLine::ID id)
{
    auto isInBounds = [id] (ParameterLine::ID start, size_t count)
        {
            const auto idNum = static_cast<size_t>(id);
            const auto startNum = static_cast<size_t>(start);
            return idNum >= startNum && idNum < startNum + count;
        };

    return 
        (id >= ParameterLine::ID::SaveStatColl && id <= ParameterLine::ID::SaveStatMty) ||
        isInBounds(ParameterLine::ID::BtnTextAdvVisPosition, 4u * Settings::SupportedAdvancedKeysNumber) ||
        isInBounds(ParameterLine::ID::BtnTextAdvClr, 8u * Settings::SupportedAdvancedKeysNumber) ||
        isInBounds(ParameterLine::ID::BtnGfxBtnPos, 3u * Settings::SupportedAdvancedKeysNumber) ||
        isInBounds(ParameterLine::ID::KeyPressVisAdvModeSpeed, 7u * Settings::SupportedAdvancedKeysNumber) ||
        isInBounds(ParameterLine::ID::StatTextAdvPos, 7u * 3u);
}

void ParameterLine::deselectValue()
{
    if (mSelectedValue == nullptr && mSelectedLine == nullptr)
        return;

    mSelectedValue->mRect.setFillColor(GfxParameter::defaultRectColor);

    // Hide always just in case
    GfxButton::setShowBounds(false);
    mSelectedParameter = nullptr;
    mSelectedLine = nullptr;
    mSelectedValue = nullptr;
    mSelectedValueIndex = -1;
    mPalette.closeWindow();
}

bool ParameterLine::isValueSelected()
{
    return mSelectedValue.get();
}

bool ParameterLine::resetRefreshState()
{
    return mRefresh && !(mRefresh = false);
}

ParameterLine::ID ParameterLine::parIdToParLineId(LogicalParameter::ID id)
{
    const auto idNum = static_cast<size_t>(id);

    auto fromGroup = [idNum](LogicalParameter::ID logBase, ParameterLine::ID parBase, size_t count) -> std::optional<ParameterLine::ID>
    {
        const auto logBaseNum = static_cast<size_t>(logBase);
        if (idNum >= logBaseNum && idNum < logBaseNum + count)
            return static_cast<ParameterLine::ID>(static_cast<size_t>(parBase) + (idNum - logBaseNum));
        return std::nullopt;
    };

    if (auto result = fromGroup(LogicalParameter::ID::StatTextDist, ParameterLine::ID::StatTextDist, 17u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::StatTextAdvMode, ParameterLine::ID::StatTextAdvMode, 1u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::StatTextAdvPos, ParameterLine::ID::StatTextAdvPos,
        LogicalParameter::StatAdvCount * 7u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::StatTextKPSText, ParameterLine::ID::StatTextKPSText, 4u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::BtnTextFont, ParameterLine::ID::BtnTextFont, 18u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::BtnTextSepPosAdvMode, ParameterLine::ID::BtnTextSepPosAdvMode, 1u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::BtnTextAdvVisPosition, ParameterLine::ID::BtnTextAdvVisPosition,
        LogicalParameter::ButtonAdvCount * 4u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::BtnTextPosAdvMode, ParameterLine::ID::BtnTextPosAdvMode, 1u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::BtnTextAdvClr, ParameterLine::ID::BtnTextAdvClr,
        LogicalParameter::ButtonAdvCount * 8u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::BtnGfxDist, ParameterLine::ID::BtnGfxDist, 6u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::BtnGfxAdvMode, ParameterLine::ID::BtnGfxAdvMode, 1u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::BtnGfxBtnPos, ParameterLine::ID::BtnGfxBtnPos,
        LogicalParameter::ButtonAdvCount * 3u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::AnimGfxVel, ParameterLine::ID::AnimGfxVel, 1u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::AnimGfxLight, ParameterLine::ID::AnimGfxLight, 4u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::AnimGfxPress,
        static_cast<ParameterLine::ID>(static_cast<size_t>(ParameterLine::ID::AnimGfxPress) + 1u), 1u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::BgTxtr, ParameterLine::ID::BgTxtr, 3u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::MainWndwTitleBar, ParameterLine::ID::MainWndwTitleBar, 6u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::KPSWndwEn, ParameterLine::ID::KPSWndwEn, 11u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::KeyPressVisHint, ParameterLine::ID::KeyPressVisHint, 1u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::KeyPressVisToggle, ParameterLine::ID::KeyPressVisToggle, 8u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::KeyPressVisAdvMode, ParameterLine::ID::KeyPressVisAdvMode, 1u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::KeyPressVisAdvModeSpeed, ParameterLine::ID::KeyPressVisAdvModeSpeed,
        LogicalParameter::ButtonAdvCount * 7u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::OtherSaveStats, ParameterLine::ID::OtherSaveStats, 4u))
        return *result;
    if (auto result = fromGroup(LogicalParameter::ID::SaveStatMaxKPS, ParameterLine::ID::SaveStatMaxKPS,
        1u + LogicalParameter::ButtonAdvCount + 1u))
        return *result;

    std::cerr << "Non handled LogicalParameter::ID was passed - " << static_cast<size_t>(id) << std::endl;
    assert(false);
    return ParameterLine::ID::StatTextColl;
}
