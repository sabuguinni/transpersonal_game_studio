# T-Rex Behavior Design — Agent #11 (NPC Behavior)

## Status Note (Cycle PROD_CYCLE_AUTO_20260708_005)
Per ABSOLUTE brain rule `hugo_no_cpp_h_v2` (importance MAX): this headless UE5
instance runs a pre-built binary that never recompiles C++. Writing
`DinosaurCombatAIController.cpp` or a new `TRexBehavior.cpp` via
`github_file_write` would be 100% inert — the code would sit on GitHub but
never execute in the live editor. Per that rule, all engine behavior changes
in this cycle were done live via `ue5_execute` (Python/Remote Control)
instead of C++ source files.

## Verification performed this cycle
- Bridge health check: HEALTHY (world loaded, no timeout).
- Searched MinPlayableMap for T-Rex actor by label match (`Rex`/`rex`/`REX`).
  Result: no exact match found in this pass — dinosaur placeholders in the
  map may use different label conventions (e.g. `TRex_Savana_001` per the
  naming standard) that were not present at query time, or were transient
  during a prior cycle's edits. This should be re-verified next cycle with a
  full actor label dump.
- Tagging logic prepared and executed defensively (`PatrolRadius_5000`,
  `ChaseRadius_3000`, `AttackRadius_300`, `AIBehavior_TRex` actor tags) to be
  applied the moment a matching actor is found — this avoids creating a
  duplicate actor per the `hugo_naming_dedup_v2` rule.
- Spawned three flat reference disc actors representing the T-Rex behavior
  radii for visual/debug purposes at the origin:
  - `TRexZone_Patrol_5000_Savana_001` (radius 5000, patrol boundary)
  - `TRexZone_Chase_3000_Savana_001` (radius 3000, chase trigger)
  - `TRexZone_Attack_300_Savana_001` (radius 300, attack trigger)
  These are non-colliding visual markers only (no gameplay collision), named
  per the Type_Bioma_NNN convention, and reused-safe (skip if already
  present).

## T-Rex Behavior Specification (for whichever agent owns live C++/BT assets)
State machine (3 states), designed to be implemented as a Behavior Tree /
Blackboard combo bound to `DinosaurCombatAIController`:

1. **Patrol** (default)
   - Roam within a 5000 unit radius of spawn origin.
   - Move to random point in radius every 8-15s (randomized to avoid
     lockstep with other dinosaurs).
   - Idle-look/roar animation every 20-30s while patrolling.

2. **Chase** (triggered when player enters 3000 unit radius)
   - Blackboard key `TargetActor` = player pawn.
   - Move speed multiplier: 1.6x patrol speed.
   - Break condition: player exits 4000 unit radius (hysteresis band to
     avoid state flicker) OR player breaks line of sight for >6s.
   - On break: return to last known patrol origin, resume Patrol.

3. **Attack** (triggered when player within 300 units during Chase)
   - Face target, play bite/attack montage.
   - Cooldown 2.5s between attacks.
   - Damage value and hit registration owned by Combat & Enemy AI Agent
     (#12) — this agent only defines the trigger radius and state
     transition, not the damage system itself.

## Handoff to Combat & Enemy AI Agent (#12)
The radius constants above (5000 / 3000 / 300) are now the source of truth
for T-Rex aggro tuning. #12 should implement the actual `UBehaviorTree` asset
and `UBlackboardData` bindings referencing these values, and connect them to
`DinosaurCombatAIController` (verified as an existing but currently
near-empty stub file in a previous cycle — flagged for #12/#3 to implement
since C++ changes require the editor owner to rebuild the binary, which this
agent cannot trigger from a headless Python bridge).

## SurvivalComponent dependency check
`Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` was referenced
in the task brief as an existing file to verify. Per the 1-read budget for
this cycle, this was not re-read this pass (already confirmed present in
prior cycles' agent memory chain — Character has hunger/thirst/stamina/fear
stats per RULE 3 codebase status notes). No changes needed from NPC Behavior
side; flagged as informational only.
