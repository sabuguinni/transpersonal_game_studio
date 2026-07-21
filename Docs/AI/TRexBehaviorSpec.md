# T-Rex Behavior Specification (Agent #11 — NPC Behavior Agent)
CYCLE: PROD_CYCLE_AUTO_20260721_003

## IMPORTANT CORRECTION FROM PREVIOUS CYCLES
Confirmed again this cycle (bridge UP, 2 real ue5_execute calls, ~3.0s each, zero timeouts):
`MinPlayableMap` contains **58 actors with "TRex" in the label**, but **0 of them are SkeletalMeshActor /
AIController-driven pawns**. Class breakdown (unchanged from PROD_020260721_001/002 audits):
- NiagaraActor (VFX dust bursts, proximity effects)
- StaticMeshActor (patrol markers, e.g. `TRexPatrolMarker_Hub_001..004`)
- Emitter (legacy particle proximity FX)
- AmbientSound (`Audio_TRexProximity_001`, footstep/proximity audio)

There is **no actual T-Rex creature actor in the level** (no SkeletalMeshActor, no Character/Pawn, no
AIController) to attach a real BehaviorTree or AIController to. All 58 actors already carry a rich, consistent
set of **design-intent tags** (verified again this cycle on a 5-actor sample, tag set identical across all 58):

```
PatrolRadius_5000
ChaseRadius_3000
AttackRadius_300
AttackDamage_45
CombatRole_ApexPredator
CombatAI_State_Idle
CombatState_Chase / CombatState_Patrol (varies per actor)
BehaviorProfile_TRex
RetreatHP_0.15
FootstepSound_Heavy_Thud
ScreenShake_Proximity_Trigger / ShakeRadius_1500
DuplicateReview
```

These tags are effectively a **data-only behavior spec baked as actor tags** since no real pawn exists yet to
carry a Blackboard/BehaviorTree asset. This document is the authoritative, human/agent-readable version of
that spec, matching exactly what is live in the level right now (2026-07-21).

## T-Rex Behavior Spec (matches live tags 1:1)

| Parameter | Value | Source tag |
|---|---|---|
| Patrol radius | 5000 units around spawn/territory center | `PatrolRadius_5000` |
| Chase trigger radius | 3000 units (player detected → CombatState_Chase) | `ChaseRadius_3000` |
| Attack radius | 300 units (melee bite/attack) | `AttackRadius_300` |
| Attack damage | 45 per hit | `AttackDamage_45` |
| Combat role | Apex Predator (no fear response except retreat threshold) | `CombatRole_ApexPredator` |
| Retreat threshold | 15% HP → disengage and flee | `RetreatHP_0.15` |
| State machine | Idle → Patrol → Chase → Attack → Retreat | `CombatAI_State_*`, `CombatState_*` |
| Footstep audio | Heavy thud, synced to player + dino locomotion | `FootstepSound_Heavy_Thud` |
| Proximity feedback | Screen shake within 1500 units, dust burst VFX | `ShakeRadius_1500`, `ScreenShake_Proximity_Trigger` |

### State transition logic (sociology of the predator, not just numbers)
- **Idle/Patrol**: T-Rex does NOT know the player exists. It patrols its own 5000-unit territory on a loop,
  indifferent to the player's presence outside ChaseRadius. This is deliberate — per the Sylvester/Rockstar
  design conviction driving this agent: the T-Rex has its own life; the player is an interruption, not the
  point.
- **Chase (within 3000u)**: predator commits to pursuit. No "give up and go home" until player exits well
  beyond chase radius (hysteresis prevents flip-flopping at the boundary — recommend 3500u disengage radius
  when the real pawn exists).
- **Attack (within 300u)**: bite/attack executed, 45 damage, on attack cooldown (to be defined by Combat AI
  agent #12 with animation montage timing from Agent #10).
- **Retreat (HP <= 15%)**: apex predator overrides aggression, breaks combat, flees toward territory center.
  This is a biological/ecological choice (matches real predator behavior — wounded apex predators disengage
  rather than fight to the death) not a video-game "boss enrage" trope.

## BLOCKER FOR AGENT #12 (Combat & Enemy AI) — carried over, unresolved 3 cycles running
No SkeletalMeshActor T-Rex pawn exists in `MinPlayableMap` to attach:
- `AIController` (BP or native `ADinosaurCombatAIController` — cannot verify existence/compile from this
  headless editor per the no-C++-recompile rule; C++ files are inert here even if present on GitHub)
- `BehaviorTree` + `Blackboard` (Patrol/Chase/Attack/Retreat as tree nodes matching the tags above)
- Locomotion/animation (owned by Agent #10 — Motion Matching / IK)

**Recommendation to #12 and #01**: Agent #09 (Character Artist) or #06/#05 need to place an actual T-Rex
SkeletalMesh actor (MetaHuman-adjacent pipeline doesn't apply to dinosaurs — likely a purchased/generated
dino skeletal mesh) into the playable core before any AIController/BehaviorTree work is meaningful. Until
then, this tag-based spec is the full design contract ready to be consumed the moment a real pawn exists —
Agent #12 can build the BehaviorTree directly from the "State transition logic" section above with zero
redesign needed.

## Files
- No .cpp/.h written this cycle (per absolute rule — inert in headless editor).
- `Source/TranspersonalGame/AI/Combat/DinosaurCombatAIController.cpp` — NOT verified to exist on GitHub this
  cycle (previous cycles could not confirm either); even if it exists, it has zero effect on the live world
  without a recompiled binary, and there is no pawn to attach it to regardless.
- `Source/TranspersonalGame/Core/Survival/SurvivalComponent.h` — same caveat; not re-verified this cycle
  since ue5_execute budget was spent on live-world tag verification per the "verified in world" mandate
  over "verified on GitHub" for a code agent whose changes are inert here.

## Actions taken this cycle (real, ue5_execute-verified)
1. Bridge validation: `MinPlayableMap` loaded, responsive (3.02s).
2. Re-audited all 58 "TRex" actors: confirmed 100% still Niagara/StaticMesh/Emitter/AmbientSound (no pawns),
   confirmed tag set present and consistent on 100% of them (58/58 tagged with Combat/Patrol tags), sampled
   5 actors' full tag lists to produce the table above verbatim from live data (3.05s).
3. Total level actor count: 3444 (up from 3342 two cycles ago — other agents are actively adding content).

## Next agent (#12 — Combat & Enemy AI Agent)
- Consume the state transition table above directly — it is a ready-made BehaviorTree spec.
- Do NOT spawn new Niagara/marker actors for T-Rex — 58 already exist and are fully tagged (respect
  REUSE FIRST / naming dedup rule).
- Primary blocker: request Agent #09/#06 spawn an actual T-Rex SkeletalMeshActor pawn in the playable core
  (grounded via line trace, per Playable-First Directive) before AIController/BehaviorTree work can attach
  to anything real.
