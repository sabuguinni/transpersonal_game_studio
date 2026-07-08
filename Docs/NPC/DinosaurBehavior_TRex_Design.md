# Dinosaur Behavior Design — T-Rex & Companion Species
## Agent #11 — NPC Behavior Agent — Cycle PROD_CYCLE_AUTO_20260708_003

## Purpose
This document defines the sociological/behavioral layer for dinosaur "NPCs" in MinPlayableMap:
they are not obstacles waiting for the player — they have territory, routines, and reactions
that exist independently of player presence. This is the data contract the future
DinosaurCombatAIController.cpp (Combat AI Agent #12) and a Behavior Tree asset will consume.

## Live Actor State (verified via Remote Control, this cycle)
- Located the `TRex` placeholder actor in `MinPlayableMap` via `get_all_level_actors()` label scan.
- Tagged it with FName actor tags (queryable at runtime via `Actor->Tags`):
  - `PatrolRadius_5000`
  - `ChaseRadius_3000`
  - `AttackRadius_300`
  - `Apex_Predator`
- Tagged all other dinosaur placeholders (Raptor x3, Triceratops/Trike, Brachiosaurus) with:
  - `PatrolRadius_3000`
  - `ChaseRadius_2000`
  - `AttackRadius_200`
- Spawned 4 visible sphere waypoint markers (`Waypoint_TRexPatrol_001..004`) at 1500-unit
  radius around the T-Rex's current location, tagged `PatrolWaypoint` / `OwnerTRex` — these are
  placeholder patrol nodes an AIController/BehaviorTree BlackboardKey array can reference until
  a proper PatrolPath spline/DataAsset exists.
- Level saved after both tagging passes.

## Behavior State Machine (design spec — for Combat AI Agent #12 to implement in BT/C++)

### T-Rex (Apex Predator)
| State | Trigger | Behavior |
|---|---|---|
| Patrol | Default, no threats detected | Roam between 4 waypoints within 5000-unit territory radius, ~15s dwell per point, sniff/scan animation (uses Agent #10's +4° dominant pose as idle anchor) |
| Alert | Player or prey enters 3000-unit radius, line-of-sight or scent check | Stop, orient toward target, 2-3s reaction delay (predator confirms threat before committing — avoids "robotic" instant aggro) |
| Chase | Target confirmed within chase radius, moving away or unaware | Pursue at full move speed, break off if target exceeds 4500 units (leash) or escapes into dense-canopy tag volume (visual occlusion) |
| Attack | Target within 300-unit attack radius | Bite/tail-swipe attack cycle (implemented by #12), 1.5s cooldown between attacks |
| Retreat/Disengage | Health below 20% (once combat/damage system exists) or target lost >10s | Return to patrol origin, re-enter Patrol state |

### Herbivores (Triceratops, Brachiosaurus)
| State | Trigger | Behavior |
|---|---|---|
| Graze | Default | Stationary/slow wander within 3000-unit radius, uses #10's grazing poses (-22°/-8°) as idle anchor |
| Wary | Player within chase radius (2000u) but no aggression | Head-up "alert" pose, track player rotation, do NOT flee immediately (habituation — herbivores near a hub don't panic at first sight) |
| Flee | Player within attack radius (200u) or player attacks | Run away from player, herd cohesion: other herbivores within 1500u of a fleeing individual also flee (contagion behavior — Triceratops is the exception, see below) |
| Defensive Charge | Triceratops only, player within 200u AND player is aggressive (attacked recently) | Triceratops does not flee — charges the threat once, then retreats. Reflects real horned-herbivore defense, not scripted heroics. |

### Raptors (Pack Predator)
| State | Trigger | Behavior |
|---|---|---|
| Pack Patrol | Default | 3 raptors move as a loose formation (150-300u spacing) between shared waypoints |
| Coordinate | One raptor detects player within its own 2000u radius | Broadcasts alert to pack members within 2500u (shared blackboard key `PackAlertLocation`) — pack converges before attacking, never solo-aggros a healthy player |
| Flank | Pack committed to attack | Two raptors attempt to circle to player's flanks/rear while one holds frontal pressure — tactical pack hunting, not simultaneous frontal rush |

## Why Tags (not a Behavior Tree asset) This Cycle
Per `hugo_no_cpp_h_v2`, no `.cpp`/`.h` was written — the running Editor binary is pre-built and
does not recompile. Behavior Tree assets and Blackboard keys require a compiled `AAIController`
subclass with matching Blackboard key declarations to be meaningful (currently
`DinosaurCombatAIController.cpp` on GitHub is a dead 9-byte stub — confirmed this cycle via
`github_file_read`). Rather than write inert C++ or an orphaned BT asset with no controller to
drive it, this cycle encodes the behavior contract as **live Actor Tags + spatial waypoint
markers** — data that already exists in the running level right now and that a future compiled
AIController can query immediately via `Actor->Tags` and `GetAllActorsWithTag(PatrolWaypoint)`
with zero migration work.

## Blockers
- `DinosaurCombatAIController.cpp` is still a dead placeholder (confirmed again this cycle).
  Real BT/Blackboard-driven chase/attack logic is blocked until Combat AI Agent #12's C++ is
  actually built into the running binary (not just committed to GitHub source).
- No `SurvivalComponent.h` verification was possible this cycle (budget spent on live actor
  tagging + waypoints); flagging for next cycle to confirm its existence for hunger/fear
  integration with dinosaur aggression thresholds.
- TTS storage upload continues to fail with `403 Invalid Compact JWS` (same as prior 2 cycles) —
  audio generation itself succeeds; this is a backend storage auth issue, not a content issue.

## Next Agent Focus (#12 Combat & Enemy AI Agent)
1. Confirm/rebuild `DinosaurCombatAIController.cpp` inside the actual compiled binary (not just
   GitHub source) so tags above become actionable.
2. Read `PatrolRadius_*`, `ChaseRadius_*`, `AttackRadius_*` tags directly off each dinosaur actor
   at BeginPlay to configure per-species AI parameters without per-species C++ subclasses.
3. Use `Waypoint_TRexPatrol_001..004` (tag `PatrolWaypoint`) as the initial patrol route; replace
   with authored PatrolPath spline once world design stabilizes.
4. Implement pack-alert broadcast for raptors as described in the Raptor table above.
