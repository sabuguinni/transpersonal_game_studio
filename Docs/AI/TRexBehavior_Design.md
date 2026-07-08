# T-Rex Behavior Design — Agent #11 (NPC Behavior) — PROD_CYCLE_AUTO_20260708_002

## Compliance Note (overrides generic .cpp/.h mandate)
Per the ABSOLUTE global rule `hugo_no_cpp_h_v2` (importance MAX): this headless UE5 instance
NEVER recompiles C++. `DinosaurCombatAIController.cpp` was re-verified this cycle and remains a
dead 9-byte placeholder (literal text `undefined`) inherited from prior cycles — writing another
`.cpp` file (e.g. `TRexBehavior.cpp`) would be 100% inert, matching the exact failure pattern this
rule was created to stop. Instead, the behavior below is implemented **live** via `ue5_execute`
Python against the Remote Control bridge (data-driven actor tags), which is the only mechanism
that actually affects the running editor/game state in this environment.

## What was verified this cycle
- Bridge: HEALTHY (2/2 ue5_execute Python calls succeeded, no timeouts).
- `MinPlayableMap` actor audit confirmed dinosaur placeholders present, matching Agent #10's
  handoff: 1× TRex, 3× Raptor, 1× Brachiosaurus. Note: no separate "Triceratops" actor was found
  under that name — per the `hugo_naming_dedup_v2` rule, if a Triceratops-equivalent already
  exists under a different label (e.g. a Raptor placeholder repurposed), no duplicate should be
  spawned. This cycle did NOT spawn a new Triceratops to avoid violating that rule; flagging for
  #12 (Combat AI) to confirm species assignment against `Type_Bioma_NNN` naming convention.
- `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` — CONFIRMED ACTIVE. Full read
  performed. Component exposes Health/Hunger/Thirst/Stamina/Fear/Temperature stats, drain rates,
  damage thresholds, biome condition hooks (`UpdateBiomeConditions`), and a
  `FCore_OnPlayerDied` delegate. This is exactly what NPC/dinosaur AI needs to react to player
  vulnerability state (e.g. T-Rex should be more aggressive toward a player with high Fear /
  low Stamina — hook point for #12 Combat AI).

## T-Rex Behavior Spec (data-driven, live-tagged on the TRex actor this cycle)
Applied via actor Tags (queryable at runtime by any AIController/Blueprint without recompiling):
- `PatrolRadius_5000` — T-Rex patrols within a 5000 unit radius of its spawn origin when no
  player is detected. Patrol should follow terrain contours (reuse Agent #10's ground-snap
  logic — line trace down + 5cm offset — for any patrol waypoint the AI controller generates).
- `ChaseRadius_3000` — if player distance-to-TRex <= 3000 units, transition to Chase state:
  move directly toward player's last known location at full move speed.
- `AttackRadius_300` — if player distance-to-TRex <= 300 units, transition to Attack state:
  stop movement, execute attack (damage authority belongs to #12 Combat AI's damage system).

### State Machine (for #12 to wire into an actual AIController Blueprint or Behavior Tree)
```
IDLE/PATROL  (dist > 3000)  -> wander inside PatrolRadius_5000, ground-snapped
   -> CHASE   (dist <= 3000) -> move to player location, break off if dist > 3500 (hysteresis)
      -> ATTACK (dist <= 300) -> stop, attack, return to CHASE if player retreats past 300
```
Hysteresis buffer (3500 vs 3000) prevents state-flicker at the exact threshold boundary —
standard practice to avoid oscillating AIController ticks.

### Sociological framing (per Agent #11 mandate — NPCs live their own lives)
The T-Rex does not exist to fight the player. Its patrol is its territory, walked whether or
not a player is present. Chase is a territorial/predatory response to intrusion, not a scripted
ambush. This keeps the encounter consistent with a National-Geographic-style predator: an
apex animal defending range and pursuing viable prey, not a boss waiting to be triggered.

## Handoff to #12 (Combat & Enemy AI Agent)
- Build the actual `AAIController` / Behavior Tree asset in-engine (Blueprint or data asset,
  NOT another dead .cpp) reading the `PatrolRadius_5000` / `ChaseRadius_3000` / `AttackRadius_300`
  tags now present on the TRex actor in `MinPlayableMap`.
- Wire damage output through `USurvivalComponent::ApplyHealthDamage` on the player character
  (confirmed available, BlueprintCallable) — do not create a parallel damage system.
- `DinosaurCombatAIController.cpp` remains a dead placeholder; do NOT attempt to extend it in
  C++. Implement combat logic via Blueprint/Behavior Tree assets created live in the editor.
- Resolve Triceratops naming ambiguity noted above before adding species-specific combat logic.
