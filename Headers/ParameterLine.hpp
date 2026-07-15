#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include "GfxParameter.hpp"
#include "LogicalParameter.hpp"
#include "ResourceIdentifiers.hpp"
#include "ColorButton.hpp"
#include "Palette.hpp"

#include <memory>
#include <iostream>
#include <thread>


class Palette;

namespace sf
{
    class Event;
    class RenderWindow;
}

class ParameterLine : public sf::Drawable, public sf::Transformable, public std::enable_shared_from_this<ParameterLine>
{
    public:
        enum class ID
        {
            StatTextColl,
            StatTextDist,
            StatPos,
            StatValPos,
            StatTextCenterOrigin,
            StatTextFont,
            StatTextClr,
            StatTextChSz,
            StatTextOutThck,
            StatTextOutClr,
            StatTextBold,
            StatTextItal,
            StatTextShow,
			StatTextShowKPS,
			StatTextShowMaxKPS,
            StatTextShowTotal,
            StatTextShowBPM,
            StatTextMty,

            StatTextAdvColl,
            StatTextAdvMode,
            StatTextSpace1,
            StatTextAdvPos,
            StatTextAdvValPos,
            StatTextAdvCenterOrigin,
            StatTextAdvClr,
            StatTextAdvChSz,
            StatTextAdvBold,
            StatTextAdvItal,
            StatTextAdvMty,

            StatTextAdvStrColl,
            StatTextKPSText,
            StatTextKPS2Text,
            StatTextTotalText,
            StatTextBPMText,
            StatTextAdvStrMty,

            BtnTextColl,
            BtnTextFont,
            BtnTextClr,
            BtnTextChSz,
            BtnTextOutThck,
            BtnTextOutClr,
            BtnTextPosition,
            BtnTextBoundsToggle,
            BtnTextBounds,
            BtnTextIgnoreBtnMovement,
            BtnTextBold,
            BtnTextItal,
            BtnTextShowVisKeys,
            BtnTextVisPosition,
            BtnTextShowTot,
            BtnTextTotPosition,
            BtnTextShowKps,
            BtnTextKPSPosition,
            BtnTextShowBpm,
            BtnTextBPMPosition,
            BtnTextMty,

            BtnTextAdvColl,
            BtnTextSepPosAdvMode,
            BtnTextAdvSpace1,
            BtnTextAdvVisPosition,
            BtnTextAdvTotPosition,
            BtnTextAdvKPSPosition,
            BtnTextAdvBPMPosition,

            BtnTextAdvSpace2,
            BtnTextPosAdvMode,
            BtnTextAdvSpace3,
            BtnTextAdvClr,
            BtnTextAdvChSz,
            BtnTextAdvOutThck,
            BtnTextAdvOutClr,
            BtnTextAdvPosition,
            BtnTextAdvBounds,
            BtnTextAdvBold,
            BtnTextAdvItal,
            BtnTextAdvMty,

            BtnGfxColl,
            BtnGfxDist,
            BtnGfxTxtr,
            BtnGfxTxtrSz,
            BtnGfxTxtrClr,
            BtnGfxBorderClr,
            BtnGfxShape,
            BtnGfxMty,

            BtnGfxAdvColl,
            BtnGfxAdvMode,
            BtnGfxSpace,
            BtnGfxBtnPos,
            BtnGfxSz,
            BtnGfxClr,
            BtnGfxAdvMty,

            AnimGfxColl,
            AnimGfxVel,
            AnimGfxMty,

            AnimGfxLightColl,
            AnimGfxLight,
            AnimGfxTxtr,
            AnimGfxScl,
            AnimGfxClr,
            AnimGfxLightMty,

            AnimGfxPressColl,
            AnimGfxPress,
            AnimGfxOffset,
            AnimGfxPressMty,

            MainWndwColl,
            BgTxtr,
            BgClr,
            BgScale,
            MainWndwTitleBar,
            MainWndwResizable,
			RenderUpdateFrequency,
            MainWndwTop,
            MainWndwBot,
            MainWndwLft,
            MainWndwRght,
            MainWndwMty,

            KPSWndwColl,
            KPSWndwEn,
            KPSWndwSz,
            KPSWndwTxtChSz,
            KPSWndwNumChSz,
            KPSWndwBgClr,
            KPSWndwTxtClr,
            KPSWndwNumClr,
            KPSWndwTxtFont,
            KPSWndwNumFont,
            KPSWndwTopPadding,
            KPSWndwDistBtw,
            KPSWndwMty,

