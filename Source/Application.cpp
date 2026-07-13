#include "../Headers/Application.hpp"
#include "../Headers/Menu.hpp"
#include "../Headers/LogicalParameter.hpp"
#include "../Headers/ChangedParametersQueue.hpp"
#include "../Headers/Settings.hpp"
#include "../Headers/DefaultFiles.hpp"
#include "../Headers/ConfigHelper.hpp"

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Transformable.hpp>

const unsigned HooksUpdateFrequency = 60u;
const sf::Time Application::TimePerHookUpdate = sf::seconds(1.f / static_cast<float>(HooksUpdateFrequency));

Application::Application()
{
    loadTextures();
    loadFonts();

    buildButtons();
    buildStatistics();

    openWindow();
    loadIcon();

    auto keySelector = std::make_unique<GfxButtonSelector>();
    mGfxButtonSelector = std::move(keySelector);

    auto buttonPositioner = std::make_unique<ButtonPositioner>(&mButtons);
    mButtonsPositioner = std::move(buttonPositioner);
    (*mButtonsPositioner)();

    auto statPositioner = std::make_unique<StatisticsPositioner>(&mStatistics);
    mStatisticsPositioner = std::move(statPositioner);
    (*mStatisticsPositioner)();

    auto bg = std::make_unique<Background>(mTextures, mWindow);
    mBackground = std::move(bg);

    auto kpsWindow = std::make_unique<KPSWindow>(mFonts);
    mKPSWindow = std::move(kpsWindow);

    auto graph = std::make_unique<KeysPerSecondGraph>();
    mGraph = std::move(graph);

    auto styleWizard = std::make_unique<StyleWizard>(mFonts, mTextures);
    mStyleWizard = std::move(styleWizard);

    mMenu.saveConfig(mButtons);
}

void Application::run()
{
    sf::Clock clock;
	auto timeSinceLastEventUpdate = sf::Time::Zero;
	auto timeSinceLastHooksUpdate = sf::Time::Zero;

    // Clamp accumulated time so that a long stall (system sleep/wake, breakpoint
    // pause) cannot force a burst of catch-up updates that starves render().
    const sf::Time MaxFrameTime = sf::seconds(0.1f);

    while (mWindow.isOpen())
    {
        auto dt = clock.restart();
        if (dt > MaxFrameTime)
            dt = MaxFrameTime;
		timeSinceLastEventUpdate += dt;
		timeSinceLastHooksUpdate += dt;

		const sf::Time TimePerEventUpdate = sf::seconds(1.f / static_cast<float>(getRenderUpdateFrequency()));
		while (true)
		{
			int updateType = UpdateType::None;
			if (timeSinceLastHooksUpdate > TimePerHookUpdate)
			{
				timeSinceLastHooksUpdate -= TimePerHookUpdate;
				updateType |= UpdateType::Hooks;
			}

			if (timeSinceLastEventUpdate > TimePerEventUpdate)
			{
				timeSinceLastEventUpdate -= TimePerEventUpdate;
				updateType |= UpdateType::Event;
			}

			if (updateType == UpdateType::None)
			{
				break;
			}

			processInput(static_cast<UpdateType>(updateType));
			update(TimePerEventUpdate.asSeconds(), static_cast<UpdateType>(updateType));
		}

        render();
    }
}

void Application::processInput(UpdateType type)
{
	if (type & UpdateType::Event)
	{
		// Open/close other windows, add/rm keys
		handleEvent();

		if (mStyleWizard->resetApplyRequest())
		{
			mMenu.reloadConfig();
			resetAssets();
		}

		// Update changed parameters
		unloadChangesQueue();

		// Update assets if there is a request
		if (ParameterLine::resetRefreshState())
			resetAssets();
	}

	if (type & UpdateType::Hooks)
	{
		// Take buttons realtime input
		for (auto &button : mButtons)
			button->processInput();
	}

	if (type & UpdateType::Event)
	{
		if (!Settings::WindowTitleBar && !isSecondaryUiActive())
			moveWindow();

		// Make separate windows handle own events
		if (mMenu.isOpen())
			mMenu.processInput();
		if (mGfxButtonSelector->isOpen())
			mGfxButtonSelector->handleOwnInput();
		if (mKPSWindow->isOpen())
			mKPSWindow->handleOwnEvent();
		if (mGraph->isOpen())
			mGraph->handleOwnEvent();
		if (mStyleWizard->isWindowOpen())
			mStyleWizard->processInput();
	}
}

