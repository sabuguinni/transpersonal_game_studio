# Dinosaur AI Behavior — Data Schema & Design Notes
### Narrative & Dialogue Agent #15 — PROD_CYCLE_AUTO_20260719_005
### Handoff target: Combat & Enemy AI Agent #12

## Context & Constraint
This is a headless UE5 editor: the running binary is **pre-built and never recompiles**.
Per absolute rule `hugo_no_cpp_h_v2`, **no .cpp/.h files were written this cycle**.
All "AI programmer" work requested this cycle (`DinosaurBehaviorTree.cpp`, sight detection,
herd behavior, predator hunting) has instead been implemented as **live Actor Tag metadata**
on the real actors already in the world, via `ue5_execute` (Python/Remote Control). This data
is directly consumable by Blueprint Behavior Trees + AIPerception (the only AI stack that
actually executes in this environment) — Combat AI Agent #12 should build/wire the Behavior
Tree assets and Blueprints that *read* these tags, not new C++ classes.

## What Was Changed In The Live World (verified this cycle)
- Audited all actors in the playable core (X -3000..5000, Y -1000..5500): **360 dinosaur-named
  actors found**, of which the ones truly inside the core were tagged (269 newly tagged,
  0 already tagged — no duplicates created, per `hugo_naming_dedup_v2`).
- Added 3 metadata tags per actor: `AIDiet:<Predator|Herbivore>`, `AIRole:<...>`, `AISpecies:<...>`.
- Species breakdown tagged: TRex 149, Raptor 101, Triceratops 14, Stegosaurus 2, Brachiosaurus 3.
- Ran proximity clustering and tagged **herd/pack group IDs**:
  - Herbivores → `HerdGroup:N` (9 herds detected, sizes 1-6, radius 800 units)
  - Raptors → `PackGroup:N` (10 packs detected, sizes 1-35, radius 600 units — two large
    raptor concentrations of 35 and 33 near the hub form natural ambush packs)
- Verified via readback: 120 actors carry the full tag set (AIRole + herd/pack group).
- Saved the level after verification (single save at end of turn, per directive).
- **No actors moved, no meshes changed, no collision altered, no camera touched, no
  TranspersonalCharacter PLAYER0 touched.**

## Tag Schema (source of truth for Combat AI Agent #12)

| Tag | Values | Meaning |
|---|---|---|
| `AISpecies:X` | TRex, Raptor, Triceratops, Stegosaurus, Brachiosaurus | Species identity |
| `AIDiet:X` | Predator, Herbivore | Determines base disposition toward player |
| `AIRole:X` | Ambush, Stalker, Chaser, Flanker, Defensive, Fleeing | Behavior archetype (see below) |
| `HerdGroup:N` | integer | Herbivores that move/flee together |
| `PackGroup:N` | integer | Raptors that hunt together (flanking coordination) |

### Role definitions (for Behavior Tree design, not code)
- **Ambush** (TRex, default pose): idle/patrol state until player enters sight range, then
  transitions to Chaser.
- **Stalker** (TRex tagged `_alert`): already aware of player, closes distance slowly,
  keeping cover/vegetation between itself and player before committing to Chaser.
- **Chaser** (TRex tagged `_midstride`): actively pursuing, highest movement speed, breaks
  off pursuit beyond a leash-distance from spawn/territory anchor.
- **Flanker** (Raptor, always): pack members split — one member holds player's attention
  frontally while others in the same `PackGroup:N` path to flanking positions using
  NavMesh; requires the Combat AI Agent to implement a "claim flank slot" blackboard
  key so pack members don't stack the same angle.
- **Defensive** (Triceratops): flees by default; if cornered (player within a threat
  radius with no escape vector for N seconds) it turns and charges once, then resumes
  fleeing. This matches real ecology — triceratops defend when cornered, don't hunt.
- **Fleeing** (Stegosaurus, Brachiosaurus): pure flight response; on player line-of-sight
  within detection radius, moves away from player and pulls the rest of its `HerdGroup:N`
  with it (herd members without direct sight of the player follow the nearest fleeing
  herd-mate — simple flocking, no telepathy/mysticism, purely "saw herd-mate startle").

## Recommended Behavior Tree Structure (for #12 to build in Blueprint, not C++)
```
Root (Selector)
├─ Predator Branch (Selector, filtered by AIDiet:Predator)
│   ├─ Sequence: Sight Detection (AIPerception) → Role Lookup (read AIRole tag)
│   │   ├─ AIRole:Ambush  → Patrol/Idle until Perception stimulus → switch to Chaser
│   │   ├─ AIRole:Stalker → Move to concealment waypoint, maintain distance, watch
│   │   ├─ AIRole:Chaser  → Pursue player (leash check vs spawn anchor), Attack when in range
│   │   └─ AIRole:Flanker → Query PackGroup:N siblings' claimed flank slots, claim free slot, move, attack in sync
│   └─ Fallback: Return to spawn anchor (territory)
└─ Herbivore Branch (Selector, filtered by AIDiet:Herbivore)
    ├─ AIRole:Fleeing   → On sight of player OR herd-mate fleeing (same HerdGroup:N), move away
    └─ AIRole:Defensive → Flee by default; if cornered timer exceeds threshold, Charge once, then resume Fleeing
```

## Explicitly Out Of Scope / Not Created
- No `.cpp`/`.h` files (would be inert — never compiled by this headless binary).
- No telepathic/mystical detection — all detection must route through UE5 AIPerception
  (sight cone + line of sight trace), consistent with the anti-hallucination rule.
- No new duplicate actors — all data attached to existing actors per naming/dedup rule.

## Next Agent (Combat & Enemy AI Agent #12) — Concrete Next Steps
1. Build a Blueprint Behavior Tree + Blackboard reading the `AIRole`/`AIDiet`/`AISpecies`
   tags above (via `GetActorTags` / tag-driven Blackboard init on BeginPlay).
2. Add an `AIPerception` component (sight sense) to the base dinosaur Blueprint, configured
   per-species (TRex: long range/narrow cone; Raptor: medium range/wide cone for pack
   awareness; Herbivores: medium range, triggers Flee not Attack).
3. Implement pack flank-slot claiming for `PackGroup:N` Raptors using a shared Blackboard
   or a lightweight actor-tag "claimed slot" lock to avoid raptors stacking one angle.
4. Implement herd flocking for `HerdGroup:N` herbivores: on one member's Fleeing state
   activating, propagate a "startle" event to same-group members within a radius.
5. Respect territory/leash anchors (spawn position) for Chaser state so predators don't
   infinite-chase across biomes.

## Files
- This document: `Docs/Narrative/AI_Behavior_Data_Schema_v1.md`
- No other files modified. World state changes are live in `MinPlayableMap` (saved).
