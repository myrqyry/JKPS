#include "../Headers/KeysPerSecondGraph.hpp"
#include "../Headers/Button.hpp"
#include "../Headers/Settings.hpp"

#include <SFML/Window/Event.hpp>

#include <algorithm>
#include <cmath>


KeysPerSecondGraph::KeysPerSecondGraph()
: mLine(sf::LineStrip, 0)
{
    mSamples.fill(0.f);
}

void KeysPerSecondGraph::pushSample(float kps)
{
    mSamples[mHead] = kps;
    mHead = (mHead + 1u) % BufferCapacity;
    if (mCount < BufferCapacity)
        ++mCount;
}

void KeysPerSecondGraph::handleOwnEvent()
{
    auto event = sf::Event();
    while (mWindow.pollEvent(event))
    {
        if (event.type == sf::Event::KeyPressed)
        {
            const auto key = event.key;
            if (key.control && key.code == Settings::KeyExit)
                mWindow.close();
        }

        if (event.type == sf::Event::Closed)
        {
            mWindow.close();
        }
    }
}

void KeysPerSecondGraph::update()
{
    if (!mWindow.isOpen())
        return;

    // Sample the live aggregate KPS each update tick. Because the buffer is a
    // fixed ring, no allocation occurs and stale frames are never drawn.
    pushSample(Button::getKeysPerSecond());
}

void KeysPerSecondGraph::render()
{
    mWindow.clear(Settings::UiTokens::SurfaceColor);

    if (mCount > 0u)
    {
        const float viewWidth = 800.f;
        const float viewHeight = 600.f;
        const float padBottom = 20.f;
        const float padTop = 20.f;
        const float plotHeight = viewHeight - padBottom - padTop;

        // Auto-scale vertically to the busiest observed sample.
        float maxKps = 1.f;
        for (std::size_t i = 0u; i < mCount; ++i)
            maxKps = std::max(maxKps, mSamples[i]);

        mLine.resize(mCount);
        const float stepX = viewWidth / static_cast<float>(BufferCapacity - 1u);
        // Walk oldest -> newest so the line reads left-to-right over time.
        for (std::size_t i = 0u; i < mCount; ++i)
        {
            const std::size_t idx = (mHead + BufferCapacity - mCount + i) % BufferCapacity;
            const float x = static_cast<float>(i) * stepX;
            const float y = viewHeight - padBottom
                - (mSamples[idx] / maxKps) * plotHeight;
            mLine[i].position = sf::Vector2f(x, y);
            mLine[i].color = Settings::UiTokens::AccentColor;
        }
        mWindow.draw(mLine);
    }

    mWindow.display();
}

void KeysPerSecondGraph::openWindow()
{
    if (!mWindow.isOpen())
    {
        sf::ContextSettings settings;
        settings.antialiasingLevel = 8;
        mWindow.create(sf::VideoMode(800, 600), "Graph", sf::Style::Close, settings);
    }
}

void KeysPerSecondGraph::closeWindow()
{
    if (mWindow.isOpen())
        mWindow.close();
}

bool KeysPerSecondGraph::isOpen() const
{
    return mWindow.isOpen();
}

void KeysPerSecondGraph::updateParameters()
{
}

bool KeysPerSecondGraph::parameterIdMatches(LogicalParameter::ID /* id */)
{
    return false;
}
