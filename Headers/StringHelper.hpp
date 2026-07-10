#pragma once
 
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <string>
#include <string_view>

struct LogKey;


unsigned readAmountOfParms(std::string_view str);
std::string readValue(std::string_view str, unsigned n);
void addChOnIdx(std::string &str, unsigned idx, char ch);
void rmChOnIdx(std::string &str, unsigned idx);
std::string eraseDigitsOverHundredths(std::string_view floatStr);

std::string keyToStr(sf::Keyboard::Key key, bool saveToCfg = false);
sf::Keyboard::Key strToKey(std::string_view str);
std::string btnToStr(sf::Mouse::Button button);
sf::Mouse::Button strToBtn(std::string_view str);
bool isKey(std::string_view str);
bool isButton(std::string_view str);
std::string logKeyToStr(const LogKey &logKey);
char enumKeyToStr(sf::Keyboard::Key key);