void Application::handleEvent()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        if (event.type == sf::Event::MouseButtonPressed)
        {
            const auto button = event.mouseButton.button;
            if (button == sf::Mouse::Right)
            {
                auto idx = 0u;
                if (isPressPerformedOnButton(idx))
                {
                    mGfxButtonSelector->setKey(mButtons[idx]->getLogKey());
                    mGfxButtonSelector->openWindow();
                }
            }
            else if (button == sf::Mouse::Left)
            {
                if (!Settings::WindowTitleBar && !isSecondaryUiActive() && mWindow.hasFocus())
                {
                    mDraggingWindow = true;
                    mDragGrabOffset = sf::Mouse::getPosition() - mWindow.getPosition();
                }
            }
        }

        if (event.type == sf::Event::MouseButtonReleased)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
                mDraggingWindow = false;
        }

        if (event.type == sf::Event::KeyPressed)
        {
            const auto key = event.key;
            if (key.control)
            {
                auto btnAmtChanged = false;
                if (key.code == Settings::KeyToIncreaseKeys || key.code == Settings::AltKeyToIncreaseKeys)
                {
                    {
                        LogKey key("A", "A", new sf::Keyboard::Key(sf::Keyboard::A), nullptr);
                        addButton(key);
                    }
                    btnAmtChanged = true;
                }

                if (key.code == Settings::KeyToIncreaseButtons)
                {
                    {
                        LogKey key("M Left", "M Left", nullptr, new sf::Mouse::Button(sf::Mouse::Left));
                        addButton(key);
                    }
                    btnAmtChanged = true;
                }

                if (key.code == Settings::KeyToDecreaseKeys || key.code == Settings::AltKeyToDecreaseKeys || key.code == Settings::KeyToDecreaseButtons)
                {
                    removeButton();
                    btnAmtChanged = true;
                }

                if (btnAmtChanged)
                {
                    (*mButtonsPositioner)();
                    (*mStatisticsPositioner)();
                    resizeWindow();
                    mBackground->rescale();
                }

                if (key.code == Settings::KeyToOpenKPSWindow)
                {
                    if (mKPSWindow->isOpen())
                        mKPSWindow->closeWindow();
                    else
                        mKPSWindow->openWindow();
                }

                if (key.code == Settings::KeyToOpenMenuWindow)
                {
                    if (mMenu.isOpen())
                        returnToOverlayMode();
                    else
                        openMenuMode();
                }

                if (key.code == Settings::KeyToOpenStyleWizard)
                {
                    if (mStyleWizard->isWindowOpen())
                        returnToOverlayMode();
                    else
                        openStyleMode();
                }

                if (key.code == Settings::KeyToOpenGraphWindow)
                {
                    if (mGraph->isOpen())
                        mGraph->closeWindow();
                    else
                        mGraph->openWindow();
                }

                if (key.code == Settings::KeyToReset)
                {
                    LogButton::resetGlobal();
                    for (auto &button : mButtons)
                        button->reset();
                }

                if (key.code == Settings::KeyExit)
                {
                    mMenu.saveConfig(mButtons);
                    mWindow.close();
                    return;
                }
            }
        }

        if (event.type == sf::Event::Closed)
        {
            mMenu.saveConfig(mButtons);
            mWindow.close();
            return;
        }

        if (event.type == sf::Event::Resized)
        {
            auto view = sf::View(sf::FloatRect(0.f, 0.f, static_cast<float>(event.size.width), static_cast<float>(event.size.height)));
            mWindow.setView(view);
        }
    }
}

void Application::update(float deltaSeconds, UpdateType type)
{
	if (type & UpdateType::Event)
	{
		for (auto &button : mButtons)
			button->update(deltaSeconds);
		for (auto &line : mStatistics)
			line->update();

		if (mMenu.isOpen())
			mMenu.update();

		if (mKPSWindow->isOpen())
			mKPSWindow->update();

		if (mGraph->isOpen())
		    mGraph->update();
	}

	if (type & UpdateType::Hooks)
	{
		Button::moveIndex();
		for (auto &button : mButtons)
			button->accumulateBeatsPerMinute();
	}
}

void Application::render()
{
    mWindow.clear();

    mWindow.draw(*mBackground);

    for (const auto &elem : mButtons)
        mWindow.draw(*elem);
    for (const auto &elem : mStatistics)
        mWindow.draw(*elem);

    if (mMenu.isOpen())
        mMenu.render();
    if (mGfxButtonSelector->isOpen())
        mGfxButtonSelector->render();
    if (mKPSWindow->isOpen())
        mKPSWindow->render();
    if (mGraph->isOpen())
        mGraph->render();
    if (mStyleWizard->isWindowOpen())
        mStyleWizard->render();

    mWindow.display();
}

