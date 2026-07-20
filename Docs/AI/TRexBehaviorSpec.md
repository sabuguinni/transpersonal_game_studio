# T-Rex Behavior Specification — NPC/Dinosaur Behavior Agent #11
Cycle: PROD_CYCLE_AUTO_20260720_005

## IMPORTANT — no .cpp/.h written this cycle
Per `hugo_no_cpp_h_v2` (imp:MAX, absolute rule): this headless editor never recompiles
C++, so `.cpp`/`.h` writes have zero effect on the live game. The original task asked
for `Source/TranspersonalGame/AI/TRexBehavior.cpp`, but that would be 100% wasted work.
Instead, the behavior spec below is documented here (for #12 Combat & Enemy AI Agent to
implement as Blueprint/Behavior Tree assets in the live editor, or for a future engine
rebuild to consume as C++ if the pipeline changes), and the equivalent state is already
applied live in-world via `ue5_execute` (actor tagging — see verification section).

## Verification of existing combat AI file (GitHub)
`Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` was read this cycle:
content is literally the string `undefined` (9 bytes). This is a stub/corrupted file from
an earlier agent, not a real controller. It has zero effect on the live editor regardless
(per the no-C++ rule) — flagged here for the record, not fixed via C++ since that would
also be wasted effort. If dinosaur combat behavior needs to be *live*, it must be built as
a Blueprint AIController + Behavior Tree asset in UE5 directly (Combat & Enemy AI Agent #12
mandate).

## Verification of SurvivalComponent
`Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` exists and is a complete,
well-formed UActorComponent header (Health/Hunger/Thirst/Stamina/Fear/Temperature,
drain rates, damage thresholds, delegates, BlueprintCallable API). This is real content
from Core Systems Programmer #03 (PROD_CYCLE_AUTO_20260702_005) — not a stub. Since it's
a `.h` file, it is inert in the current headless build (no recompilation), but it stands
as the reference design for hunger/thirst mechanics wherever a live Blueprint equivalent
is implemented.

## T-Rex Behavior Design (for Blueprint/BT implementation by Agent #12)

### States
1. **Patrol** — default state. T-Rex wanders within a 5000 unit radius of its spawn/anchor
   point, moving between 3-5 patrol nodes at a slow "grazing/alert" pace. Uses existing
   pose variants already present in the world (`_grazing_Posed`, `_alert_Posed`,
   `_midstride_Posed`) as blend targets for a patrol Behavior Tree once real skeletal
   T-Rex actors are BT-driven (currently these are static posed meshes, not yet BT pawns).
2. **Chase** — triggered when distance to player < 3000 units (matches existing
   `AggroZone_TRex_*` trigger volumes already placed in the hub, e.g.
   `Trigger_Quest_Combat_TRex_AggroZone_001`, `AmbushZone_TRex_001`). T-Rex breaks patrol,
   turns to face player, and moves at a running gait toward last known player location.
3. **Attack** — triggered when distance to player < 300 units. T-Rex performs bite/tail-
   swipe attack montage, applies damage via `SurvivalComponent::ApplyHealthDamage` on the
   player's SurvivalComponent, then briefly cools down before re-evaluating distance.
4. **Disengage** — if player exceeds 4000 units distance (hysteresis above chase radius)
   or breaks line-of-sight for >8s, T-Rex returns to nearest patrol node and re-enters
   Patrol state.

### Numbers (as required this cycle)
- Patrol radius: **5000 units** around spawn anchor.
- Chase trigger: player within **3000 units**.
- Attack trigger: player within **300 units**.
- Disengage: player beyond **4000 units** (hysteresis) or LOS lost 8s+.

### Live world state applied this cycle
- Confirmed **196 real T-Rex actors** in the playable core (excluding 109 pure
  Helper_Actor/Marker_AI/Light_Aux anchor clutter, which are non-visual scaffolding).
- Tagged all 196 real T-Rex actors with Actor Tag `Behavior_TRex_PatrolChaseAttack` via
  live `ue5_execute` — this is a real, verifiable in-editor change (actor.tags array),
  giving Agent #12 a ready-made query set (`get_all_level_actors` filter by tag) to attach
  AIControllers/Behavior Trees to without re-scanning/re-identifying which actors are real
  dinosaurs vs. VFX/trigger scaffolding reusing the TRex name.

## Handoff to #12 Combat & Enemy AI Agent
- Query actors by tag `Behavior_TRex_PatrolChaseAttack` (196 actors) to attach real
  AIController + Behavior Tree logic — do not re-derive the real/clutter split, it's done.
- `DinosaurCombatAIController.cpp` on GitHub is a stub (`undefined`) — build combat AI as
  live Blueprint/BT assets directly in UE5, not as new C++.
- `SurvivalComponent.h` design (Health/Fear/Stamina) is the reference for how T-Rex attacks
  should apply damage/fear to the player if a live equivalent exists in Blueprint form.
- Patrol/Chase/Attack/Disengage radii above (5000/3000/300/4000) are the agreed numbers —
  keep consistent across any BT implementation so tuning stays centralized.
