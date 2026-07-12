# JKPS customization windows, Material 3 Expressive design

This spec defines the next UI pass for JKPS customization windows only: the
`Menu` settings editor and the `StyleWizard` flow. The goal is to make those
windows feel more modern and polished, improve clarity and reduce clutter, and
make editing and window handling feel smoother without changing the underlying
workflow.

The design keeps the current fixed-layout desktop model. JKPS is an overlay
tool, not a general-purpose desktop app, so the customization windows must stay
fast, legible, and predictable. The redesign therefore focuses on stronger
visual hierarchy, expressive color and motion tokens, and safer layout rules
instead of a full interaction rewrite.

## Scope

This spec covers the two customization surfaces that currently drive most of
the visual editing work:

- `Menu`, the dense parameter editor in `Source/Menu.cpp:324-337`.
- `StyleWizard`, the guided theme editor in `Source/StyleWizard.cpp:227-239`.

It does not change the main overlay layout, the key-tracking model, or the
configuration schema. It also does not reintroduce arbitrary window resizing for
these editors. The windows keep their designed canvas sizes and the content
inside them becomes clearer and more expressive.

## Goals

The customization windows must hit three priorities, in order:

1. Feel more modern and polished.
2. Improve clarity and reduce clutter.
3. Make resizing and editing feel smoother.

The design must support all three without turning JKPS into a different product.
The current task flow remains intact: users still edit parameters in `Menu`, and
users still apply guided changes through `StyleWizard`.

## Non-goals

This pass does not attempt to redesign the whole app.

- It does not replace the overlay controls with a new navigation model.
- It does not change the `LogicalParameter` storage model.
- It does not convert JKPS into a fully fluid, freely resizable desktop app.
- It does not introduce new configuration categories.

Those changes would be larger than the current UI problem and would distract
from the windows that need the most attention.

## Current state

JKPS already has the right functional split, but the customization windows are
visually dense and use different interaction styles. `Menu` is a compact editor
with tabs, parameter rows, and a scroll area. `StyleWizard` is a guided flow
with multiple steps and a more narrative rhythm. Both windows open at fixed
canvas sizes, and both are built around a layout that expects its own view to
stay stable.

The current code confirms that behavior:

- `Menu` opens at `959x700` and uses a fixed desktop-style canvas in
  `Source/Menu.cpp:324-337`.
- `StyleWizard` opens at `900x800` and builds its own view in
  `Source/StyleWizard.cpp:227-239`.
- `Application` treats these windows as auxiliary UIs and routes their events
  separately from the main overlay in `Source/Application.cpp:223-237` and
  `Source/Application.cpp:286-327`.

The result works, but it reads as utility software more than a polished editor.
The new design turns that structure into an intentional Material 3 Expressive
experience.

## Design direction

The recommended direction is an expressive shell with a clearer hierarchy.
That means the windows keep their workflows, but the chrome, spacing, grouping,
and motion become more deliberate.

The design takes the existing fixed canvas as a constraint rather than a flaw.
For a tool like JKPS, a stable editing surface is better than a freely resizable
window that can clip or distort the layout. The windows should feel like
designed control surfaces, not generic OS dialogs.

## Visual language

The customization windows should use Material 3 Expressive ideas without
copying a mobile-first pattern blindly. The result should feel light, tactile,
and friendly, but still clearly desktop utility software.

The visual system centers on four ideas:

- **Rounded surfaces** for panels, grouped controls, and action areas.
- **Soft elevation** to separate card-like groups without heavy borders.
- **Strong hierarchy** to show what is primary, secondary, and optional.
- **Controlled color** so active states are obvious without turning the UI loud.

The existing `Settings::UiTokens` namespace is the bridge for this work. The
spec treats those tokens as the start of a shared palette, not a final theme
system. The windows should draw from semantic roles such as surface,
surface-container, primary, accent, outline, and focus ring.

## Menu redesign

`Menu` should become a structured editor with clearer visual segmentation. It
already has the right information architecture: tab strip, parameter list, and
scroll behavior. The redesign keeps that structure and improves how the user
reads it.

The new Menu layout should have these pieces:

