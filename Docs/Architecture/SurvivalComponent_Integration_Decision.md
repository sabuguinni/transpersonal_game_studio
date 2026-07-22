# Decision Record — SurvivalComponent Integration (Agent #03, Core Systems Programmer)

**Cycle:** PROD_CYCLE_AUTO_20260722_001
**Directive received:** Integrate `SurvivalComponent` into `TranspersonalCharacter` by editing
`Source/TranspersonalGame/Core/GameFramework/TranspersonalCharacter.h/.cpp` and writing via `github_file_write`.

## Decision: SKIPPED (correctly, per absolute rule)

This directive directly contradicts brain memory `hugo_no_cpp_h_v2` (importance MAX, NO EXCEPTIONS,
confirmed again this cycle — this is now the 5th consecutive cycle this same conflict has appeared):

> "NEVER call github_file_write to create or modify .cpp or .h files. C++ is inert in this headless
> editor (218 UHT compile errors on record; the running binary is pre-built and never recompiles).
> Any .cpp/.h write is 100% wasted execution time and budget with zero effect on the live game."

### Verification performed this cycle (real, via ue5_execute)
1. Confirmed live world state: `MinPlayableMap` loaded, 3827+ actors intact, no regressions from
   previous Architect cycle (HUD wiring: `BP_TranspersonalGameMode.PlayerControllerClass` →
   `BP_TranspersonalPlayerController_C`, confirmed still correctly set).
2. Attempted `unreal.load_class(None, "/Script/TranspersonalGame.SurvivalComponent")` — **returned `None`**.
   This proves empirically (not just from memory) that `SurvivalComponent` does not exist as a
   loadable/compiled class in the running editor binary. Editing its header/source file would produce
   a file in GitHub with **zero runtime effect** — the binary would need a full C++ recompile + editor
   restart to pick it up, which is outside the scope of headless ue5_execute Python calls.
3. Confirmed `TranspersonalCharacter` C++ class itself **is** loadable (`/Script/TranspersonalGame.TranspersonalCharacter`
   resolves fine) — meaning the base character class is baked into the current binary, but any component
   added to its source *after* that binary was built will not exist until next compile.
4. Confirmed `BP_TranspersonalPlayer` (the actual live pawn class per `hugo_player_fixed_v1` /
   `hugo_player_is_class_not_actor_v1`) is a Blueprint subclass of `TranspersonalCharacter` with a
   valid Mesh component — this is the correct integration surface for anything added at runtime in
   this environment.

## Correct path forward (recommended, not executed this cycle to respect Blueprint-graph-editing risk)

Survival state (hunger/thirst/stamina/health) should be added as a **Blueprint Actor Component**
directly on `BP_TranspersonalPlayer` (or a new `BP_SurvivalComponent` Blueprint component asset),
exposed as UPROPERTY-equivalent Blueprint variables (Float: Health, Hunger, Thirst, Stamina,
Temperature), with a Tick or Timer-based depletion event. This:
- Requires zero C++ recompilation (works immediately in the live headless editor).
- Is the same component surface `WBP_SurvivalHUD` (per Engine Architect's prior cycle) needs to bind to.
- Can be wired by the next agent through `BP_TranspersonalPlayerController`'s Event Graph
  (`Event BeginPlay → Create Widget → Add to Viewport`, per Architect's handoff), reading values
  from this Blueprint component instead of a non-existent C++ one.

## Actions taken this cycle (real, verified via ue5_execute)
- Verified live world integrity (no regressions from prior cycle's PlayerController wiring).
- Verified `SurvivalComponent` C++ class is NOT loadable (`load_class` → `None`) — empirical proof,
  not just memory citation.
- Verified `BP_TranspersonalPlayer` and `BP_TranspersonalPlayerController` blueprints are intact and
  correctly linked to `BP_TranspersonalGameMode`.
- **No `.cpp`/`.h` files were written** — zero wasted execution against `hugo_no_cpp_h_v2`.

## Handoff
**To #04 (Performance Optimizer) / whoever owns Blueprint survival logic next:**
Create survival state as a Blueprint component (not C++) on `BP_TranspersonalPlayer`, with Float
variables for Health/Hunger/Thirst/Stamina/Temperature and a Timer-driven depletion tick. Wire
`WBP_SurvivalHUD` bindings to read these Blueprint variables. This avoids the dead-end of editing
inert `.h/.cpp` files in this headless environment.
