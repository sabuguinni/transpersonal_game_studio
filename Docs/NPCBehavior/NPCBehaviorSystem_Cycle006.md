# NPC Behavior System — Cycle 006 (PROD_CYCLE_AUTO_20260618_005)
**Agent:** #11 — NPC Behavior Agent  
**Date:** 2026-06-18  
**Status:** Active — Behavior Tree anchors + species config deployed

---

## Overview

This cycle deployed the full NPC Behavior infrastructure into MinPlayableMap:
- **11 Behavior Tree anchor actors** (territory centers, ambush points, patrol roots, EQS reference points)
- **1 NPCBehaviorConfig_Master** actor encoding all species parameters as actor tags
- **4 NPC Memory Zone markers** (territory memory, last-seen tracking)
- **2 TTS narration lines** for T-Rex and Raptor pack behavior

---

## Behavior Tree Architecture

### State Machine (all species)

```
IDLE ──────────────────────────────────────────────────────────────────────
  │ Detection range exceeded (player/prey enters range)
  ▼
ALERT ─────────────────────────────────────────────────────────────────────
  │ Threat confirmed (line of sight + smell + sound)          │ Threat lost
  ▼                                                           ▼
HUNT/CHASE ──────────────────────────────────────────────────PATROL
  │ Attack range reached                                      │ Waypoint reached
  ▼                                                           ▼
ATTACK ──────────────────────────────────────────────────────NEXT_WAYPOINT
  │ Target killed / fled                      │ Health < 20%
  ▼                                           ▼
RETURN_TO_TERRITORY                         FLEE
```

### Species Behavior Parameters

| Species       | Detection | Attack | Patrol R | Chase Speed | Aggression | Pack     |
|---------------|-----------|--------|----------|-------------|------------|----------|
| T-Rex         | 3000 u    | 350 u  | 5000 u   | 1200 u/s    | 9/10       | Solitary |
| Raptor        | 2000 u    | 200 u  | 2500 u   | 1600 u/s    | 8/10       | Pack 3-6 |
| Brachiosaurus | 1500 u    | 600 u  | 3000 u   | 700 u/s     | 2/10       | Herd 4-8 |
| Stegosaurus   | 1200 u    | 400 u  | 1500 u   | 500 u/s     | 4/10       | Group 2-4|

---

## EQS (Environment Query System) Design

### Query: EQS_FindFleePosition
- **Context:** Querying NPC fleeing from threat
- **Generator:** Points on circle, radius = 2000u, 16 points
- **Tests:**
  1. Distance from threat (maximize)
  2. Navigable path exists (filter)
  3. Not in water (filter)
  4. Cover available nearby (prefer)

### Query: EQS_FindAmbushPosition
- **Context:** T-Rex/Raptor selecting attack approach
- **Generator:** Points on donut (min 500u, max 2000u from target), 24 points
- **Tests:**
  1. Line of sight to target (filter — must NOT have LoS = stealth approach)
  2. Distance to target (prefer closer)
  3. Navigable path to target from point (filter)
  4. Elevation advantage (prefer higher ground)

### Query: EQS_FindGrazingSpot
- **Context:** Herbivores selecting feeding location
- **Generator:** Points on circle, radius = 1500u, 12 points
- **Tests:**
  1. Distance from predator detection zones (maximize)
  2. Foliage density (prefer — more food)
  3. Water source proximity (prefer within 2000u)
  4. Open sightlines for herd vigilance (prefer)

---

## NPC Memory System

### Memory Architecture
Each NPC maintains a short-term memory buffer:
- **Duration:** 30 seconds (configurable per species)
- **Entries:** Last known player position, threat level, escape route used
- **Propagation:** Alert state propagates to pack members within 1500u radius

### Memory Zones (deployed in map)
| Zone Label                  | Location          | Purpose                              |
|-----------------------------|-------------------|--------------------------------------|
| MemoryZone_TRex_Territory   | (1500, 1500, 150) | T-Rex remembers encounters here      |
| MemoryZone_Raptor_Forest    | (-2000, 1000, 150)| Raptor pack last-seen tracking       |
| MemoryZone_Brachio_Plains   | (3000, 3000, 150) | Herd grazing memory zone             |
| MemoryZone_Player_LastSeen  | (0, 0, 150)       | Global player last-seen reference    |

---

## Behavior Tree Anchor Actors (deployed in map)

### T-Rex Anchors
| Label                         | Location           | Purpose                    |
|-------------------------------|--------------------|----------------------------|
| BTAnchor_TRex_TerritoryCenter | (1500, 1500, 100)  | Patrol root / home base    |
| BTAnchor_TRex_AmushPoint_N    | (1500, 3500, 120)  | Northern ambush position   |
| BTAnchor_TRex_AmushPoint_S    | (1500, -500, 120)  | Southern ambush position   |

