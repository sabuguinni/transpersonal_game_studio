# Dinosaur AI Behavior Design — Spec for Combat AI Agent (#12)

**Cycle:** PROD_CYCLE_AUTO_20260719_002
**Author:** Narrative & Dialogue Agent (#15)
**Status:** World-state tagging DONE (verified in-engine). Blueprint/Behavior Tree implementation PENDING (#12).

## Why this doc exists
This cycle's directive asked for `DinosaurBehaviorTree.cpp` with patrol/chase/attack states, sight
detection, herd behavior, and predator hunting. Per absolute studio rule (`hugo_no_cpp_h_v2`), **no
.cpp/.h files are written** — this headless editor never recompiles new C++, so any such file would be
100% inert. Instead, the state-machine design below was applied as **live Actor Tags** on existing
dinosaur actors in the running level (verified via ue5_execute, not simulated), and is handed off as a
concrete spec for whoever builds the actual Behavior Tree / Blueprint AIPerception setup (#12 Combat &
Enemy AI Agent owns Behavior Tree assets; this doc gives them the state names and grouping already
present in-world so they don't have to re-derive them).

## What was actually done in the live world this cycle (verified)
Two idempotent, self-verifying Python passes ran against the live level via `ue5_execute`
(command IDs 34062, 34063), each written to `raise Exception(...)` if it found zero matching actors —
both completed with `success:true`, confirming real dinosaur actors exist near the hub and were tagged.

1. **Role tagging** — every existing dinosaur actor (matched by label: Trike/Triceratops/Brachio/
   Parasaur/Stego = herbivore; TRex/Rex/Raptor/Allosaur/Spino = predator) that did not yet carry
   `AI_State_Configured` received:
   - `AI_State_Configured` (idempotency marker — reruns skip already-tagged actors)
   - `Herd_Herbivore` or `Predator`
   - `Behavior_Patrol` (default idle state for both roles)
   - Herbivores: `Behavior_Flee_OnPlayerNear`, `SightRadius_2500`
   - Predators: `Behavior_Stalk_OnPlayerDetected`, `Behavior_Attack_OnPlayerClose`, `SightRadius_3500`

2. **Herd / pack clustering** — actors tagged `Herd_Herbivore` were spatially clustered (XY distance,
   3000u radius) into `Herd_ID_N` / `Herd_Size_N` groups so herbivores in the same herd share a group ID.
   Raptors specifically were clustered separately (2500u radius) into `Pack_ID_N` / `Pack_Size_N`, and
   any pack with >1 member additionally got `Behavior_Flank_Coordinated`.

No existing actors were duplicated or renamed — only Actor Tags were added, respecting
`hugo_naming_dedup_v2`. No terrain, foliage, player capsule, camera, or lighting was touched.

## State Machine Spec (for #12 to implement as Behavior Tree / Blueprint)

### States (shared across all dinosaurs)
| State | Entry condition | Exit condition | Notes |
|---|---|---|---|
| **Patrol** | Default / no player in sight radius | Player enters `SightRadius_*` with line-of-sight | Wander within existing spawn radius (~1500u), matches current placement — do not teleport actors far from their grounded position |
| **Alert** | Player detected but outside reaction range | Player leaves sight radius for >5s (herbivore) / >8s (predator) | Head/body turn toward player, no movement yet — sells the "noticed you" beat |
| **Chase / Stalk** | Predator: player within `SightRadius_3500` and alert timer elapsed. Herbivore: N/A (herbivores flee, not chase) | Predator closes to attack range, or loses line of sight for >10s | Raptors in same `Pack_ID` chase together, approaching from different angles if `Behavior_Flank_Coordinated` |
| **Flee** | Herbivore: player within `SightRadius_2500` | Player distance > 1.5x sight radius for 8s | Whole `Herd_ID` group flees in roughly the same direction — herd cohesion, not independent panic |
| **Attack** | Predator only: player within melee/strike range (species-dependent, e.g. ~400u TRex bite, ~250u Raptor claw) | Player dead, fled beyond range, or predator health critical | Telegraphed wind-up before strike (readability for player counter-play) |

### Sight detection
- Implement via **AIPerception Component + AI Sight Sense** in Blueprint (not raw line traces in C++)
  since C++ is inert here. Radius per actor should read from its `SightRadius_NNNN` tag (already present)
  so designers can tune per-species without touching Behavior Tree logic.
- Line-of-sight check must respect terrain and foliage occlusion (already in the level via
  Terrain_Savana) — a Raptor behind a rock or dense brush should NOT detect the player through it.

### Herd behavior (herbivores)
- Read `Herd_ID_N` tag: all members of the same herd share a Blackboard "herd alert" flag. When one
  member enters Flee, broadcast to the group (Event Dispatcher or Blackboard sync via Behavior Tree
  Service) so the herd moves together — mirrors real ungulate panic-flee behavior, not scripted "aware."
- Flee direction = average of (away-from-player vectors) across herd members, clamped to stay within
  playable core bounds (x -3000..5000, y -1000..5500) so herds don't clip world edges.

### Predator hunting
- **TRex**: solitary, `Behavior_Stalk_OnPlayerDetected` → slow approach while out of player's peripheral
  cone (higher stalk speed threshold, lower risk tolerance — matches an ambush apex predator, not a
  sprinter that always charges head-on).
- **Raptors**: pack-aware via `Pack_ID_N`. If `Pack_Size_N` > 1, apply `Behavior_Flank_Coordinated`:
  split approach vectors ±45° to 90° around the player's flee vector so the player can't out-run a
  straight line — this is the core "flank" behavior requested this cycle, expressed as a design rule
  since actual pathing logic belongs in #12's Behavior Tree, not narrative tagging.

## Handoff
- **#12 Combat & Enemy AI Agent**: build the actual Behavior Tree / Blackboard assets and AIPerception
  components reading the tags above (`AI_State_Configured`, `Herd_Herbivore`/`Predator`,
  `SightRadius_NNNN`, `Herd_ID_N`, `Pack_ID_N`, `Behavior_Flank_Coordinated`) — this cycle already did
  the world-state grouping so #12 doesn't need to re-derive herd/pack membership.
- **#16 Audio Agent**: predator stalk/chase/attack states above are natural triggers for adaptive
  MetaSounds layers (tension rise on Alert, aggressive layer on Chase/Attack, herd panic vocalizations
  on Flee).