- A stronger top chrome area with the window title and a short context line.
- A left-side tab rail or clearer tab header that reads as navigation.
- Parameter groups presented as rounded cards with consistent spacing.
- An active-state treatment that makes the selected section obvious.
- A footer action row for the most important operations, such as reset and
  apply/save.

The parameter list should be calmer and less dense. Labels, toggles, sliders,
and swatches need more breathing room. The point is not to hide information; it
is to make the current information easier to scan.

## StyleWizard redesign

`StyleWizard` should feel more guided and less like a dense form. The current
step flow is already close to the right shape, so the redesign should focus on
progress, preview, and confidence.

The new StyleWizard layout should have these pieces:

- A visible step indicator that shows progress through the flow.
- A large central content surface for the current step.
- A smaller preview or summary area that reflects the active theme choice.
- Clear primary and secondary actions at the bottom of the window.
- A calmer transition between steps so the flow feels deliberate instead of
  abrupt.

This window should feel like a guided decision space. Users should always know
where they are, what changed, and what happens next.

## Motion and interaction

The animation language should stay restrained. JKPS is an overlay tool, so the
motion should support the task instead of attracting attention.

Use motion for:

- Step transitions in `StyleWizard`.
- Selection changes in `Menu` tabs and groups.
- Hover and focus states for controls.
- Subtle press feedback on buttons and toggles.

Motion must stay short, spring-like, and readable. Keep transitions under 400
milliseconds and prefer transform or opacity changes over layout thrashing.
When reduced motion is enabled, fall back to simple fades and instant state
changes.

## Accessibility

The customization windows must remain usable from the keyboard and readable at a
glance. The redesign should improve accessibility rather than rely on the new
visual style to do the work.

The windows must provide:

- Visible focus indicators with sufficient contrast.
- A text contrast ratio that meets WCAG AA for normal text.
- Clear distinction between primary and secondary actions.
- Keyboard navigation that matches the current workflow.
- A reduced-motion path for users who prefer less animation.

The visual language should help here: contrast, padding, and grouping do a lot
of accessibility work before any ARIA-like concepts are needed in this desktop
context.

## Resize and layout behavior

The customization windows should not become freely resizable canvases. Their
layout is designed around fixed dimensions, and the current bug history shows
that arbitrary resize behavior quickly leads to clipped content and awkward
scaling.

Instead, the windows should behave like stable editing surfaces:

- Keep the designed frame size as the canonical size.
- Snap back to that size when a system resize event tries to distort the layout.
- Use internal scrolling, spacing, and grouping to absorb content density.
- Keep the content responsive inside the frame, not the frame itself.

That approach preserves the current interaction model and prevents the
customization surfaces from becoming brittle.

## Proposed implementation shape

The implementation should stay incremental and low risk.

1. Define the expressive token set for the customization windows.
2. Restyle Menu surfaces, tab chrome, and parameter groups.
3. Restyle StyleWizard steps, progress cues, and actions.
4. Add motion and focus states that follow the M3 Expressive rules.
5. Keep the windows fixed-size and reinforce the resize snap-back behavior.

The spec assumes the current code structure remains in place. `Menu` and
`StyleWizard` are separate windows today, and that separation is useful. The
work is about making those windows feel coherent, not merging them.

## Risks and constraints

The main risk is visual regression. The windows are dense, and small spacing
changes can make them feel worse instead of better. The design therefore keeps
the existing information architecture and changes the chrome, hierarchy, and
tokens first.

The second risk is over-animating a tool that should stay fast. The motion plan
is intentionally conservative because JKPS is used while other applications are
running.

The third risk is accidental layout clipping. The spec explicitly avoids freer
resize behavior for that reason and keeps the window sizes stable.

## Acceptance criteria

The redesign is successful when all of the following are true:

- The Menu looks like a deliberate editor rather than a dense utility sheet.
- The StyleWizard feels guided and calm rather than cramped.
- Primary actions are easy to find and secondary actions are visually quieter.
- Focus states are visible and keyboard navigation still works.
- The windows do not stretch into awkward aspect ratios.
- No customization content is clipped by resize behavior.

## Next steps

After this spec is approved, the next step is to write an implementation plan
for the customization windows only. The implementation plan should break the
work into small, verifiable slices so the UI can be changed safely without
re-opening the layout regressions we already fixed.