void Application::unloadChangesQueue()
{
    auto &queue = mMenu.getChangedParametersQueue();
    while (!queue.isEmpty())
    {
        auto pair = queue.pop();

        if (GfxStatisticsLine::parameterIdMatches(pair.first))
        {
            for (auto &line : mStatistics)
                line->updateParameters();
            (*mStatisticsPositioner)();
        }

        if (Button::parameterIdMatches(pair.first))
        {
            unsigned idx = 0;
            for (auto &button : mButtons)
            {
                button->updateParameters();
                ++idx;
            }
            (*mButtonsPositioner)();
        }

        if (KPSWindow::parameterIdMatches(pair.first))
        {
            mKPSWindow->updateParameters();
        }

        if (parameterIdMatches(pair.first))
        {
            mWindow.setSize(sf::Vector2u(getWindowWidth(), getWindowHeight()));
            mWindow.setView(sf::View(sf::FloatRect(0.f, 0.f, static_cast<float>(mWindow.getSize().x), static_cast<float>(mWindow.getSize().y))));
            mMenu.requestFocus();
        }

        if (pair.first == LogicalParameter::ID::MainWndwTitleBar || pair.first == LogicalParameter::ID::MainWndwResizable)
        {
            openWindow();
        }

		if (pair.first == LogicalParameter::ID::RenderUpdateFrequency)
		{
			mWindow.setFramerateLimit(getApplicationUpdateFrequency());
		}

        mBackground->rescale();
    }
}

void Application::resetAssets()
{
    mFonts.clear();
    mTextures.clear();

    loadTextures();
    loadFonts();

    auto idx = 0u;
    for (auto &button : mButtons)
    {
        button->updateAssets();
        button->setPosition(Button::getWidth(idx), Button::getHeight(idx));
        ++idx;
    }

    for (auto &line : mStatistics)
        line->updateAsset();

    mBackground->updateAssets();
    (*mButtonsPositioner)();
    (*mStatisticsPositioner)();

    mMenu.saveConfig(mButtons);
}

void Application::loadTextures()
{
    if (!mTextures.loadFromFile(Textures::Button, Settings::GfxButtonTexturePath))
        mTextures.loadFromMemory(Textures::Button, Settings::DefaultButtonTexture, 2700);

    if (!mTextures.loadFromFile(Textures::Animation, Settings::AnimationTexturePath))
        mTextures.loadFromMemory(Textures::Animation, Settings::DefaultAnimationTexture, 15800);

    // mTextures.loadFromMemory(Textures::KeyPressVis, Settings::KeyPressVisTexture, 4200);

    Settings::isGreenscreenSet = Settings::BackgroundTexturePath == "GreenscreenBG.png";
    if (Settings::isGreenscreenSet)
        mTextures.loadFromMemory(Textures::Background, Settings::DefaultGreenscreenBackgroundTexture, 596);
    else
    {
        if (!mTextures.loadFromFile(Textures::Background, Settings::BackgroundTexturePath))
            mTextures.loadFromMemory(Textures::Background, Settings::DefaultBackgroundTexture, 2700);
    }
}

void Application::loadFonts()
{
    if (!mFonts.loadFromFile(Fonts::ButtonValue, Settings::ButtonTextFontPath))
        mFonts.loadFromMemory(Fonts::ButtonValue, Settings::KeyCountersDefaultFont, 581700);

    if (!mFonts.loadFromFile(Fonts::Statistics, Settings::StatisticsTextFontPath))
        mFonts.loadFromMemory(Fonts::Statistics, Settings::StatisticsDefaultFont, 581700);

    if (!mFonts.loadFromFile(Fonts::KPSText, Settings::KPSWindowTextFontPath))
        mFonts.loadFromMemory(Fonts::KPSText, Settings::DefaultKPSWindowFont, 459300);

    if (!mFonts.loadFromFile(Fonts::KPSNumber, Settings::KPSWindowNumberFontPath))
        mFonts.loadFromMemory(Fonts::KPSNumber, Settings::DefaultKPSWindowFont, 459300);
}

void Application::loadIcon()
{
    sf::Image icon;
    icon.loadFromMemory(IconTexture, 53200);
    mWindow.setIcon(256, 256, icon.getPixelsPtr());
}

