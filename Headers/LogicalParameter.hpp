#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics/Color.hpp>

#include <cassert>
#include <algorithm>
#include <string>
#include <variant>


struct LogicalParameter
{
    public:
        static constexpr unsigned ButtonAdvCount = 20;
        static constexpr unsigned StatAdvCount = 3;
        enum class Type
        {
            Empty,
            Collection,
            Hint,
            Unsigned,
            Int,
            Bool,
            Float,
            String,
            StringPath,
            Color,
            VectorU,
            VectorI,
            VectorF,
        };

        enum class ID
        {
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

            StatTextAdvMode,
            StatTextAdvPos,
            StatTextAdvValPos,
            StatTextAdvCenterOrigin,
            StatTextAdvClr,
            StatTextAdvChSz,
            StatTextAdvBold,
            StatTextAdvItal,
            
            StatTextKPSText,
            StatTextKPS2Text,
            StatTextTotalText,
            StatTextBPMText,

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

            BtnTextSepPosAdvMode,
            BtnTextAdvVisPosition,
            BtnTextAdvTotPosition,
            BtnTextAdvKPSPosition,
            BtnTextAdvBPMPosition,

            BtnTextPosAdvMode,
            BtnTextAdvClr,
            BtnTextAdvChSz,
            BtnTextAdvOutThck,
            BtnTextAdvOutClr,
            BtnTextAdvPosition,
            BtnTextAdvBounds,
            BtnTextAdvBold,
            BtnTextAdvItal,

            BtnGfxDist,
            BtnGfxTxtr,
            BtnGfxTxtrSz,
            BtnGfxTxtrClr,
            BtnGfxBorderClr,
            BtnGfxShape,

            BtnGfxAdvMode,
            BtnGfxBtnPos,
            BtnGfxSz,
            BtnGfxClr,
            
            AnimGfxVel,
            
            AnimGfxLight,
            AnimGfxTxtr,
            AnimGfxScl,
            AnimGfxClr,
            AnimGfxPress,
            AnimGfxOffset,

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

            KeyPressVisHint,
            KeyPressVisToggle,
            KeyPressVisSpeed,
            KeyPressVisRotation,
            KeyPressVisFadeLineLen,
            KeyPressVisOrig,
            KeyPressVisColor,
            KeyPressVisWidthScale,
            KeyPressVisFixedHeight,

            KeyPressVisAdvMode,
            KeyPressVisAdvModeSpeed,
            KeyPressVisAdvModeRotation,
            KeyPressVisAdvModeFadeLineLen,
            KeyPressVisAdvModeOrig,
            KeyPressVisAdvModeColor,
            KeyPressVisAdvModeWidthScale,
            KeyPressVisAdvModeFixedHeight,

            OtherSaveStats,
            OtherShowOppOnAlt,
            OtherReduceMotion,
            OtherMultpl,

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
        };

        // Type-safe replacement for the former `void*` + union. The active
        // alternative is chosen at the call site, so passing a pointer whose
        // type does not match one of these alternatives is a compile error.
        // std::monostate represents "no value" (Empty / Collection / Hint).
        using ValuePtr = std::variant<
            std::monostate,
            unsigned *,
            int *,
            bool *,
            float *,
            std::string *,
            sf::Color *,
            sf::Vector2u *,
            sf::Vector2i *,
            sf::Vector2f *>;

        LogicalParameter(Type type, ValuePtr valPtr, const std::string &parName = "", const std::string &defVal = "", 
            float lowLimits = 0.f, float highLimits = 0.f, const std::string &val = "");

        template <typename T>
        void setDigit(T var);

        void setBool(const std::string &str);
        void setBool(bool b);
        void setString(const std::string &str);
        void setColor(sf::Color color);
        void setColor(const std::string &str, unsigned idx);

        template <typename T>
        void setVector(T vec);
        void setVector(const std::string &str, unsigned idx);


        template <typename T>
        T getDigit() const;

        bool getBool() const;
        std::string getString() const;
        sf::Color getColor() const;

        template <typename T>
        T getVector() const;

        std::string getValPt(int pt) const;
        void resetToDefaultValue();


        void setValStr(const std::string &str, unsigned idx = 0);
        std::string getValStr() const;
        const std::string &getDefValStr() const;

        bool resetChangeState();

        static std::string getInverseBool(bool b);
        

    public:
        const Type mType;
        const std::string mParName;
        float mLowLimits, mHighLimits;

    private:
        bool mChanged;
        ValuePtr mVal;
        const std::string mDefValStr;
        std::string mValStr;
};

template <typename T>
void LogicalParameter::setDigit(T var)
{
    assert(mType == Type::Unsigned || mType == Type::Int || mType == Type::Float);

    switch(mType)
    {
		case Type::Unsigned: 
			*std::get<unsigned *>(mVal) = static_cast<unsigned>(std::clamp(static_cast<float>(var), mLowLimits, mHighLimits));
			mValStr = std::to_string(static_cast<int>(*std::get<unsigned *>(mVal)));
			break;
        case Type::Int: 
			*std::get<int *>(mVal) = static_cast<int>(std::clamp(static_cast<float>(var), mLowLimits, mHighLimits));
			mValStr = std::to_string(static_cast<int>(*std::get<int *>(mVal)));
			break;
        case Type::Float: 
			*std::get<float *>(mVal) = std::clamp(static_cast<float>(var), mLowLimits, mHighLimits);
			mValStr = std::to_string(static_cast<int>(*std::get<float *>(mVal))); /*+ 1 dec digit*/
			break;
        
        default: break;
    }
    mChanged = true;
}

template <typename T>
void LogicalParameter::setVector(T vec)
{
    assert(mType == Type::VectorU || mType == Type::VectorI || mType == Type::VectorF);

    switch(mType)
    {
        case Type::VectorU: std::get<sf::Vector2u *>(mVal)->x = static_cast<unsigned>(vec.x); std::get<sf::Vector2u *>(mVal)->y = static_cast<unsigned>(vec.y); break;
        case Type::VectorI: std::get<sf::Vector2i *>(mVal)->x = static_cast<int>(vec.x); std::get<sf::Vector2i *>(mVal)->y = static_cast<int>(vec.y); break;
        case Type::VectorF: std::get<sf::Vector2f *>(mVal)->x = vec.x; std::get<sf::Vector2f *>(mVal)->y = vec.y; break;

        default: break;
    }

    mValStr = std::to_string(static_cast<int>(vec.x)) + ',' + std::to_string(static_cast<int>(vec.y));
    mChanged = true;
}

template <typename T>
T LogicalParameter::getDigit() const
{
    assert(mType == Type::Unsigned || mType == Type::Int || mType == Type::Bool || mType == Type::Float);

    switch(mType)
    {
        case Type::Unsigned: return static_cast<T>(*std::get<unsigned *>(mVal));
        case Type::Int: return static_cast<T>(*std::get<int *>(mVal));
        case Type::Bool: return static_cast<T>(*std::get<bool *>(mVal));
        case Type::Float: return static_cast<T>(*std::get<float *>(mVal));
        default: return T{};
    }
}

template <typename T>
T LogicalParameter::getVector() const
{
    assert(mType == Type::VectorU || mType == Type::VectorI || mType == Type::VectorF);

    switch(mType)
    {
        case Type::VectorU: return *std::get<sf::Vector2u *>(mVal);
        case Type::VectorI: return *std::get<sf::Vector2i *>(mVal);
        case Type::VectorF: return *std::get<sf::Vector2f *>(mVal);
        default: return T(-1, -1);
    }
}
