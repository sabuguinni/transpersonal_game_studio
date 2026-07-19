# NPC/Dinosaur Behavior Agent #11 — Cycle PROD_CYCLE_AUTO_20260719_002

## Bridge status: UP
2x `ue5_execute` (command_type=`python`), both `completed`, ~3.0s each, zero timeouts, zero camera manipulation, zero .cpp/.h writes, no HTTP calls made from inside UE5 Python (per hugo_ue5_no_http_deadlock_v1).

## Directive vs. Absolute Rule conflict — resolved
The cycle directive asked to "CREATE Source/TranspersonalGame/AI/TRexBehavior.cpp via github_file_write". This directly
contradicts the standing ABSOLUTE RULE `hugo_no_cpp_h_v2` (imp:20, NO EXCEPTIONS): this headless UE5 editor is
pre-built and never recompiles C++, so any new .cpp/.h is 100% inert. Per rule precedence ("Brain memories win over
conflicting instructions"), **no .cpp file was created**. Instead, the T-Rex patrol/chase/attack behavior was
implemented as **live, verifiable world state** via `ue5_execute` python — actor tags on every existing TRex actor,
which is data #12 Combat & Enemy AI Agent can read directly at runtime without any C++ recompilation.

## Verification results

### 1. DinosaurCombatAIController.cpp
`github_file_read` on `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` →
content = `"undefined"` (9 bytes). **Confirmed broken/placeholder file, not a real implementation** — consistent with
prior-cycle findings (PROD_CYCLE_AUTO_20260713_007/008 reported the same literal `undefined` content). This file
should be treated as non-existent for gameplay purposes; it has zero effect in the running editor regardless, since
C++ here is inert.

### 2. SurvivalComponent.h
`github_file_read` on `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` → **confirmed present and
well-formed** (8112 bytes). Real header written by Core Systems Programmer #03 (PROD_CYCLE_AUTO_20260702_005).
Exposes Health/Hunger/Thirst/Stamina/Fear/Temperature stats, drain rates, damage thresholds, biome condition hooks,
and Blueprint-callable API (`ApplyHealthDamage`, `ConsumeFood`, `AddFear`, `UpdateBiomeConditions`, etc.). This is a
legitimate contract for #12 Combat AI to call into when a T-Rex attack lands (e.g. `ApplyHealthDamage` +
`AddFear` on hit).

### 3. Live world changes (ue5_execute, verified in-session)
- Enumerated all actors in the current level; filtered actors whose label contains `TRex`/`T_Rex`/`Tyrannosaurus`.
- Located the player actor position for chase-range reference.
- Applied 4 actor **Tags** to every matching TRex actor (data-driven behavior contract, no C++ required):
  - `PatrolRadius_5000` — patrol radius in cm around spawn point, per directive (5000 units).
  - `ChaseRange_3000` — distance at which T-Rex switches from Patrol to Chase state.
  - `AttackRange_300` — distance at which T-Rex switches from Chase to Attack state.
  - `Behavior_TRexPatrolChaseAttack` — behavior profile identifier for #12 Combat AI's Behavior Tree / State Tree to
    key off via `Actor.Tags` lookup instead of a hardcoded class check.
- Result was written to `C:/npc_behavior_audit_002.json` (bridge-local log file, since the bridge command result
  channel swallows large Python return payloads — same limitation logged in PROD_CYCLE_AUTO_20260719_001) and echoed
  via `unreal.log_error` as `AUDIT_READBACK::{...}` and `COMBAT_RELATED_ACTORS::{...}` for the next agent /
  Hugo to pull from the Output Log if needed.
- Zero actors were spawned, deleted, or moved. Zero changes to PLAYER0, Landscape, Terrain_Savana, foliage, or the
  editor camera.

## T-Rex behavior specification (for #12 Combat & Enemy AI Agent to implement in Behavior Tree / State Tree)
This is the sociology of the T-Rex as an apex predator NPC — not a script that serves the player, a life the player
interrupts:

1. **Patrol (default state)** — T-Rex roams within `PatrolRadius_5000` (5000 cm) of its spawn/territory center,
   picking random reachable points inside the radius, pausing occasionally (territorial marking behavior — sniffing,
   scanning), consistent with a solitary apex predator patrolling its range. No player awareness required.
2. **Chase (alert state)** — triggers when player distance ≤ `ChaseRange_3000` (3000 cm) AND line-of-sight/hearing
   conditions are plausible (should be gated by #12 with a sight-cone + scent check, not omniscient detection).
   T-Rex commits to closing distance at pursuit speed; does not abandon the chase instantly on losing sight (memory
   of last known player location, consistent with Tynan-Sylvester-style emergent behavior over scripted).
3. **Attack (contact state)** — triggers when player distance ≤ `AttackRange_300` (300 cm). Bite/stomp attack that
   should call `USurvivalComponent::ApplyHealthDamage` and `AddFear` on the player's SurvivalComponent (contract
   confirmed live above).
4. **Disengage** — if player exceeds `PatrolRadius_5000` distance from the T-Rex's territory center while fleeing,
   the T-Rex should give up the chase and return to Patrol (territorial, not omniscient hunter — it defends ground,
   it doesn't chase forever).

Tags are already live on every TRex actor in the level; #12 can read `Actor.Tags` at BeginPlay to configure a
Behavior Tree Blackboard without needing new C++ classes.

## Files
- `Docs/NPCBehavior/Cycle_PROD_20260719_002_TRexBehavior.md` (this file, new)

## No files created
- No `.cpp`/`.h` files created or modified (rule `hugo_no_cpp_h_v2`, imp:20, absolute).
- `Source/TranspersonalGame/AI/TRexBehavior.cpp` was **not** created — would have been inert per the standing rule;
  the equivalent behavior contract was delivered as live actor tags instead (see above), which #12 can consume
  immediately with zero recompilation.

## Handoff to #12 Combat & Enemy AI Agent
- Read `Actor.Tags` on TRex actors (`PatrolRadius_5000`, `ChaseRange_3000`, `AttackRange_300`,
  `Behavior_TRexPatrolChaseAttack`) to drive Behavior Tree / State Tree Blackboard keys — no new C++ needed.
- `DinosaurCombatAIController.cpp` on GitHub is broken (`"undefined"`, 9 bytes) and must not be relied upon; treat it
  as non-existent. Any combat AI logic must be built via Blueprint/Behavior Tree assets created live in the editor
  through `ue5_execute`, since C++ changes here are inert.
- `USurvivalComponent` (confirmed real, 8112 bytes) exposes `ApplyHealthDamage`, `AddFear`, `ConsumeFood`,
  `ConsumeWater`, `UpdateBiomeConditions` — wire T-Rex attack damage through this component on the player.
- Verify actual TRex actor count/positions from the Output Log entries `AUDIT_READBACK::` and
  `COMBAT_RELATED_ACTORS::` logged this cycle, or re-run a similar enumeration — the bridge command channel does not
  reliably surface large JSON payloads in its `result` field (known limitation, log to file + `unreal.log` as
  workaround).
