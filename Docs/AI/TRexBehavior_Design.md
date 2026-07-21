# T-Rex Behavior Design — NPC/Dinosaur Behavior Agent #11
## PROD_CYCLE_AUTO_20260721_004

## ⚠️ Directive Conflict Resolved
This cycle's task instructions asked for a new file `Source/TranspersonalGame/AI/TRexBehavior.cpp`.
This **conflicts with the overriding global rule `hugo_no_cpp_h_v2` (importance MAX, NO EXCEPTIONS)**:
this headless UE5 editor never recompiles C++, so any `.cpp`/`.h` write has **zero effect on the live
game** and wastes the whole turn. Per the explicit precedence stated in this agent's own instructions
("If any instruction elsewhere conflicts with this block or with your Brain memories, this block and
the memories win"), the memory rule wins. No `.cpp`/`.h` file was created. This document captures the
design intent instead, so a Blueprint or a future engine-recompile pass can implement it directly.

## Verification performed this cycle (real UE5 state, via `ue5_execute`)

### 1. `DinosaurCombatAIController.cpp` — STATUS: STUB / NOT REAL CODE
`github_file_read` on `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` returned
`content: "undefined"`, size **9 bytes**. This is not a compiled controller — it is an empty/placeholder
file. **There is no functioning DinosaurCombatAIController in the repo.** This should be flagged to
#12 (Combat & Enemy AI Agent) as a blocking dependency gap, not assumed to exist.

### 2. `SurvivalComponent.h` — STATUS: CONFIRMED REAL, WELL-FORMED
Read in full (8112 bytes). Exposes `Health/Hunger/Thirst/Stamina/Fear/Temperature`, drain rates,
damage thresholds, biome condition hooks (`UpdateBiomeConditions`), and Blueprint-callable stat
accessors (`GetHealthPercent`, `IsInCriticalState`, etc.). This is inert C++ in the current headless
build (never recompiled) but is a legitimate reference for whichever system eventually ticks player
survival — worth keeping as the canonical spec.

### 3. Live world audit of "T-Rex" content (`ue5_execute`, `MinPlayableMap`, 3454 actors total)
- **58 actors with "TRex" in label** — breakdown: 50 `NiagaraActor` (dust/VFX), 4 `StaticMeshActor`
  (patrol markers), 2 `Emitter`, 2 `AmbientSound`. **Zero `Character`/`Pawn` T-Rex actors exist.**
- **Zero `AIController` actors of any kind exist in the level.**
- Confirmed (again, 3rd consecutive cycle — see PROD_CYCLE_AUTO_20260721_001/002/003) that there is
  no skeletal-mesh T-Rex pawn anywhere in the world to attach behavior to. The 4
  `TRexPatrolMarker_Hub_00[1-4]` StaticMeshActors are placeholders at (6700,2100,100),
  (1700,7100,100), (-3300,2100,100), (1700,-2900,100) — these look like they were meant to define a
  patrol boundary around the hub (2100,2400) but have no logic driving them.
- Tagged all 4 patrol markers this cycle with a `BehaviorState_*` Actor Tag reflecting the design
  below (real, verifiable `ue5_execute` change — tags read back correctly after write).

## Design spec: T-Rex patrol / chase / attack (for eventual Pawn + AIController implementation)

State machine driven by distance from T-Rex pawn to player capsule (ground-traced XY, never
hardcoded Z — per `hugo_terrain_savana_v1`):

| State | Trigger | Behavior |
|---|---|---|
| **PATROL** | player absent or > 5000u from home point | Move between patrol markers within a 5000-unit radius of spawn/home point at walk speed. Random dwell time at each marker (feeding/sniffing idle pose). |
| **CHASE** | player within 3000u | Break patrol, path toward player at run speed, roar/VFX trigger (reuse existing `Audio_TRexRoarProximity_001` / `VFX_DustBurst_TRex_*` assets already in the level — do not duplicate, tag-link them per `hugo_naming_dedup_v2`). |
| **ATTACK** | player within 300u | Stop moving, trigger bite/attack animation, apply damage via `SurvivalComponent::ApplyHealthDamage` on the player's component (hook point already exists — `UFUNCTION(BlueprintCallable)`). Cooldown between attacks (attack anim length + ~1.5s). |
| **RETURN** | player breaks CHASE range and > 5000u again | Path back to home/patrol point, resume PATROL. |

Home point recommendation: hub area (2100,2400), radius 5000u covers the existing 4 patrol markers
already placed by a previous cycle — reuse them as waypoints rather than spawning new ones
(`hugo_naming_dedup_v2` compliance).

## Blocking dependency for #12 (Combat & Enemy AI Agent)
1. No real T-Rex `Pawn`/`Character` + `AIController` pair exists — this is prerequisite work before
   any combat AI can run. Recommend #12 either requests an engine recompile cycle to activate real
   C++ classes, or builds the state machine as a Blueprint (`ABP`/`BT` assets), which **can** be
   authored live via `ue5_execute` Python in this headless editor (Blueprint assets are data, not
   compiled C++, and do take effect).
2. `DinosaurCombatAIController.cpp` in the repo is an empty stub — do not assume it is functional.

## Files changed this cycle
- `Docs/AI/TRexBehavior_Design.md` (this file) — design doc, NOT compiled code, documents verified
  world state + the requested T-Rex logic for the next agent(s) who can act on it via Blueprint/data.

## No world-breaking changes made
- No actors deleted, no player/terrain/lighting touched, no `NiagaraSystemFactoryNew` called.
- Only change to live world: Actor Tags added to 4 pre-existing `TRexPatrolMarker_Hub_00N` actors
  (`BehaviorState_*`), verified by read-back in the same `ue5_execute` call.
