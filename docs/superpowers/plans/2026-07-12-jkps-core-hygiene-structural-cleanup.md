# JKPS Core Hygiene and Structural Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use `superpowers:subagent-driven-development` (recommended) or `superpowers:executing-plans` to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Verify and implement the reviewed codebase fixes: standard clamp usage, safer `LogicalParameter` state, grouped settings for the shared runtime toggles, dead `GfxButton` header cleanup, and CMake build hygiene.

**Architecture:** Keep the current runtime behavior and config format intact. Make the risky changes in small slices: first fix correctness issues in `LogicalParameter`, then introduce compatibility-preserving settings namespaces, then trim dead `GfxButton` declarations, and finally lock down the build with an in-source guard.

**Tech Stack:** C++20, SFML 2.x, CMake, existing `Settings` / `LogicalParameter` / `GfxButton` / `Application` pipeline.

## Global Constraints

- Keep the existing config file format readable.
- Do not change the runtime overlay behavior.
- Preserve the current Menu/StyleWizard mutual exclusion policy.
- Prefer compatibility-preserving refactors over breaking API changes.
- Keep builds passing with `cmake --build build`.

---

### Task 1: Replace the hand-rolled clamp and hide `LogicalParameter` change state

This task removes the only verified low-level correctness smell in the parameter code and tightens encapsulation around the change flag without changing runtime behavior.

**Files:**
- Modify: `Headers/LogicalParameter.hpp`
- Modify: `Source/LogicalParameter.cpp`

**Interfaces:**
- Consumes: `LogicalParameter::setDigit()`, `LogicalParameter::resetChangeState()`.
- Produces: standard-library clamp usage and a private `mChanged` flag with the existing public reset path.

**Steps:**
- [ ] Replace the project-local `clamp<T>` template in `Headers/LogicalParameter.hpp` with `std::clamp`.
- [ ] Move `bool mChanged;` from the `public:` section of `LogicalParameter` into `private:` and keep `resetChangeState()` as the only public change-flag reset path.
- [ ] Update the `setDigit()` template so each branch uses `std::clamp(static_cast<float>(var), mLowLimits, mHighLimits)` directly.
- [ ] Keep `Source/LogicalParameter.cpp` behavior unchanged; it should still set `mChanged = true` in the same places, but now through the private member.
- [ ] Build the project with `cmake --build build`.
- [ ] Start the app with `timeout 5s ./build/JKPS` and confirm it launches cleanly after the parameter code change.

**Verification:**
- Run: `cmake --build build`
- Run: `timeout 5s ./build/JKPS`

**Code shape:**

```cpp
// Headers/LogicalParameter.hpp
#include <algorithm>

private:
    bool mChanged;

template <typename T>
void LogicalParameter::setDigit(T var)
{
    switch (mType)
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
            mValStr = std::to_string(static_cast<int>(*std::get<float *>(mVal)));
            break;
        default: break;
    }
    mChanged = true;
}
```

---

### Task 2: Group the shared settings toggles without breaking existing call sites

This task starts reducing the `Settings` namespace soup by moving the shared runtime toggles into nested groups while preserving the old flat names as compatibility aliases.

**Files:**
- Modify: `Headers/Settings.hpp`
- Modify: `Source/Settings.cpp`

**Interfaces:**
- Consumes: current `Settings::SaveStats`, `Settings::ShowOppOnAlt`, `Settings::ButtonPressMultiplier`, `Settings::ReduceMotion`, and `Settings::Window*` uses.
- Produces: nested `Settings::Other` and `Settings::Window` groups plus compatibility aliases for the existing flat call sites.

**Steps:**
- [ ] Add `namespace Other` inside `Settings` and move the runtime toggles that belong to the existing `[Other]` config section into it: `SaveStats`, `ShowOppOnAlt`, `ButtonPressMultiplier`, and `ReduceMotion`.
- [ ] Add `namespace Window` inside `Settings` and move the main-window toggles and sizing inputs into it: `WindowTitleBar`, `WindowResizable`, `RenderUpdateFrequency`, `WindowBonusSizeTop`, `WindowBonusSizeBottom`, `WindowBonusSizeLeft`, and `WindowBonusSizeRight`.
- [ ] Keep the existing flat names alive with inline references so existing code continues to compile unchanged during the migration.
- [ ] Update `Source/Settings.cpp` to define the new nested namespace storage and keep the alias variables in sync.
- [ ] Verify the existing `Application` and `Menu` code still builds without call-site churn.
- [ ] Build the project with `cmake --build build`.
- [ ] Launch `timeout 5s ./build/JKPS` and confirm startup still reaches the main window without config errors.

**Verification:**
- Run: `cmake --build build`
- Run: `timeout 5s ./build/JKPS`

**Code shape:**

