#include "../Headers/LogicalParameter.hpp"
#include "../Headers/ConfigHelper.hpp"

#include <string.h>
#include <iostream>
#include <algorithm>
#include <cctype>


LogicalParameter::LogicalParameter(Type type, ValuePtr valPtr, const std::string &parName, const std::string &defVal, 
    float lowLimits, float highLimits, const std::string &val)
: mType(type)
, mParName(parName)
, mLowLimits(lowLimits)
, mHighLimits(highLimits)
, mChanged(false)
, mVal(valPtr)
, mDefValStr(defVal)
{ 
    if (type != Type::Bool && type != Type::String && type != Type::StringPath)
    {
        if (defVal.find(' ') != std::string::npos)
        {
            std::cerr << "LogicalParameter - parName \"" 
                + parName + "\"" + " defVal \"" 
                + defVal + "\" has a space" << std::endl;
            exit(1);
        }
    }

    if (type == Type::Color)
        this->mHighLimits = 255;
    if (val == "") 
        this->mValStr = defVal; 
    
    switch(type)
    {
        case Type::Unsigned: 
            setDigit<unsigned>(static_cast<unsigned>(std::stoi(defVal)));
            break;
            
        case Type::Int: 
            setDigit<int>(std::stoi(defVal)); 
            break;

        case Type::Bool: 
            setBool(defVal); 
            break;

        case Type::Float: 
            setDigit<float>(std::stof(defVal));
            break;

        case Type::String: 
        case Type::StringPath:
            setString(defVal); 
            break;

        case Type::Color: 
            setColor(ConfigHelper::readColorParameter(*this, defVal));
            break;

        case Type::VectorU: 
            setVector(ConfigHelper::readVectorParameter(*this, defVal));
            break;

        case Type::VectorI: 
            setVector(ConfigHelper::readVectorParameter(*this, defVal));
            break;

        case Type::VectorF: 
            setVector(ConfigHelper::readVectorParameter(*this, defVal));
            break;

        default: break; // Empty or Collection
    }
} 

void LogicalParameter::setColor(sf::Color color)
{
    assert(mType == Type::Color);

    auto *cP = std::get<sf::Color *>(mVal);
    *cP = color;

    mValStr = 
        std::to_string(static_cast<int>(cP->r)) + ',' + 
        std::to_string(static_cast<int>(cP->g)) + ',' + 
        std::to_string(static_cast<int>(cP->b)) + ',' + 
        std::to_string(static_cast<int>(cP->a));
    mChanged = true;
}

void LogicalParameter::setColor(const std::string &str, unsigned idx)
{
    assert(mType == Type::Color);
    assert(idx <= 3u);

    auto *cP = std::get<sf::Color *>(mVal);
    unsigned char c = static_cast<unsigned char>(std::stoi(str));
    switch (idx)
    {
        case 0: cP->r = c; break;
        case 1: cP->g = c; break;
        case 2: cP->b = c; break;
        case 3: cP->a = c; break;
        default: break;
    }
    mValStr = 
        std::to_string(static_cast<int>(cP->r)) + ',' + 
        std::to_string(static_cast<int>(cP->g)) + ',' + 
        std::to_string(static_cast<int>(cP->b)) + ',' + 
        std::to_string(static_cast<int>(cP->a));
    mChanged = true;
}

void LogicalParameter::setVector(const std::string &str, unsigned idx)
{
    assert(mType == Type::VectorU || mType == Type::VectorI || mType == Type::VectorF);
    assert(idx == 0u || idx == 1u);

    switch(mType)
    {
        case Type::VectorU: 
            {
                auto *vUp = std::get<sf::Vector2u *>(mVal);
                switch(idx)
                {
                    case 0: vUp->x = static_cast<unsigned>(std::stoi(str)); break;
                    case 1: vUp->y = static_cast<unsigned>(std::stoi(str)); break;
                }
                mValStr = std::to_string(vUp->x) + ',' + std::to_string(vUp->y);
            }
            break;
        case Type::VectorI: 
            {
                auto *vIp = std::get<sf::Vector2i *>(mVal);
                switch(idx)
                {
                    case 0: vIp->x = std::stoi(str); break;
                    case 1: vIp->y = std::stoi(str); break;
                }
                mValStr = std::to_string(vIp->x) + ',' + std::to_string(vIp->y);
            }
            break;
        case Type::VectorF: 
            {
                auto *vFp = std::get<sf::Vector2f *>(mVal);
                switch(idx)
                {
                    case 0: vFp->x = static_cast<float>(std::stof(str)); break;
                    case 1: vFp->y = static_cast<float>(std::stof(str)); break;
                }
                mValStr = std::to_string(static_cast<int>(vFp->x)) + ',' + std::to_string(static_cast<int>(vFp->y));
            }
            break;
        default: break;
    }
    mChanged = true;
}


