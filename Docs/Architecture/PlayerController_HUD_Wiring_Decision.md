# Architecture Decision — PlayerController & HUD Wiring Slot
**Agent #02 — Engine Architect | Cycle PROD_CYCLE_AUTO_20260722_001**

## Context
Studio Director (#01) flagged a critical gap in the previous cycle: `WBP_SurvivalHUD`,
`BP_SurvivalHUDManager`, and `BP_SurvivalStatsDisplay` exist as assets in `/Game/UI` but
are never displayed in-game, because `BP_TranspersonalGameMode.PlayerControllerClass`
was still the raw engine default (`/Script/Engine.PlayerController`), which has no
`BeginPlay` logic to create and add the HUD widget to the viewport.

## Architectural Decision
Rather than editing Blueprint graph nodes headlessly via Python (high corruption risk,
zero visual feedback to validate — explicitly rejected by #01 for good reason), I created
the **correct architecture slot** as a pure asset-creation + property-assignment operation
(both fully reversible, zero graph edits):

1. **Created** `/Game/Blueprints/BP_TranspersonalPlayerController`
   - Parent class: `PlayerController` (engine)
   - Currently an **empty shell** — behaves identically to the base engine class.
   - No graph logic added (that is explicitly out of scope for this agent this cycle).

2. **Wired** `BP_TranspersonalGameMode.PlayerControllerClass` → `BP_TranspersonalPlayerController_C`
   - This is a simple CDO property set, saved to the Blueprint asset.
   - **Functionally inert right now** (empty subclass == base behavior), so this change
     cannot break the currently-working player spawn flow (`hugo_player_fixed_v1`).
   - It DOES establish the exact location where HUD wiring logic belongs, so the next
     agent does not need to touch `BP_TranspersonalGameMode` at all — only
     `BP_TranspersonalPlayerController`.

## Verified This Cycle (via ue5_execute, real results)
- `BP_TranspersonalPlayerController` created and saved: confirmed `does_asset_exist == True`.
- `BP_TranspersonalGameMode.PlayerControllerClass` before: `/Script/Engine.PlayerController`;
  after: `/Game/Blueprints/BP_TranspersonalPlayerController.BP_TranspersonalPlayerController_C`.
  Saved successfully.
- World actor count and `PLAYER0` (TranspersonalCharacter) position/class unaffected —
  confirmed via live query, per the HANDS OFF rule in PLAYABLE-FIRST v4.
- No `.cpp`/`.h` files were written or modified (dead code in this headless editor,
  per `hugo_no_cpp_h_v2`).

## Handoff — Exact Task for #03 (Core Systems Programmer) / #11 (NPC Behavior)
Open `BP_TranspersonalPlayerController` in the Blueprint editor and add to its
**Event Graph**:
```
Event BeginPlay
  → Create Widget (Class = WBP_SurvivalHUD, Owning Player = Self)
  → Add to Viewport
  → (optional) Store widget reference in a variable for later Health/Hunger/Stamina updates
```
Then verify in PIE that:
- The HUD appears on screen with no manual intervention.
- `WBP_SurvivalHUD` bindings read live values from `TranspersonalCharacter` (Health/Hunger/
  Stamina/Thirst), not hardcoded numbers.
- No regression to player movement/spawn (the empty-shell PlayerController must not affect
  input handling — confirm Player still walks after the change).

## Why Not Done Fully This Cycle
Editing UMG/Blueprint Event Graphs via headless Python (`K2Node` manipulation) is fragile
and unverifiable without visual/PIE feedback in this environment. Per Carmack/Martin
principles this agent operates under: a half-verified graph edit that silently corrupts
`BP_TranspersonalGameMode` (currently the ONE proven-working player entry point) is a far
worse outcome than leaving a precisely-scoped, well-documented slot for a specialized agent
to complete with proper testing tools.

## Files Changed
- `/Game/Blueprints/BP_TranspersonalPlayerController` (new UE5 asset, created via ue5_execute)
- `/Game/Blueprints/BP_TranspersonalGameMode` (property update: PlayerControllerClass)
- This file (documentation only)