            KeyPressVisHint,
            KeyPressVisColl,
            KeyPressVisToggle,
            KeyPressVisSpeed,
            KeyPressVisRotation,
            KeyPressVisFadeLineLen,
            KeyPressVisOrig,
            KeyPressVisColor,
            KeyPressVisWidthScale,
            KeyPressVisFixedHeight,
            KeyPressVisMty,

            KeyPressVisAdvModeColl,
            KeyPressVisAdvMode,
            KeyPressVisAdvModeSpace,
            KeyPressVisAdvModeSpeed,
            KeyPressVisAdvModeRotation,
            KeyPressVisAdvModeFadeLineLen,
            KeyPressVisAdvModeOrig,
            KeyPressVisAdvModeColor,
            KeyPressVisAdvModeWidthScale,
            KeyPressVisAdvModeFixedHeight,
            KeyPressVisAdvModeMty,

            OtherColl,
            OtherSaveStats,
            OtherShowOppOnAlt,
            OtherReduceMotion,
            OtherMultpl,
            OtherMty,

            SaveStatColl,
            SaveStatMaxKPS,
            SaveStatTotal,
            SaveStatTotal1,
            SaveStatTotal2,
            SaveStatTotal3,
            SaveStatTotal4,
            SaveStatTotal5,
            SaveStatTotal6,
            SaveStatTotal7,
            SaveStatTotal8,
            SaveStatTotal9,
            SaveStatTotal10,
            SaveStatTotal11,
            SaveStatTotal12,
            SaveStatTotal13,
            SaveStatTotal14,
            SaveStatTotal15,
            SaveStatTotal16,
            SaveStatTotal17,
            SaveStatTotal18,
            SaveStatTotal19,
            SaveStatTotal20,
            SaveStatMty,

            InfoColl,
            Info1,
            Info2,
            Info3,
            Info4,
            InfoMty,

            HotkeyColl,
            HotKey1,
            HotKey2,
            HotKey3,
            HotKey4,
            HotKey5,
            HotKey6,
            HotKey7,
            HotKey8,
            HotKey9,
            HotKey10,
            HotKey11,
            HotKey12,
            HotKey13,

            ProgramVersion,
            LastLine
        };


    public:
        ParameterLine(
            std::shared_ptr<LogicalParameter> parameter,
            const FontHolder &fonts, 
            const TextureHolder &textures,
            sf::RenderWindow &window);

        bool handleEvent(sf::Event event);
        void processInput();
        virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override;

        bool resetState();
        void setCharacterSize(unsigned size);

        const std::shared_ptr<LogicalParameter> getParameter() const;
        LogicalParameter::Type getType() const;

        static void setColor(sf::Color color);
        static ParameterLine::ID parIdToParLineId(LogicalParameter::ID id);
        static bool isEmpty(ParameterLine::ID id);
        static bool isToSkip(ParameterLine::ID id);
        static void deselectValue();
        static bool isValueSelected();
        static bool resetRefreshState();


    private:
        // Clicks with keyboard to modify value of buttons
        bool handleValueModEvent(sf::Event event);
        bool handleButtonsInteractionEvent(sf::Event event);

        bool tabulation();
        bool selectRgbCircle(sf::Mouse::Button button, sf::Vector2f mousePos);

        void buildButtons(const std::string &valueStr, const FontHolder &fonts, const TextureHolder &textures);
        void buildLimits(const FontHolder &fonts);

        void select(std::shared_ptr<GfxParameter> ptr);
        void deselect();
        bool isSelectedValHere() const;
        bool isItSelectedLine(const std::shared_ptr<ParameterLine> val) const;
        static void setCursorPos();

        sf::Color pickColor(LogicalParameter::Type type) const;
        sf::Color lineToColor(const std::shared_ptr<ParameterLine> linePtr) const;

        bool checkLimits(float check) const;
        void runThread(std::string &curVal, const std::string &prevVal);
        static void warningVisualization(bool *isRunning);

        bool isHidden() const;


    private:
        sf::RenderWindow &mWindow;

        const LogicalParameter::Type mType;
        sf::RectangleShape mRectLine;
        sf::Text mParameterName;
        sf::Text mLimits;
        std::shared_ptr<LogicalParameter> mParameter;
        std::vector<std::shared_ptr<GfxParameter>> mParameterValues;
        std::unique_ptr<ColorButton> mColorButtonP;

        static sf::RectangleShape mCursor;
        static std::shared_ptr<LogicalParameter> mSelectedParameter;
        static std::shared_ptr<ParameterLine> mSelectedLine;
        static std::shared_ptr<GfxParameter> mSelectedValue;
        static int mSelectedValueIndex;
        bool paramValWasChanged;
        std::thread mWarningTh;
        bool mIsThRunning;

        static Palette mPalette;
        static bool mRefresh;
};