void Application::buildStatistics()
{
    using Ptr = std::unique_ptr<GfxStatisticsLine>;
    auto linePtr = Ptr();
    auto id = static_cast<unsigned>(GfxStatisticsLine::StatisticsID::KPS);

    linePtr = Ptr(new GfxStatisticsLine(mFonts, Settings::ShowStatisticsKPS, static_cast<GfxStatisticsLine::StatisticsID>(id)));
    mStatistics[id] = std::move(linePtr);
    ++id;

    linePtr = Ptr(new GfxStatisticsLine(mFonts, Settings::ShowStatisticsTotal, static_cast<GfxStatisticsLine::StatisticsID>(id)));
    mStatistics[id] = std::move(linePtr);
    ++id;

    linePtr = Ptr(new GfxStatisticsLine(mFonts, Settings::ShowStatisticsBPM, static_cast<GfxStatisticsLine::StatisticsID>(id)));
    mStatistics[id] = std::move(linePtr);
    ++id;
}

void Application::buildButtons()
{
    auto logKeyQueue = ConfigHelper::oldGetLogKeys();
    auto logKeyBtnsQueue = ConfigHelper::oldGetLogButtons();

    while (logKeyBtnsQueue.size())
    {
        logKeyQueue.push(logKeyBtnsQueue.front());
        logKeyBtnsQueue.pop();
    }
    if (logKeyQueue.empty())
        logKeyQueue = ConfigHelper::getLogKeys();

    for (auto i = 0ul; !logKeyQueue.empty(); ++i)
    {
        addButton(logKeyQueue.front());
        logKeyQueue.pop();
    }

    if (mButtons.empty())
    {
        {
            LogKey key("Z", "Z", new sf::Keyboard::Key(sf::Keyboard::Z), nullptr);
            addButton(key);
        }
        {
            LogKey key("X", "X", new sf::Keyboard::Key(sf::Keyboard::X), nullptr);
            addButton(key);
        }
    }
}

bool Application::isPressPerformedOnButton(unsigned &btnIdx) const
{
    const auto size = Button::size();
    for (auto i = 0u; i < size; ++i)
    {
        if (isMouseInRange(i))
        {
            btnIdx = i;
            return true;
        }
    }
    return false;
}

bool Application::isMouseInRange(unsigned idx) const
{
    const auto mousePosition = static_cast<sf::Vector2f>(sf::Mouse::getPosition(mWindow));
    const auto textureSize = static_cast<sf::Vector2f>(Settings::GfxButtonTextureSize);
    const auto &button = *mButtons[idx];
    const auto buttonPosition = button.getPosition() - textureSize / 2.f;
    const auto buttonRectangle = sf::FloatRect(buttonPosition, textureSize);

    return buttonRectangle.contains(mousePosition);
}

void Application::addButton(LogKey &logKey)
{
    const auto idx = static_cast<unsigned>(mButtons.size());
    mButtons.emplace_back(std::make_unique<Button>(idx, logKey, mTextures, mFonts));
    Button::setCount(static_cast<unsigned>(mButtons.size()));
}

void Application::removeButton()
{
    if (mButtons.empty())
        return;

    const auto idx = static_cast<unsigned>(mButtons.size() - 1);
    Settings::KeysTotal[idx] = 0;
    mButtons.pop_back();
    Button::setCount(static_cast<unsigned>(mButtons.size()));
}

void Application::openWindow()
{
    sf::Uint32 style;
#ifdef _WIN32
    style = Settings::WindowTitleBar
        ? (Settings::WindowResizable ? sf::Style::Close | sf::Style::Resize : sf::Style::Close)
        : (Settings::WindowResizable ? sf::Style::Resize : sf::Style::None);
#elif __linux__
    style = Settings::WindowTitleBar
        ? (Settings::WindowResizable ? sf::Style::Default : sf::Style::Close)
        : (Settings::WindowResizable ? sf::Style::Resize : sf::Style::None);
#else
#error Unsupported compiler
#endif

    if (mWindow.isOpen())
        mWindow.close();
    sf::ContextSettings settings;
    settings.antialiasingLevel = 8;
    mWindow.create(sf::VideoMode(getWindowWidth(), getWindowHeight()), "JKPS", style, settings);
    mWindow.setKeyRepeatEnabled(false);
    mWindow.setFramerateLimit(getApplicationUpdateFrequency());
#ifdef __linux__
    if (!Settings::WindowTitleBar)
    {
        auto desktop = sf::VideoMode::getDesktopMode();
        auto windowSize = mWindow.getSize();
        mWindow.setPosition(sf::Vector2i(
            static_cast<int>(desktop.width) / 2 - static_cast<int>(windowSize.x) / 2,
            static_cast<int>(desktop.height) / 2 - static_cast<int>(windowSize.y) / 2));
    }
#endif
}

