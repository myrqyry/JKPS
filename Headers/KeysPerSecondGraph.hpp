#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/VertexArray.hpp>

#include "LogicalParameter.hpp"

#include <array>
#include <cstddef>


class KeysPerSecondGraph
{
    public:
        KeysPerSecondGraph();

        void handleOwnEvent();
        void update();
        void render();

        void openWindow();
        void closeWindow();
        bool isOpen() const;

        void updateParameters();
        static bool parameterIdMatches(LogicalParameter::ID id);


    private:
        // Fixed-capacity ring buffer of sampled KPS values. Capacity is constant
        // so no allocation happens while the graph is live; sampling wraps around
        // and overwrites the oldest sample.
        static constexpr std::size_t BufferCapacity = 256u;

        void pushSample(float kps);

        sf::RenderWindow mWindow;

        std::array<float, BufferCapacity> mSamples;
        std::size_t mHead { 0u };
        std::size_t mCount { 0u };

        sf::VertexArray mLine;
};