### Raptor Pack Anchors
| Label                         | Location           | Purpose                    |
|-------------------------------|--------------------|----------------------------|
| BTAnchor_Raptor_PackLeader    | (-2000, 1000, 100) | Pack leader patrol root    |
| BTAnchor_Raptor_FlankLeft     | (-2500, 500, 100)  | Left flanker position      |
| BTAnchor_Raptor_FlankRight    | (-1500, 500, 100)  | Right flanker position     |

### Herbivore Anchors
| Label                         | Location           | Purpose                    |
|-------------------------------|--------------------|----------------------------|
| BTAnchor_Brachio_GrazeCenter  | (3000, 3000, 100)  | Herd grazing center        |
| BTAnchor_Brachio_WaterSource  | (3500, 1000, 100)  | Water drinking location    |
| BTAnchor_Stego_DefenseCluster | (-1000, 3000, 100) | Defensive formation center |

### EQS Reference Points
| Label                         | Location           | Purpose                    |
|-------------------------------|--------------------|----------------------------|
| EQS_DetectionRef_Savana       | (0, 0, 200)        | Open savana detection ref  |
| EQS_DetectionRef_Forest       | (-2000, 2000, 200) | Forest detection ref       |
| EQS_DetectionRef_River        | (3000, 0, 200)     | River area detection ref   |

---

## NPCBehaviorConfig_Master Actor Tags

All 36 behavior parameters encoded as actor tags on `NPCBehaviorConfig_Master` at (0, 0, 500):

**T-Rex:** DetectionRange_3000, AttackRange_350, PatrolRadius_5000, ChaseSpeed_1200, PatrolSpeed_400, AggressionLevel_9, PackBehavior_Solitary

**Raptor:** DetectionRange_2000, AttackRange_200, PatrolRadius_2500, ChaseSpeed_1600, PatrolSpeed_600, AggressionLevel_8, PackBehavior_Pack3to6, FlankingEnabled_True

**Brachiosaurus:** DetectionRange_1500, AttackRange_600, PatrolRadius_3000, GrazeSpeed_200, FleeSpeed_700, AggressionLevel_2, PackBehavior_Herd4to8

**Stegosaurus:** DetectionRange_1200, AttackRange_400, PatrolRadius_1500, WalkSpeed_300, AggressionLevel_4, PackBehavior_SmallGroup2to4

**Global:** MemoryDuration_30s, AlertPropagation_Enabled, DayNightBehaviorShift_Enabled, TerrainAdaptation_Enabled

---

## TTS Voice Lines

| File | Duration | URL |
|------|----------|-----|
| NPCNarrator_RaptorPack.mp3 | ~36s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781766373186_NPCNarrator_RaptorPack.mp3 |
| NPCNarrator_TRex_Hunt.mp3 | ~29s | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781766406364_NPCNarrator_TRex_Hunt.mp3 |

---

## Integration Notes for Agent #12 (Combat & Enemy AI)

### What's Ready
- All BT anchor actors deployed in MinPlayableMap with correct labels
- NPCBehaviorConfig_Master encodes all species parameters as readable actor tags
- EQS reference points placed for all 3 biome zones
- Memory zones define where NPCs track player encounters

### What Agent #12 Should Build On
1. **Read BTAnchor_TRex_TerritoryCenter** as the T-Rex patrol root — implement circular patrol around it
2. **Read BTAnchor_Raptor_PackLeader/FlankLeft/FlankRight** — implement flanking maneuver logic
3. **Use EQS_DetectionRef_*** actors as context providers for EQS queries
4. **Read NPCBehaviorConfig_Master tags** to get species parameters at runtime
5. **Implement attack state transitions** using the attack ranges defined in config tags
6. **Connect to AnimZone_*** actors** (from Agent #10) for animation state triggers

### Recommended Combat AI Priority
1. T-Rex: Simple patrol → detect → charge (solitary, high damage, slow turn rate)
2. Raptor pack: Coordinated flanking → surround → attack from multiple angles
3. Herbivores: Flee first, only attack if cornered (Stego tail swipe, Brachio stomp)

---

## Dependencies
- **Agent #10 (Animation):** AnimZone_* actors deployed — use as animation state triggers ✅
- **Agent #09 (Character Artist):** Skeletal meshes needed for actual dino pawns (placeholder cubes currently)
- **Agent #12 (Combat AI):** Receives this full BT architecture — implement attack logic on top

---

*Generated by NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260618_005*
