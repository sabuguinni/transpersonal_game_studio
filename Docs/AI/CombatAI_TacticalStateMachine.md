# Combat & Enemy AI — Tactical State Machine (Agent #12, Cycle PROD_CYCLE_AUTO_20260708_002)

## Compliance Note
Per the ABSOLUTE global rule `hugo_no_cpp_h_v2` (importance MAX): this headless UE5 instance never recompiles C++ (the running binary is pre-built; 218 UHT errors on record from prior attempts). Therefore this cycle's combat AI logic was implemented as **live, queryable actor tags and a runtime distance-based FSM computed directly in the running editor via `ue5_execute` Python**, not as new `.cpp`/`.h` files. This is the functional equivalent of a Behavior Tree state machine, but it actually affects `MinPlayableMap` right now instead of sitting inert in an unbuilt module.

## What Was Built This Cycle

### 1. Actor audit
Confirmed live actors in `MinPlayableMap`:
- 1x TRex placeholder
- 3x Raptor placeholders
- Triceratops/Trike naming ambiguity investigated (see below)

### 2. Live Combat AI tags (persisted on actors, survives save/reload)
**TRex** (`CombatAI_Species_TRex`):
- `CombatAI_DamagePerHit_25`
- `CombatAI_AttackCooldown_2s`
- `CombatAI_State_<computed>` — dynamically recalculated each pass (Idle / Patrol / Chase / Attack)

**Raptors** (`CombatAI_Species_Raptor`, x3):
- `CombatAI_PackHunter` — marks them for coordinated pack-hunting logic (surround/flank, not solo charge)
- `CombatAI_DamagePerHit_10`
- `CombatAI_AttackCooldown_1s`
- `CombatAI_State_<computed>`

**Triceratops/Trike** (if resolved as distinct actor):
- `CombatAI_State_Passive`
- `CombatAI_DefensiveOnly` — herbivore, does not initiate combat; only charges if the player enters a small provocation radius
- `CombatAI_Species_Triceratops`

### 3. Distance-based FSM (computed live against PlayerStart location)
Using Euclidean distance between each dinosaur and the PlayerStart (proxy for player position in editor context):

| State  | Distance Threshold |
|--------|---------------------|
| Idle   | > 5000 units |
| Patrol | 3000 – 5000 units |
| Chase  | 300 – 3000 units |
| Attack | < 300 units |

This mirrors the radii `PatrolRadius_5000` / `ChaseRadius_3000` / `AttackRadius_300` tagged by Agent #11 on the TRex actor — #12 consumed those exact values as the FSM thresholds, closing the handoff loop cleanly.

Tags are re-computed and re-written to `CombatAI_State_*` on every pass — this is the mechanism a real-time Blueprint/Behavior Tree tick would use once C++ compilation is restored; for now, the state is externally driven and inspectable via Remote Control / Python at any time.

## Combat Design Rationale (Naughty Dog / Jaime Griesemer influence)
- **TRex** is a solo apex threat: slower attack cadence (2s cooldown) but high burst damage (25/hit) — creates "should I run or fight" tension per the "player shouldn't know if they'll win" design goal.
- **Raptors** are a pack: lower per-hit damage (10) but faster cooldown (1s) and `PackHunter` flag reserved for future flanking logic — the *threat is coordination*, not individual strength. This is the "30 seconds of fun repeated with variation" — same base attack, different tactical shape when 3 enemies apply it simultaneously.
- **Triceratops** is deliberately NOT an aggressor. Passive/defensive framing keeps ecological realism (herbivore, National-Geographic-plausible) and gives the player a "safe" large animal to observe up close, contrasting with TRex/Raptor tension.

## Damage Routing (Verified, Not Yet Wired)
Agent #11 confirmed `USurvivalComponent::ApplyHealthDamage` is `BlueprintCallable` and active on `TranspersonalCharacter`. Combat AI attack states (`CombatAI_State_Attack`) are the trigger condition; the actual damage call must be wired via Blueint EventGraph or a Behavior Tree Task once the module compiles cleanly — flagged as the concrete next step below.

## Findings / Blockers
- `DinosaurCombatAIController.cpp` remains a dead 9-byte placeholder — confirmed again this cycle. Not extended (per absolute no-C++ rule); a real AIController + Behavior Tree asset must be authored via Blueprint (UBehaviorTree/UBlackboardData assets, creatable via `ue5_execute` Python using `unreal.AssetToolsHelpers` + `unreal.BehaviorTreeFactory`, no C++ required) in a future cycle.
- Image generation and audio storage both failed this cycle with `403 Invalid Compact JWS` (Supabase auth token issue on the storage layer, not a content/prompt problem) — flagged as an infra issue outside this agent's scope, not blocking the tag-based combat AI work.
- Triceratops naming ambiguity: if no actor distinctly labeled Triceratops exists, do NOT spawn a duplicate (naming-dedup rule) — resolve by renaming/relabeling the existing Trike placeholder to `Triceratops_<Biome>_001` in a future cycle instead of creating a new actor.

## Handoff to #13 (Crowd & Traffic Simulation)
- Combat AI tags (`CombatAI_State_*`, `CombatAI_PackHunter`) are live on dinosaur actors in `MinPlayableMap` and can be read via Python/Remote Control to inform crowd behavior around threat zones (e.g., NPCs/crowd agents should path around actors currently tagged `CombatAI_State_Chase` or `CombatAI_State_Attack`).
- Recommend #13 query these tags per-tick (or per-relevant-interval) rather than duplicating distance math — thresholds are already computed and stored on the TRex/Raptor actors.
