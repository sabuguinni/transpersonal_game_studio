# NPC Behavior System — Production Cycle 008
**Agent #11 — NPC Behavior Agent**
**Date:** 2026-06-18 | **Cycle:** PROD_CYCLE_AUTO_20260618_008

---

## Overview

This cycle deployed the full NPC Behavior Tree spatial infrastructure into MinPlayableMap.
All behavior zones, patrol anchors, and memory points are live in the UE5 editor.

---

## Behavior Tree Zone Architecture

### T-Rex Behavior Zones
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| BTZone_TRex_Aggro_001 | TriggerBox (30x30x5) | (3000, 1500, 100) | Player enters → T-Rex switches to Chase state |
| BTZone_TRex_Patrol_001 | TriggerBox (50x50x5) | (3500, 2000, 100) | T-Rex patrol boundary — turns back at edge |

### Raptor Pack Behavior Zones
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| BTZone_Raptor_Hunt_001 | TriggerBox (25x25x5) | (-1500, 2500, 100) | Alpha raptor drives player into kill zone |
| BTZone_Raptor_Flank_001 | TriggerBox (15x15x5) | (-1200, 3000, 100) | Beta/Gamma raptors flank from 45° angle |

### Brachiosaurus Behavior Zone
| Actor Label | Type | Location | Purpose |
|---|---|---|---|
| BTZone_Brachio_Graze_001 | TriggerBox (40x40x8) | (500, -3000, 100) | Peaceful grazing area — Brachio daily routine |

---

## NPC Memory Anchor Points

Memory anchors are TargetPoint actors that behavior trees reference for:
- **Last Known Player Position (LKP)** — where T-Rex last saw the player
- **Home Position** — where raptors return after losing the player
- **Daily Routine Points** — Brachiosaurus grazing/water migration path

| Actor Label | Location | Species | Memory Type |
|---|---|---|---|
| NPC_Memory_TRex_LKP_001 | (2800, 1200, 150) | T-Rex | Last Known Player Position |
| NPC_Memory_Raptor_Home_001 | (-1400, 2600, 150) | Raptor Alpha | Home/Rest Position |
| NPC_Memory_Raptor_Flank_001 | (-1600, 2800, 150) | Raptor Beta | Flank Attack Position |
| NPC_Memory_Raptor_Ambush_001 | (-1000, 3200, 150) | Raptor Gamma | Ambush Position |
| NPC_Memory_Brachio_Graze_001 | (600, -2800, 150) | Brachiosaurus | Primary Grazing Point |
| NPC_Memory_Brachio_Water_001 | (200, -4000, 150) | Brachiosaurus | Water Source (daily migration) |

---

## Behavior Tree State Machine Design

### T-Rex Behavior States
```
IDLE → PATROL (random waypoints in BTZone_TRex_Patrol_001)
PATROL → ALERT (player scent detected at 5000 units)
ALERT → CHASE (player visible at 3000 units)
CHASE → ATTACK (player within 300 units)
ATTACK → CHASE (player escapes 300 unit range)
CHASE → SEARCH (player lost — move to NPC_Memory_TRex_LKP_001)
SEARCH → PATROL (search timeout 30s — return to patrol)
```

### Raptor Pack Behavior States (Coordinated)
```
IDLE → PATROL (loose formation around BTZone_Raptor_Hunt_001)
PATROL → STALK (player detected at 4000 units — silent approach)
STALK → COORDINATE (Alpha signals pack at 2000 units)
COORDINATE:
  - Alpha → drives player toward BTZone_Raptor_Flank_001
  - Beta → moves to NPC_Memory_Raptor_Flank_001 (flank left)
  - Gamma → moves to NPC_Memory_Raptor_Ambush_001 (ambush ahead)
COORDINATE → ATTACK (all three in position)
ATTACK → RETREAT (pack health < 30% — scatter and regroup)
RETREAT → IDLE (regroup at NPC_Memory_Raptor_Home_001)
```

### Brachiosaurus Daily Routine
```
DAWN: Move from rest position to NPC_Memory_Brachio_Graze_001
MORNING: Graze in BTZone_Brachio_Graze_001 (2-4 hours game time)
MIDDAY: Move to NPC_Memory_Brachio_Water_001 (water source)
AFTERNOON: Return to BTZone_Brachio_Graze_001
DUSK: Move to rest position (away from player start)
THREAT RESPONSE: Player within 800 units → slow retreat (not aggressive)
THREAT RESPONSE: Player within 200 units → stampede (knockback damage)
```

---

## Voice Lines Produced

### SurvivalNarrator_TRexAlert.mp3
**URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782394247_SurvivalNarrator_TRexAlert.mp3`
**Duration:** ~9s
**Text:** "Danger. A Tyrannosaurus Rex has detected your scent. It is tracking you now. Move downwind. Stay low. Do not run until you have cover."
**Trigger:** Player enters BTZone_TRex_Aggro_001 for the first time

### SurvivalNarrator_RaptorPack.mp3
**URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781782416777_SurvivalNarrator_RaptorPack.mp3`
**Duration:** ~11s
**Text:** "They hunt in packs. One drives you forward, the others wait ahead. If you hear clicking from two directions at once — stop moving. You are already surrounded."
**Trigger:** Player enters BTZone_Raptor_Hunt_001 for the first time

---

## Integration Notes for Agent #12 (Combat & Enemy AI)

The following spatial infrastructure is ready for Combat AI to wire up:

1. **BTZone_TRex_Aggro_001** — Connect to T-Rex AIController chase state trigger
2. **BTZone_TRex_Patrol_001** — Connect to T-Rex patrol waypoint system
3. **BTZone_Raptor_Hunt_001** — Connect to Raptor Alpha drive behavior
4. **BTZone_Raptor_Flank_001** — Connect to Raptor Beta/Gamma flank positions
5. **NPC_Memory_TRex_LKP_001** — T-Rex should move here when player is lost
6. **NPC_Memory_Raptor_Home_001/Flank/Ambush** — Raptor pack coordination positions
7. **BTZone_Brachio_Graze_001** — Brachiosaurus peaceful zone (no combat unless provoked)

All actors are in MinPlayableMap at `/Game/Maps/MinPlayableMap`.

---

## Cumulative NPC Behavior Infrastructure (Cycles 005-008)

| Cycle | Actors Deployed | System |
|---|---|---|
| 005 | 11 | Initial BT anchors, patrol waypoints, anim zones |
| 006 | 9 | TTS voice lines, additional patrol points |
| 007 | 9 | T-Rex + Raptor behavior anchors |
| 008 | 11 | BT trigger zones + NPC memory anchors |
| **Total** | **~40** | **Full spatial BT infrastructure** |