void LogicalParameter::setBool(const std::string &str)
{
    assert(mType == Type::Bool); 
    assert(
        str == "true" || str == "false"
    ||  str == "True" || str == "False"
    ||  str == "TRUE" || str == "FALSE");

    std::string lower(str);
    std::transform(lower.begin(), lower.end(), lower.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    *std::get<bool *>(mVal) = (lower == "true");
    mValStr = str;
    mChanged = true;
}

void LogicalParameter::setBool(bool b)
{
    assert(mType == Type::Bool);
    *std::get<bool *>(mVal) = b;
    mValStr = b ? "True" : "False";
    mChanged = true;
}

void LogicalParameter::setString(const std::string &str)
{
    assert(mType == Type::String || mType == Type::StringPath);
    *std::get<std::string *>(mVal) = str;
    mValStr = str;
    mChanged = true;
}

bool LogicalParameter::getBool() const
{
    assert(mType == Type::Bool);
    return *std::get<bool *>(mVal);
}

std::string LogicalParameter::getString() const
{
    assert(mType == Type::String || mType == Type::StringPath);
    return *std::get<std::string *>(mVal);
}

sf::Color LogicalParameter::getColor() const
{
    assert(mType == Type::Color);
    return *std::get<sf::Color *>(mVal);
}

std::string LogicalParameter::getValPt(int pt) const
{
    assert(mType == Type::Color || mType == Type::VectorU 
    || mType == Type::VectorI || mType == Type::VectorF);

    assert(pt >= 0 && pt <= 3);
    // Don't allow to pass pt >= 2, when type is not color
    assert(!(pt >= 2 && mType != Type::Color));

    std::string res = mValStr; 
    while(--pt >= 0)
        res = res.substr(res.find(',') + 1); 

    if (res.find(',') != std::string::npos)
        res = res.substr(0, res.find(','));

    return res;
}

void LogicalParameter::setValStr(const std::string &str, unsigned idx)
{
    mValStr = str;
    switch (mType)
    {
        case LogicalParameter::Type::Unsigned:
        case LogicalParameter::Type::Int: setDigit(std::stoi(str)); break;
        case LogicalParameter::Type::Float: setDigit(std::stof(str)); break;
        case LogicalParameter::Type::Bool: setBool(str); break;
        case LogicalParameter::Type::String:
        case LogicalParameter::Type::StringPath: setString(str); break;
        case LogicalParameter::Type::Color: setColor(str, idx); break;
        case LogicalParameter::Type::VectorU:
        case LogicalParameter::Type::VectorI:
        case LogicalParameter::Type::VectorF: setVector(str, idx); break;
        default: break;
    }
}


std::string LogicalParameter::getValStr() const
{
    return mValStr;
}

const std::string &LogicalParameter::getDefValStr() const
{
    return mDefValStr;
}

bool LogicalParameter::resetChangeState()
{
    return mChanged && !(mChanged = false);
}


std::string LogicalParameter::getInverseBool(bool b)
{
    return b ? "False" : "True";
}

void LogicalParameter::resetToDefaultValue()
{
    switch(mType)
    {
        case Type::Unsigned: 
            setDigit<unsigned>(static_cast<unsigned>(std::stoi(mDefValStr)));
            break;
            
        case Type::Int: 
            setDigit<int>(std::stoi(mDefValStr)); 
            break;

        case Type::Bool: 
            setBool(mDefValStr); 
            break;

        case Type::Float: 
            setDigit<float>(std::stof(mDefValStr)); 
            break;

        case Type::String: 
        case Type::StringPath:
            setString(mDefValStr); 
            break;

        case Type::Color: 
            setColor(ConfigHelper::readColorParameter(*this, mDefValStr)); 
            break;

        case Type::VectorU: 
            setVector(ConfigHelper::readVectorParameter(*this, mDefValStr)); 
            break;

        case Type::VectorI: 
            setVector(ConfigHelper::readVectorParameter(*this, mDefValStr)); 
            break;

        case Type::VectorF: 
            setVector(ConfigHelper::readVectorParameter(*this, mDefValStr)); 
            break;

        default: break; // Empty or Collection
    }
}