void Application::resizeWindow()
{
    const auto size = sf::Vector2u(getWindowWidth(), getWindowHeight());
    mWindow.setSize(size);

    auto windowSize = static_cast<sf::Vector2f>(mWindow.getSize());
    auto view = sf::View(sf::FloatRect(0, 0, windowSize.x, windowSize.y));
    mWindow.setView(view);
}

void Application::moveWindow()
{
    // Absolute, offset-based dragging: while a drag is active, keep the window
    // pinned under the exact point the user grabbed. This avoids the drift and
    // lurching of delta-accumulation across variable event cadence.
    if (mDraggingWindow && sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        if (mWindow.hasFocus())
        {
            const auto desktop = sf::VideoMode::getDesktopMode();
            const auto windowSize = mWindow.getSize();
            auto position = sf::Mouse::getPosition() - mDragGrabOffset;
            const auto maxX = std::max(0, static_cast<int>(desktop.width) - static_cast<int>(windowSize.x));
            const auto maxY = std::max(0, static_cast<int>(desktop.height) - static_cast<int>(windowSize.y));
            position.x = std::clamp(position.x, 0, maxX);
            position.y = std::clamp(position.y, 0, maxY);
            mWindow.setPosition(position);
        }
    }
    else
    {
        mDraggingWindow = false;
    }
}

void Application::openMenuMode()
{
    if (mStyleWizard->isWindowOpen())
        mStyleWizard->closeWindow();

    if (!mMenu.isOpen())
        mMenu.openWindow();

    mUiMode = UiMode::MenuEditing;
}

void Application::openStyleMode()
{
    if (mMenu.isOpen())
        mMenu.closeWindow();

    if (!mStyleWizard->isWindowOpen())
        mStyleWizard->openWindow(sf::Vector2i(mWindow.getPosition().x + 100, mWindow.getPosition().y + 100));

    mUiMode = UiMode::StyleEditing;
}

void Application::returnToOverlayMode()
{
    if (mMenu.isOpen())
        mMenu.closeWindow();
    if (mStyleWizard->isWindowOpen())
        mStyleWizard->closeWindow();

    mUiMode = UiMode::OverlayOnly;
}

bool Application::isSecondaryUiActive() const
{
    return mMenu.isOpen()
        || mStyleWizard->isWindowOpen()
        || mGfxButtonSelector->isOpen();
}

unsigned Application::getWindowWidth()
{
    const unsigned btnAmt = Button::size();
    const float totalWidth =
        static_cast<float>(Settings::GfxButtonTextureSize.x * btnAmt)
        + static_cast<float>(static_cast<int>(btnAmt) - 1) * Settings::GfxButtonDistance
        + static_cast<float>(Settings::WindowBonusSizeLeft + Settings::WindowBonusSizeRight);

    // Clamp in the float domain: a negative total (possible with negative
    // button distance / texture size) would wrap when cast to unsigned.
    return static_cast<unsigned>(std::max(5.f, totalWidth));
}

unsigned Application::getWindowHeight()
{
    const float totalHeight =
        static_cast<float>(Settings::GfxButtonTextureSize.y)
        + static_cast<float>(Settings::WindowBonusSizeTop + Settings::WindowBonusSizeBottom);

    return static_cast<unsigned>(std::max(5.f, totalHeight));
}

sf::IntRect Application::getWindowRect()
{
    return { { }, sf::Vector2i(static_cast<int>(getWindowWidth()), static_cast<int>(getWindowHeight())) };
}

bool Application::parameterIdMatches(LogicalParameter::ID id)
{
    return
        id == LogicalParameter::ID::BtnGfxTxtrSz ||
        id == LogicalParameter::ID::BtnTextChSz ||
        id == LogicalParameter::ID::BtnGfxDist  ||
        id == LogicalParameter::ID::MainWndwTop ||
        id == LogicalParameter::ID::MainWndwBot ||
        id == LogicalParameter::ID::MainWndwLft ||
        id == LogicalParameter::ID::MainWndwRght ||
        id == LogicalParameter::ID::MainWndwResizable;
}

unsigned Application::getRenderUpdateFrequency() const
{
	return Settings::RenderUpdateFrequency;
}

unsigned Application::getApplicationUpdateFrequency() const
{
	return std::max(Settings::RenderUpdateFrequency, HooksUpdateFrequency);
}