```cpp
// Headers/Settings.hpp
namespace Settings
{
    namespace Other
    {
        extern bool SaveStats;
        extern bool ShowOppOnAlt;
        extern unsigned ButtonPressMultiplier;
        extern bool ReduceMotion;
    }

    namespace Window
    {
        extern bool WindowTitleBar;
        extern bool WindowResizable;
        extern unsigned RenderUpdateFrequency;
        extern int WindowBonusSizeTop;
        extern int WindowBonusSizeBottom;
        extern int WindowBonusSizeLeft;
        extern int WindowBonusSizeRight;
    }

    inline bool &SaveStats = Other::SaveStats;
    inline bool &ShowOppOnAlt = Other::ShowOppOnAlt;
    inline unsigned &ButtonPressMultiplier = Other::ButtonPressMultiplier;
    inline bool &ReduceMotion = Other::ReduceMotion;

    inline bool &WindowTitleBar = Window::WindowTitleBar;
    inline bool &WindowResizable = Window::WindowResizable;
    inline unsigned &RenderUpdateFrequency = Window::RenderUpdateFrequency;
    inline int &WindowBonusSizeTop = Window::WindowBonusSizeTop;
    inline int &WindowBonusSizeBottom = Window::WindowBonusSizeBottom;
    inline int &WindowBonusSizeLeft = Window::WindowBonusSizeLeft;
    inline int &WindowBonusSizeRight = Window::WindowBonusSizeRight;
}
```

---

### Task 3: Remove dead `GfxButton` theme scaffolding from the header

This task trims the biggest header bloat item that is not currently implemented in source: the unused theme/press-animation scaffolding in `GfxButton.hpp`.

**Files:**
- Modify: `Headers/GfxButton.hpp`

**Interfaces:**
- Consumes: the currently live `GfxButton` public API (`update`, `draw`, `updateAssets`, `updateParameters`, `setShowBounds`, `getWidth`, `getHeight`, `getTextIdToDisplay`).
- Produces: a smaller header with no dead `applyEnhanced*` declarations or unused theme structs.

**Steps:**
- [ ] Remove the dead protected enums/structs that are not used anywhere in `Source/GfxButton.cpp`: `PressState`, `PressAnimation`, `ThemeColors`, `ResponsiveSize`, `KeyVisualizerStyle`, `EnhancedGradientColors`, and `ThemeInfo`.
- [ ] Remove the dead protected helper declarations that have no source implementation and no call sites: `applyEnhancedButtonStyling`, `applyEnhancedGradientToSprite`, `applyEnhancedPressBorder`, `applyEnhancedScaling`, `applyEnhancedGradientOverlay`, `getEnhancedGradientColors`, `getEnhancedOverlayButtonColor`, `getEnhancedButtonBorderColor`, `getEnhancedShadowColor`, `getEnhancedPressColor`, `applyResponsiveScaling`, `applyResponsiveTextScaling`, `getEnvironmentScaleFactor`, `getWindowContentScale`, `applyThemeBasedGradient`, `getCurrentTheme`, and `initializeThemeSystem`.
- [ ] Remove the unused static `settingsThemes` declaration.
- [ ] Keep the live `RectEmitter` and rendering members unchanged.
- [ ] Build the project with `cmake --build build`.

**Verification:**
- Run: `cmake --build build`

**Code shape:**

```cpp
// Headers/GfxButton.hpp — keep only the live API
public:
    GfxButton(const unsigned idx, const TextureHolder& textureHolder, const FontHolder& fontHolder);
    void update(float deltaSeconds, bool keyState);
    void draw(sf::RenderTarget &target, sf::RenderStates states) const override;
    void updateAssets();
    void updateParameters();
    static void setShowBounds(bool flag, int idx = -1);
    static float getWidth(unsigned idx);
    static float getHeight(unsigned idx);
    static TextID getTextIdToDisplay();
```

---

### Task 4: Lock down the build and do the final runtime check

This task adds the CMake guard that prevents in-source builds and then verifies the app still launches with the cleaned-up code.

**Files:**
- Modify: `CMakeLists.txt`

**Interfaces:**
- Consumes: the current out-of-source build workflow.
- Produces: a hard error when the source tree is used as the build tree.

**Steps:**
- [ ] Add a guard at the top of `CMakeLists.txt` that aborts when `CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR`.
- [ ] Keep the current `file(GLOB_RECURSE ...)` setup unchanged; this task is only about preventing accidental in-source builds.
- [ ] Build the project with `cmake --build build`.
- [ ] Launch `timeout 5s ./build/JKPS` and confirm the app starts without emitting new startup warnings.
- [ ] Check `git status --short` to confirm only the intended files changed.

**Verification:**
- Run: `cmake --build build`
- Run: `timeout 5s ./build/JKPS`
- Run: `git status --short`

**Code shape:**

```cmake
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds are not allowed. Use cmake -B build.")
endif()
```
