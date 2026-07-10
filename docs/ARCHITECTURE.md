# JKPS — Architecture Reference

> **Scope:** This document is the definitive technical reference for the JKPS (Keys Per Second) overlay application — a C++17 / SFML desktop program that renders a transparent, frameless overlay showing live keyboard/mouse activity, per-key and aggregate KPS/BPM statistics, and a fully editable visual theme.
>
> **Audience:** New contributors, architects, and maintainers. Reading paths are suggested at the end.
>
> **Last aligned with:** source tree at `/home/myrqyry/MQR/JKPS` (main), 14.3K LOC, ~55 translation units.
> **Language standard:** C++20 (`CMAKE_CXX_STANDARD 20`, extensions off).

---

## Table of Contents

1. [Executive Summary](#1-executive-summary)
2. [Reading Paths](#2-reading-paths)
3. [System Overview & Boundaries](#3-system-overview--boundaries)
4. [Runtime Model: The Fixed-Frequency Loop](#4-runtime-model-the-fixed-frequency-loop)
5. [Core Components](#5-core-components)
6. [Data Model](#6-data-model)
7. [The Configuration Pipeline](#7-the-configuration-pipeline)
8. [Rendering & Layout](#8-rendering--layout)
9. [Input & Interaction](#9-input--interaction)
10. [Design Decisions & Rationale](#10-design-decisions--rationale)
11. [Changes Applied in This Session](#11-changes-applied-in-this-session)
12. [Troubleshooting & Pitfalls](#12-troubleshooting--pitfalls)
13. [Appendix A: Settings Vocabulary](#appendix-a-settings-vocabulary)
14. [Appendix B: Glossary](#appendix-b-glossary)

---

## 1. Executive Summary

JKPS is a **single-window SFML application** with a small constellation of auxiliary windows (a settings *Menu*, a guided *Style Wizard*, a separate *KPS window*, and a per-button *visual-key selector*). Its job is to sit on top of other applications and visualize input throughput in real time.

The program is organized around a single owning object — `Application` (`Headers/Application.hpp`) — that owns the `sf::RenderWindow` and every subsystem. Subsystems are kept deliberately decoupled: they hold no back-pointer to `Application` and communicate upward only through two well-defined channels:

- **The changed-parameters queue** (`ChangedParametersQueue`) — when a user edits a value in the Menu, the change is queued; `Application::unloadChangesQueue()` drains it once per config frame and pushes the change to every subsystem that listens for that parameter ID.
- **`LogicalParameter::parameterIdMatches(ID)` static gates** — each visual subsystem (buttons, statistics lines, KPS window, the application window itself) declares which `LogicalParameter::ID` values it cares about, so a single queued change fans out to exactly the right consumers without the emitter knowing who they are.

The rendering loop is **fixed-frequency and decoupled from frame rate**: it accumulates wall-clock time and, on every *Event* tick and every *Hooks* tick, drives the relevant subsystems. This keeps key sampling deterministic (60 Hz hooks) independent of the configured render rate.

**Key facts at a glance**

| Property | Value |
|---|---|
| Language / standard | C++17 |
| Rendering / windowing | SFML 2.x (`sfml-graphics`, `sfml-window`) |
| Entry point | `Source/Main.cpp` → `Application app; app.run();` |
| Primary owner | `Application` (owns window + all subsystems) |
| Build | CMake (`CMakeLists.txt`), output `build/JKPS` |
| Loops | Event loop (render frequency) + Hooks loop (60 Hz, key sampling) |
| Persistence | Plain-text `JKPS.cfg` + `JKPSErrorLog.txt` on error |
| Translation units | ~55 (40 headers + sources, plus embedded default media) |

---

## 2. Reading Paths

- **I'm a new contributor** → §3 (overview) → §4 (loop) → §5.1 (`Application`) → §7 (config pipeline). Then pick one subsystem in §5 and trace it.
- **I'm debugging input/visuals** → §4 (loop) → §9 (input) → §5.2 (Button/LogButton/GfxButton) → §8 (rendering/layout).
- **I'm changing config/settings** → §7 (pipeline) → §6.2 (`LogicalParameter`) → Appendix A (vocabulary) → §11 (this session's refactors).
- **I'm an architect / reviewing** → §10 (decisions) → §11 (recent changes) → §12 (pitfalls).

---

## 3. System Overview & Boundaries

### 3.1 External boundaries

```
                 +------------------------------ SFML / OS ------------------------------+
                 |  Windowing (RenderWindow)   Input events   Font/Texture loading     |
                 +-----------------------------------+-----------------------------------+
                                                 |
   +---------------------------------------------+-------------------------------+
   |                          APPLICATION (owner)                                 |
   |                                                                              |
   |  mWindow (sf::RenderWindow)                                                  |
   |     │ draws                                                                  |
   |     ├── Background              (gradient / texture backdrop)               |
   |     ├── mButtons[]  (Button = LogButton + GfxButton)   ← key/mouse trackers |
   |     ├── mStatistics[] (GfxStatisticsLine x N)           ← KPS/Total/BPM text |
   |     ├── Menu                  (settings editor window)                     |
   |     ├── GfxButtonSelector     (per-key rebind popover)                      |
   |     ├── KPSWindow             (separate KPS readout window)                 |
   |     ├── KeysPerSecondGraph    (disabled; see §5.8)                          |
   |     └── StyleWizard           (guided theme editor)                         |
   |                                                                              |
   |  cross-cutting: Settings (globals) · LogicalParameter (typed config value)  |
   |                ChangedParametersQueue · ResourceHolder (textures/fonts)     |
   +---------------------------------------------+-------------------------------+
                                                 |
                 +------------------------------ Config file --------------------------+
                 |  JKPS.cfg (read on start, written by Menu/StyleWizard)              |
                 +--------------------------------------------------------------------+
```

The program talks to the outside world only through:
1. **The OS windowing/input layer** (SFML `RenderWindow`, `Event`, `Mouse`, `Keyboard`).
2. **The filesystem** — `JKPS.cfg` (config), and font/texture asset files (with embedded fallbacks compiled into the binary under `Headers/Default media/`).
3. **The user** — via keyboard, mouse, and the auxiliary editing windows.

There is **no networking, no threads for main work** (key sampling is `sf::Clock`-driven on the main thread), and no external process communication.

### 3.2 Component inventory

| Subsystem | Header | Role |
|---|---|---|
| `Application` | `Application.hpp` | Owner/orchestrator; owns window + all subsystems; runs the loop. |
| `Button` (→ `LogButton` + `GfxButton`) | `Button.hpp` | One tracked key/mouse button: input sampling + visual. |
| `LogButton` | `LogButton.hpp` | Input logic: press state, KPS, BPM, total counters (static aggregates). |
| `GfxButton` | `GfxButton.hpp` | Visual: sprite layers, text, press animations, gradients, particles. |
| `GfxStatisticsLine` | `GfxStatisticsLine.hpp` | A single statistics row (KPS / Total / BPM). |
| `Background` | `Background.hpp` | Window backdrop (texture or gradient), rescales to window. |
| `Menu` | `Menu.hpp` | The settings editor window + config (de)serialization. |
| `ParameterLine` / `Palette` | `ParameterLine.hpp` | One editable config row in the Menu; container of rows. |
| `ChangedParametersQueue` | `ChangedParametersQueue.hpp` | Fan-out buffer for config edits. |
| `LogicalParameter` | `LogicalParameter.hpp` | A single typed, bound config value (the config atom). |
| `ConfigHelper` | `ConfigHelper.hpp` | Parses/serializes `JKPS.cfg`; builds the initial `LogKey` set. |
| `StyleWizard` | `StyleWizard.hpp` | Guided, step-based theme editor. |
| `KPSWindow` | `KPSWindow.hpp` | Standalone KPS readout window. |
| `KeysPerSecondGraph` | `KeysPerSecondGraph.hpp` | Graph window — **currently disabled** (§5.8). |
| `GfxButtonSelector` | `GfxButtonSelector.hpp` | Popover to rebind a button's key/mouse mapping. |
| `ButtonPositioner` / `StatisticLinesPositioner` | `ButtonPositioner.hpp` | Pure layout functors (recompute element positions). |
| `ResourceHolder` / `ResourceIdentifiers` | `ResourceHolder.hpp` | Typed texture/font caches keyed by enum. |
| `Settings` | `Settings.hpp` | Flat global namespace of all config field values + `UiTokens`. |

---

## 4. Runtime Model: The Fixed-Frequency Loop

The heartbeat of JKPS is `Application::run()` (`Source/Application.cpp:52`). It is **not** a naive "poll once per `display()`" loop — it is a **time-accumulator loop** that emits discrete *Event* and *Hooks* ticks.

```cpp
void Application::run()
{
    sf::Clock clock;
    auto timeSinceLastEventUpdate  = sf::Time::Zero;
    auto timeSinceLastHooksUpdate  = sf::Time::Zero;
    const sf::Time MaxFrameTime = sf::seconds(0.1f);   // spiral-of-death guard

    while (mWindow.isOpen())
    {
        auto dt = clock.restart();
        if (dt > MaxFrameTime) dt = MaxFrameTime;       // clamp long stalls
        timeSinceLastEventUpdate  += dt;
        timeSinceLastHooksUpdate  += dt;

        const sf::Time TimePerEventUpdate =
            sf::seconds(1.f / static_cast<float>(getRenderUpdateFrequency()));

        while (true)
        {
            int updateType = UpdateType::None;
            if (timeSinceLastHooksUpdate > TimePerHookUpdate) {     // 1/60 s
                timeSinceLastHooksUpdate -= TimePerHookUpdate;
                updateType |= UpdateType::Hooks;
            }
            if (timeSinceLastEventUpdate > TimePerEventUpdate) {    // render rate
                timeSinceLastEventUpdate -= TimePerEventUpdate;
                updateType |= UpdateType::Event;
            }
            if (updateType == UpdateType::None) break;

            processInput(static_cast<UpdateType>(updateType));
            update(TimePerEventUpdate.asSeconds(), static_cast<UpdateType>(updateType));
        }
        render();
    }
}
```

### 4.1 `UpdateType` flags

```cpp
enum UpdateType { None = 0, Event = 1 << 0, Hooks = 1 << 1 };
```

- **`Event`** tick → runs at `getRenderUpdateFrequency()` (default 60 Hz). Drives UI/interaction work: `handleEvent()`, window drag, Menu/StyleWizard updates, draining the changed-parameters queue, and `Button::update` / `GfxStatisticsLine::update`.
- **`Hooks`** tick → fixed 60 Hz (`HooksUpdateFrequency = 60u`, `Application.cpp:12`). Drives *input sampling only*: `Button::moveIndex()` advances the circular buffer, and `LogButton::accumulateBeatsPerMinute()` samples BPM. Keeping this fixed and independent of render rate makes KPS/BPM math stable regardless of display refresh.

### 4.2 Why this design

Two decoupled rates mean:
- A slow display (e.g., 30 Hz) still gets accurate 60 Hz key sampling.
- Changing `RenderUpdateFrequency` in settings rescales only the UI tick, never the counters' timing base.
- The `MaxFrameTime` clamp prevents a "spiral of death": after a long stall (sleep/resume, breakpoint pause) the accumulator cannot queue hundreds of catch-up ticks that would starve `render()`.

### 4.3 Per-tick dispatch

`Application::processInput()` and `Application::update()` both switch on the flags:

- **Event** → `handleEvent()` (hotkeys, mouse, window close), StyleWizard reset/reload checks, `unloadChangesQueue()`, asset-refresh checks, and `Button`/`GfxStatisticsLine` `update()`.
- **Hooks** → `Button::moveIndex()` + `LogButton::accumulateBeatsPerMinute()` for each button.

---

## 5. Core Components

### 5.1 `Application` — the owner

`Application` (`Headers/Application.hpp:29`) is the composition root. It owns the `sf::RenderWindow` and every subsystem via `std::unique_ptr` / `std::vector<std::unique_ptr<Button>>`. Constructing it wires everything together (`Application.cpp:15`):

1. `loadTextures()` / `loadFonts()` — populate the `ResourceHolder` caches (with embedded fallbacks if files are missing).
2. `buildButtons()` / `buildStatistics()` — create tracked keys from config + the KPS/Total/BPM rows.
3. `openWindow()` — create the `RenderWindow` (style `None` when `Settings::WindowTitleBar == false`, i.e., frameless overlay).
4. Construct the auxiliary windows (`GfxButtonSelector`, `ButtonPositioner`, `StatisticsPositioner`, `Background`, `KPSWindow`, `KeysPerSecondGraph`, `StyleWizard`).
5. `mMenu.saveConfig(mButtons)` — write back an initial config.

**UI mode policy.** The Menu and StyleWizard are mutually exclusive editing surfaces. `Application` tracks this with a private `enum class UiMode { OverlayOnly, MenuEditing, StyleEditing }` (`Application.hpp:53`) and three helpers:

- `openMenuMode()` — closes StyleWizard if open, opens Menu.
- `openStyleMode()` — closes Menu if open, opens StyleWizard.
- `returnToOverlayMode()` — closes both.

`isSecondaryUiActive()` (`Application.cpp:620`) derives the "a panel is open" fact directly from live window state (`mMenu.isOpen() || mStyleWizard->isWindowOpen() || mGfxButtonSelector->isOpen()`) — robust even if a panel self-closes. This gate is what suppresses frameless window-dragging while an editing surface is up (§9.2).

**Static geometry.** `getWindowWidth()` / `getWindowHeight()` (`Application.cpp:646`) are *static* so the window-creation code (which runs before `Application` is fully constructed) can size the window. The formula is:

```
Width  = TextureSize.x * Amt + (Amt - 1) * ButtonDistance + BonusLeft + BonusRight
Height = TextureSize.y        + BonusTop + BonusBottom
```

> The historical config doc lists `Height = TextureSize.y * Amt + ...`, but that is a documentation typo — buttons are a single horizontal row, so height is one row tall plus top/bottom bonus. The code is correct; see §12.

### 5.2 `Button` = `LogButton` + `GfxButton`

`Button` (`Headers/Button.hpp`) uses **multiple inheritance** to fuse tracking logic with rendering:

```cpp
class Button : public LogButton, public GfxButton
```

- **`LogButton`** (`Headers/LogButton.hpp`) — the *brain*. Holds a `LogKey`, tracks press state, and exposes **static aggregate counters**: `getKeysPerSecond()`, `getMaxKeysPerSecond()`, `getTotal()`, `getBeatsPerMinute()`. These are process-wide because the statistics text shows *combined* throughput. Per-instance it also tracks `getLocalBeatsPerMinute()`.
- **`GfxButton`** (`Headers/GfxButton.hpp`) — the *body*. Manages five sprite layers (`SpriteID`: `ButtonSprite`, `AnimationSprite`, `PressEffectSprite`, `ShadowSprite`, `KeyVisualizerSprite`) and up to four text layers (`TextID`: `VisualKey`, `KeyCounter`, `KeyPerSecond`, `BeatsPerMinute`). It implements the rich theming surface: press animations, gradients (`applyEnhancedGradientToSprite`, `applyThemeBasedGradient`), responsive scaling (`applyResponsiveScaling`), and a particle emitter (`RectEmitter`).

`LogButton` carries a **static button counter** (`mSize`) with an explicit `Button::setCount(unsigned)`; `Application::addButton` / `removeButton` manage it. This counter is consumed by external layout code (`StatisticLinesPositioner`, `Application::getWindowWidth`), so it must stay accurate — the destructor no longer mutates it (see §11).

### 5.3 `GfxStatisticsLine`

`GfxStatisticsLine` (`Headers/GfxStatisticsLine.hpp`) renders one statistics row. Its `StatisticsID` enum `{ KPS, Total, BPM, StatisticsIdCounter }` doubles as a count, so `Application` holds `std::array<..., GfxStatisticsLine::StatisticsIdCounter> mStatistics`. Each line binds to a `bool &show` flag and updates its text from the static counters in `LogButton`.

### 5.4 `Background`

`Background` (`Headers/Background.hpp`) draws the window backdrop. `rescale()` is called on every window-size change and config reload so the background matches the current dimensions; the texture can optionally be scaled to match (`ScaleBackgroundTextureIfItDoesNotFit`).

### 5.5 `Menu` — settings editor + config owner

`Menu` (`Headers/Menu.hpp`) is the largest subsystem. It is both:
- **The editor UI** — a tabbed window of `ParameterLine`s grouped into `KeyBlock`s; supports value editing, RGB color picking, collection lists, and advanced per-key-press-visualization tabs (`enum AdvancedKeys { StatText, BtnTextSepVal, BtnText, GfxBtn, KeyPressVis }`).
- **The config (de)serializer** — `saveConfig(mButtons)` writes `JKPS.cfg`; `reloadConfig()` re-reads it. `ConfigHelper` does the actual file I/O (§7).

When a value changes in the Menu, it is pushed to the `ChangedParametersQueue`; `Application` drains that queue on the next Event tick (§7.3).

### 5.6 `ParameterLine` / `Palette`

`ParameterLine` (`Headers/ParameterLine.hpp`) is one editable row: a label, a `LogicalParameter`, and interaction handlers (`handleEvent`, `handleValueModEvent`, `handleButtonsInteractionEvent`, `tabulation`). `Palette` (`Headers/Palette.hpp`) is a container of `ParameterLine`s with selection/cursor management. Both are `sf::Drawable` + `sf::Transformable`.

### 5.7 `StyleWizard`

`StyleWizard` (`Headers/StyleWizard.hpp`) is a **guided, step-based** alternative to the raw Menu: `enum Step { Welcome, PresetSelection, ColorCustomization, AnimationSettings, WindowSettings, ReviewApply }`. It applies presets (`applyPreset`) and writes config via `saveConfig()`. `resetApplyRequest()` lets it signal `Application` to reload+reset assets.

### 5.8 `KeysPerSecondGraph` — disabled

`KeysPerSecondGraph` (`Headers/KeysPerSecondGraph.hpp`) exists and builds, but is **intentionally disabled** in this build: its `update()` and `render()` calls are commented out in `Application::update()` and `render()` (`Application.cpp:295-296`, `326-327`). The render call is suppressed specifically to avoid drawing stale data if the window is ever opened. Its `parameterIdMatches` and other plumbing remain intact.

### 5.9 `KPSWindow`

`KPSWindow` (`Headers/KPSWindow.hpp`) is a **separate** `sf::RenderWindow` (constructed internally) showing a dedicated KPS readout. It has its own `handleOwnEvent()`, `update()`, `render()`, and even its own `moveWindow()` for frameless dragging. It is toggled with `Settings::KeyToOpenKPSWindow`.

### 5.10 `GfxButtonSelector`

`GfxButtonSelector` (`Headers/GfxButtonSelector.hpp`) is a small popover opened by right-clicking a button (see §9.1). It rebinds a `LogKey` to a new key or mouse button (`setKey`, `saveKey`). Its `KeyType { Keyboard, Mouse }` and `Buttons { RealKeyButton, VisualKeyButton, AcceptButton, ButtonsCount }` enumerate its internal widget layout.

---

## 6. Data Model

### 6.1 `LogKey` — the tracked input

```cpp
struct LogKey {
    std::string realStr;        // e.g. "A"
    std::string visualStr;      // e.g. "A" or a display alias
    sf::Keyboard::Key *keyboardKey;   // non-owning pointer; exactly one of these is set
    sf::Mouse::Button   *mouseButton; // (the other is nullptr)
    bool changed;
};
```

A `LogKey` is either a keyboard key **or** a mouse button (mutually exclusive pointers). The initial set is produced by `ConfigHelper::getLogKeys()`, which parses the `Buttons` and `Keys` collections from `JKPS.cfg`.

### 6.2 `LogicalParameter` — the typed config atom

`LogicalParameter` (`Headers/LogicalParameter.hpp`) is the **fundamental unit of configuration**. Each holds:
- a `Type` (`enum Type`: `Empty, Collection, Hint, Unsigned, Int, Bool, Float, String, StringPath, Color, VectorU, VectorI, VectorF`),
- a `ValuePtr` (a `std::variant` of pointers to the target `Settings` field — see §11.2),
- a default value and change state.

It exposes typed accessors (`getDigit`, `getBool`, `getColor`, `getVector`, `getValStr`, `resetToDefaultValue`) and an inverse helper `getInverseBool`. The constructor takes a `ValuePtr` plus string/default/limit metadata. Its static `parameterIdMatches(ID)` lets subsystems declare interest in specific `LogicalParameter::ID`s.

Crucially, `LogicalParameter` **binds directly to a live `Settings` global** via the variant pointer. Writing the parameter writes the global; reading it reflects the global. This is the mechanism that makes config edits immediately visible to rendering code.

### 6.3 Settings — the flat global namespace

`Settings` (`Headers/Settings.hpp`) is a single namespace of `extern` globals (≈200 fields) — everything from `StatisticsTextColor` to `GfxButtonTextureSize` to hotkeys. `LogicalParameter`s bind to these. A nested `UiTokens` namespace (`BaseSpacing`, `PrimaryTextSize`, `SecondaryTextSize`, `SurfaceColor`, `AccentColor`, `BorderColor`) holds the new design-token foundation (§10.4).

---

## 7. The Configuration Pipeline

This is the system's most important cross-cutting flow.

### 7.1 Startup (load)

```
ConfigHelper::readConfig(parameters, collectionNames)   [Menu owns the parameters]
   └─ reads JKPS.cfg line-by-line
   └─ for each known parameter: readParameter(LogicalParameter&, collection)
        └─ scanParameterValue(...) parses "Key: value"
        └─ writes into the bound Settings global via the LogicalParameter
   └─ ConfigHelper::getLogKeys() builds the initial LogKey queue
Application::buildButtons() drains that queue → mButtons[]
```

If parsing fails, the default value is used and an error is appended to `JKPSErrorLog.txt`. The user-facing rule (from the project's config doc) is: **one space after the colon; comma-separated numbers/keys with no spaces; paths without quotes and with extension.** Any mistake → default + log entry.

### 7.2 Editing (queue + fan-out)

```
User edits value in Menu → ParameterLine → LogicalParameter
   └─ ChangedParametersQueue::push({ID, shared_ptr<LogicalParameter>})
        (Menu::getChangedParametersQueue())

... later, on an Event tick ...
Application::unloadChangesQueue()                         [Application.cpp:334]
   └─ while queue not empty:
        pair = queue.pop()
        if GfxStatisticsLine::parameterIdMatches(pair.first)  → mStatistics[].updateParameters() + reposition
        if Button::parameterIdMatches(pair.first)            → mButtons[].updateParameters() + reposition
        if KPSWindow::parameterIdMatches(pair.first)         → mKPSWindow->updateParameters()
        if Application::parameterIdMatches(pair.first)        → resize window + refocus Menu
        if ID in {MainWndwTitleBar, MainWndwResizable}        → openWindow() (recreate)
        if ID == RenderUpdateFrequency                        → setFramerateLimit()
        mBackground->rescale()   (always, after any change)
```

This **fan-out via static `parameterIdMatches` gates** is the key decoupling: the Menu never holds pointers to every consumer; it just enqueues `(ID, param)`. Each consumer statically declares which IDs it reacts to. Adding a new consumer that responds to an existing setting is a one-line `parameterIdMatches` change.

### 7.3 Asset reload

Some edits (fonts, textures, color sets) require reloading GPU assets. The flow uses a refresh-flag handshake:
- `ParameterLine::resetRefreshState()` / `resetRefreshState()` signals "assets changed."
- `StyleWizard::resetApplyRequest()` signals "apply preset → full reload."
- `Application::resetAssets()` (`Application.cpp:385`) clears and reloads `mFonts`/`mTextures`, then calls `updateAssets()` on every button/line/background and re-runs both positioners, and finally `mMenu.saveConfig(mButtons)`.

---

## 8. Rendering & Layout

### 8.1 Draw order (`Application::render`, `Application.cpp:307`)

```
mWindow.clear();
draw Background;
draw each Button;
draw each GfxStatisticsLine;
if Menu.isOpen()              draw Menu;
if GfxButtonSelector.isOpen() draw Selector;
if KPSWindow.isOpen()         draw KPSWindow;        // separate window, its own draw
if StyleWizard.isWindowOpen() draw StyleWizard;
mWindow.display();
```

The Menu, Selector, and StyleWizard are **separate `sf::RenderWindow`s** drawn on top; the KPSWindow is its own window entirely.

### 8.2 Layout functors

Positioning is delegated to two stateless functors:
- `ButtonPositioner` (`ButtonPositioner.hpp`) — `operator()` walks `mButtons` and sets each button's position based on `GfxButton::getWidth/getHeight`, `Settings::GfxButtonDistance`, and `WindowBonusSize*`.
- `StatisticLinesPositioner` (`StatisticLinesPositioner.hpp`) — positions the statistics rows using `Settings::StatisticsTextDistance` and per-line offsets.

Both are re-invoked whenever the relevant parameters change or assets reload. They are intentionally pure (no side effects beyond positioning), which makes the layout reproducible after a config edit.

### 8.3 Dirty-flag caching (performance)

`Button` caches its text layout to avoid recomputing geometry every frame: `mTextLayoutDirty` is set only when the displayed string or character size actually changes; `controlBounds()` skips the redundant `keepInBounds`/`centerOrigins` pass unless dirty. This keeps the 60 Hz render cheap even with many buttons (see §11.3).

---

## 9. Input & Interaction

### 9.1 Hotkeys (in `Application::handleEvent`)

| Key (default) | Action |
|---|---|
| `Ctrl`+`A` / `Ctrl`+Alt+`A` | Add a keyboard key button |
| `Ctrl`+`,` | Add a mouse-button button |
| `Ctrl`+`Z`/`X`/`Alt+Z/X`/`Ctrl+.` | Remove a button |
| `Ctrl`+`K` | Toggle KPS window |
| `Ctrl`+`A`-style menu key (`KeyToOpenMenuWindow`) | Toggle Menu (mutually exclusive with StyleWizard) |
| `Ctrl`+`S` (`KeyToOpenStyleWizard`) | Toggle StyleWizard |
| `Ctrl`+`X` (`KeyToReset`) | Reset all button counters |
| `Ctrl`+`W` (`KeyExit`) | Save config + close |

The toggles were refactored to route through `openMenuMode()` / `openStyleMode()` / `returnToOverlayMode()` so the two editors can never be open at once (§5.1).

Right-click a button → opens `GfxButtonSelector` to rebind that key/mouse mapping.

### 9.2 Frameless window dragging

When `Settings::WindowTitleBar == false` (frameless overlay), the window is moved by dragging. This was reworked to be **offset-based and event-driven** (was delta-accumulation that drifted):

- On **left `MouseButtonPressed`** (only if frameless, no secondary UI open, window focused): set `mDraggingWindow = true` and record `mDragGrabOffset = mousePosition − windowPosition`.
- While dragging (left held): `mWindow.setPosition(mousePosition − mDragGrabOffset)` — **absolute**, so no drift.
- On **`MouseButtonReleased`** or when the button isn't held: stop dragging.

Dragging is **suppressed while any secondary UI is active** (`!isSecondaryUiActive()`), so opening the Menu/StyleWizard/Selector doesn't yank the overlay around (§5.1).

---

## 10. Design Decisions & Rationale

### 10.1 `LogicalParameter` binds directly to `Settings` globals
*Why:* A single source of truth per field. Editing a parameter edits the live global; rendering reads the same global. No copy/sync step.
*Trade-off:* The variant-of-pointers couples `LogicalParameter` to the global namespace layout, but it keeps the config system tiny and fast. We hardened this in §11.2 with `std::variant` + `std::monostate`.

### 10.2 Static `parameterIdMatches` fan-out
*Why:* Keeps the Menu (the only editor) ignorant of every consumer. New consumers opt in by declaring interest.
*Trade-off:* The set of IDs a subsystem cares about is duplicated as a `switch`/comparison rather than data — acceptable for ~15 consumers.

### 10.3 Dual-rate fixed loop
*Why:* Deterministic 60 Hz key sampling independent of render rate; stable KPS/BPM math; spiral-of-death guard via `MaxFrameTime`.
*Trade-off:* Slightly more complex than a naive loop, but the complexity pays for itself in measurement stability.

### 10.4 Design tokens (`Settings::UiTokens`) — bridge, not replace
*Why:* The review called the `Settings` namespace a "god object." Rather than a risky mass extraction, we introduced a `UiTokens` namespace (spacing, text sizes, surface/accent/border colors) as the canonical vocabulary new layout code should prefer, while keeping the ~200 fine-grained `Settings` fields for backward compatibility and per-element overrides.
*Trade-off:* Two vocabulary layers coexist; this is intentional incremental migration, not a rewrite. The tokens are now consumed by the re-enabled `KeysPerSecondGraph` (surface background + accent line color), demonstrating the bridge pattern without altering existing button/statistics visuals.

### 10.5 Multiple inheritance for `Button`
*Why:* `LogButton` (logic) and `GfxButton` (visual) are independently useful concepts; `Button` fuses them so each tracked key is one object with both behaviors and a single position.
*Trade-off:* MI adds a (small) vtable and demands careful base-class construction order — handled via explicit `idx`/`key` constructor args (§11.1).

### 10.6 Embedded default media
Fonts and textures are compiled into the binary (`Headers/Default media/...`) so the program runs even with no asset files on disk; `loadTextures()`/`loadFonts()` fall back to `loadFromMemory` when the file path is missing.

---

## 11. Changes Applied in This Session

This section records the refactors/fixes made while producing this document's baseline. All compile clean (`[100%] Built target JKPS`) and pass a 4–5 s runtime smoke test.

### 11.1 Button static-counter refactor (`Button`, `Application`)
- `Button` constructor now takes an explicit `unsigned idx` (forwarded to `LogButton`/`GfxButton`) plus `LogKey&`.
- Added `static void Button::setCount(unsigned)`; `Application::addButton`/`removeButton` pass the explicit index and call `setCount`. `removeButton` explicitly zeroes `Settings::KeysTotal[idx]`.
- `~Button()` no longer mutates `KeysTotal` / `mSize` — eliminating a side-effect-on-destruction hazard and keeping the external `Button::size()` count coherent for `StatisticLinesPositioner` and `getWindowWidth()`.

### 11.2 `LogicalParameter` `void*` → `std::variant` (type-safe config)
- Replaced the `union Val` + `void* mVal` with `using ValuePtr = std::variant<std::monostate, unsigned*, int*, bool*, float*, std::string*, sf::Color*, sf::Vector2u*, sf::Vector2i*, sf::Vector2f*>;`.
- Removed untyped `setDigit`/`setVal` accessors; all reads go through `std::get<T*>(mVal)`.
- `Source/Menu.cpp`: the 31 `nullptr` value-pointers used for `Empty`/`Collection`/`Hint` parameters became `std::monostate{}` (3 unrelated `nullptr`s left untouched).
- Float parameters now parse with `std::stof` (was `std::stoi`), fixing silent truncation of fractional values.
- `setBool` hardened: lowercases the input and compares to `"true"` instead of a fragile `str.size() == 4` check.

### 11.3 `Button` layout dirty-flag caching
Added `bool mTextLayoutDirty` + `unsigned mCachedCharSize`. `setTextStrings()` only flags dirty on a real string/char-size change; `controlBounds()` skips redundant `keepInBounds`/`centerOrigins` unless dirty. Visual behavior preserved; per-frame cost reduced.

### 11.4 UI mode + mutually exclusive panels + drag suppression (`Application`)
- Added `enum class UiMode { OverlayOnly, MenuEditing, StyleEditing }` and helpers `openMenuMode`/`openStyleMode`/`returnToOverlayMode`/`isSecondaryUiActive`.
- Menu/StyleWizard toggles now route through these helpers → the two editors can never be open simultaneously.
- `moveWindow()` (frameless drag) is gated behind `!isSecondaryUiActive()`, and rewritten as **offset-based** dragging (press records grab offset; position = mouse − offset; release stops). Eliminates the prior drift/lurch from delta-accumulation.

### 11.5 Window-size clamping (UB guard)
`getWindowWidth()`/`getWindowHeight()` now clamp in the **float domain** (`std::max(5.f, total)`) *before* casting to `unsigned`. Previously a negative total (possible with negative `GfxButtonDistance`/texture size, both allowed by config) wrapped to a huge value under `std::max(5u, ...)`, producing a giant window.

### 11.6 `UiTokens` design-token layer (`Settings`)
Added `namespace Settings::UiTokens` with `BaseSpacing`, `PrimaryTextSize`, `SecondaryTextSize`, `SurfaceColor`, `AccentColor`, `BorderColor` (externs + sensible defaults). Foundation for future cohesive theming; existing `Settings` fields retained.

### 11.7 Misc robustness
- `Application::run()` clamps `dt` to `MaxFrameTime` (0.1 s) — spiral-of-death guard.
- `CMakeLists.txt`: `CONFIGURE_DEPENDS` added to both `GLOB_RECURSE` source globs (new files are auto-detected on rebuild).
- Removed a duplicate `#include "../Headers/Menu.hpp"` in `Application.cpp`.
- `KeysPerSecondGraph` render call commented out (it was already not updating) to avoid stale rendering.

### 11.8 Roadmap improvements (post-architecture pass)

Implemented from the community refactor roadmap, with two items deliberately **deferred/rejected** (see below).

- **1.2 Robust config parsing (`std::from_chars`).** `ConfigHelper`'s `readDigitParameter` / `readVectorParameter` / `readColorParameter` no longer use `std::stof` + a manual `isNumber` check. A new `parseFloatFromView(std::string_view, size_t&)` helper scans a float via `std::from_chars` (exception-free, allocation-free) and returns NaN on no-digit. All three readers fall back to the default value **non-recursively** (the previous `handleError` re-invoked the reader, which could infinite-recurse and overflow the stack). `isNumber` was removed as dead code.
- **2.2 `RectEmitter` cache-friendly refresh.** Replaced the growing `mUsedRectIndices` vector (erased/rebuilt each frame) with a uniform `std::vector<bool> mActive` flag array plus swap-and-pop removal. Inactive particles are skipped; no per-frame allocation occurs while particles live and die — better CPU-cache behavior.
- **3.1 `KeysPerSecondGraph` re-enabled with a ring buffer.** Re-architected from a hardcoded triangle-fan mockup into a fixed-capacity `std::array<float, 256>` ring buffer sampled from `Button::getKeysPerSecond()` each tick and drawn as a scrolling `sf::LineStrip` (auto-scaled to the busiest sample). The open/close hotkey (`KeyToOpenGraphWindow`, default `G`) is re-enabled.
- **3.2 Incremental `UiTokens` migration.** The graph now renders its background via `Settings::UiTokens::SurfaceColor` and its line via `Settings::UiTokens::AccentColor` (identical to the prior hard-coded values, so zero visual change). Existing button/statistics visuals still read their per-element `Settings` overrides — the "bridge, not replace" approach avoids silently changing users' layouts.
- **4.2 C++20 + `std::string_view` parsing.** Bumped `CMAKE_CXX_STANDARD` to `20` (extensions off). Read-only `StringHelper` parsers (`readAmountOfParms`, `readValue`, `eraseDigitsOverHundredths`, `strToKey`, `strToBtn`, `isKey`, `isButton`) now take `std::string_view`; `Utility::retrieveNumber` already did. `ConfigHelper` parsing uses `std::string_view` throughout. Also cleared two latent `-Werror` issues (`[[maybe_unused]]` on `ResourceHolder::insertResource`'s `inserted`; initialized `key`/`button` in `readKeys`).

**Deferred / rejected roadmap items (with rationale):**
- **1.1 Hierarchical context structs** — *deferred.* Collapsing `Settings` into `ButtonTheme`/`StatisticsConfig`/`WindowLayout` would rewire the ~150 `LogicalParameter` bindings for no current payoff and high regression risk. `UiTokens` is the safer incremental bridge.
- **4.1 Explicit CMake source listing** — *rejected.* It would undo the `CONFIGURE_DEPENDS` auto-discovery we rely on; `CONFIGURE_DEPENDS` is the modern, correct choice for this project.

---

## 12. Troubleshooting & Pitfalls

| Symptom | Cause | Fix |
|---|---|---|
| Window is **huge** after editing config | Negative `GfxButtonDistance`/texture size wrapped in `unsigned` cast | Fixed in §11.5 (float-domain clamp). Verify `getWindowWidth()` clamps before cast. |
| Overlay **drifts / jumps while dragging** | Old delta-accumulation `moveWindow()` | Fixed in §11.4 (offset-based + event-driven). |
| Editing Menu **also moves the overlay** | Drag not suppressed during secondary UI | Fixed: `moveWindow()` gated by `!isSecondaryUiActive()`. |
| Config value **ignored / reset to default** | Typo in `JKPS.cfg` (no space after colon, spaces in comma list, quoted path) | Follow the format in §7.1; check `JKPSErrorLog.txt`. |
| Float setting **loses decimals** | Old `std::stoi` parse | Fixed in §11.2 (`std::stof`). |
| `setBool` flips wrong way | Old `str.size() == 4` heuristic | Fixed in §11.2 (lowercased `"true"` compare). |
| Graph window shows **nothing / stale** | `KeysPerSecondGraph` disabled | Expected (§5.8). Re-enable both `update()` and `render()` to use it. |
| New `Settings` field doesn't affect UI | Not bound to a `LogicalParameter`, or consumer doesn't `parameterIdMatches` it | Bind via `ValuePtr`, and add the ID to the relevant `parameterIdMatches`. |
| Build doesn't pick up a **new file** | Static GLOB without `CONFIGURE_DEPENDS` | Fixed in §11.7 (now set). Re-run CMake if needed. |

---

## Appendix A: Settings Vocabulary

Selected `Settings` fields (full list in `Headers/Settings.hpp`; format rules in the project config doc):

**Statistics text** — `StatisticsTextDistance`, `StatisticsTextPosition`, `StatisticsTextValuePosition`, `StatisticsTextColor`, `StatisticsTextCharacterSize`, `StatisticsTextOutlineThickness/Color`, `StatisticsTextBold/Italic`, `ShowStatisticsText/KPS/MaxKPS/Total/BPM`, advanced per-line overrides (`StatisticsTextAdv*`).

**Button text** — `ButtonTextFontPath`, `ButtonTextColor`, `ButtonTextCharacterSize`, `ButtonTextOutlineThickness/Color`, `ButtonTextPosition`, `ButtonTextBoundsToggle`, `ShowVisualKeys`, `ShowKeyCounters`, `ShowKeyKPS`, `ShowKeyBPM`.

**Button graphics** — `GfxButtonTexturePath`, `GfxButtonTextureSize` (drives window width via §5.1), `GfxButtonTextureColor`, `GfxButtonDistance` (drives spacing/width).

**Animation** — `LightAnimation`, `PressAnimation`, `AnimationTexturePath`, `AnimationFrames`, `AnimationScaleOnClick`, `AnimationColor`, `AnimationOffset`.

**Main window** — `BackgroundTexturePath`, `BackgroundColor`, `ScaleBackgroundTextureIfItDoesNotFit`, `WindowTitleBar` (false → frameless overlay + custom drag), `WindowResizable`, `WindowBonusSizeTop/Bottom/Left/Right`, `RenderUpdateFrequency`.

**Extra KPS window** — `EnableFromStart`, `WindowSize`, `TextSize`, `NumberSize`, `KPSBackgroundColor`, `KPSTextColor`, `KPSTopPadding`, `KPSExtraWindowDistanceBetweenText`.

**Hotkeys** — `KeyToIncreaseKeys`, `AltKeyToIncreaseKeys`, `KeyToDecreaseKeys/Buttons`, `KeyToReset`, `KeyExit`, `KeyToOpenKPSWindow/MenuWindow/GraphWindow/StyleWizard`.

**Design tokens** — `UiTokens::BaseSpacing`, `UiTokens::PrimaryTextSize`, `UiTokens::SecondaryTextSize`, `UiTokens::SurfaceColor`, `UiTokens::AccentColor`, `UiTokens::BorderColor`.

**Colors** are RGB(A), 0–255 per channel; alpha defaults to 255. **Positions/distances** are typically −500..500; **sizes** 0..500 (per the config doc).

## Appendix B: Glossary

| Term | Meaning |
|---|---|
| **Overlay** | The main `sf::RenderWindow`, usually frameless, drawn on top of other apps. |
| **Button** | One tracked key/mouse input = `LogButton` (logic) + `GfxButton` (visual). |
| **LogicalParameter** | A single typed, `Settings`-bound config value (the config atom). |
| **ParameterLine** | One editable row inside the Menu. |
| **ChangedParametersQueue** | Fan-out buffer for config edits drained each Event tick. |
| **UiMode** | `Application`'s state of which editing surface (if any) is open. |
| **Hooks tick** | Fixed 60 Hz input-sampling tick (BPM/KPS sampling). |
| **Event tick** | Render-frequency UI tick. |
| **UiTokens** | New design-token namespace inside `Settings` for cohesive theming. |
| **GfxButtonSelector** | Popover to rebind a button's key/mouse mapping (right-click). |

---

*End of document.*
