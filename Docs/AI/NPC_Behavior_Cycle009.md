# NPC Behavior Agent #11 — Cycle 009 Report

## Overview
This cycle focused on NPC behavior zones, survivor NPC placement, and voice line generation for in-world characters.

---

## Deliverables

### Voice Lines Generated
| Character | Line | URL |
|-----------|------|-----|
| Survivor_NPC | "Stay still. It cannot see you if you do not move. Breathe slow. Let it pass. The forest is patient — you must be too." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781713057942_Survivor_NPC.mp3 |
| Elder_NPC | "Run. Do not look back. The pack hunts together — if one sees you, they all see you. Split them up. Use the trees." | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781713069118_Elder_NPC.mp3 |

---

### NPC Behavior Zones (TriggerBox actors in MinPlayableMap)
| Label | Position | Purpose |
|-------|----------|---------|
| BehaviorZone_Stealth_001 | (-600, 400, 100) | NPCs enter stealth/crouch state |
| BehaviorZone_Danger_001 | (1200, -300, 100) | NPCs flee or hide |
| BehaviorZone_Safe_001 | (-1500, -800, 100) | NPCs relax, perform idle routines |
| BehaviorZone_Hunt_001 | (800, 1800, 100) | Predator active hunt zone — NPCs avoid |

### NPC Survivor Actors (SkeletalMeshActor in MinPlayableMap)
| Label | Position | Role |
|-------|----------|------|
| NPC_Survivor_001 | (-400, 200, 100) | Generic tribal survivor |
| NPC_Survivor_002 | (300, -600, 100) | Generic tribal survivor |
| NPC_Elder_001 | (-900, -400, 100) | Tribal elder — knowledge/quest giver |
| NPC_Scout_001 | (600, 900, 100) | Scout — patrols forest perimeter |

---

## NPC Behavior Design (for Blueprint/BT implementation)

### Daily Routine States
```
IDLE → PATROL → FORAGE → REST → IDLE (loop)
```

### Alert State Machine
```
IDLE
  └─ PlayerNear(500u) → ALERT
       ├─ DinoNear(1000u) → FLEE → HIDE
       └─ PlayerFriendly → GREET → DIALOGUE
```

### Behavior Zone Responses
- **BehaviorZone_Stealth_001**: NPCs crouch, reduce movement speed 50%, scan surroundings
- **BehaviorZone_Danger_001**: NPCs sprint to nearest safe zone, call alert to nearby NPCs
- **BehaviorZone_Safe_001**: NPCs perform idle animations (sit, eat, craft, talk)
- **BehaviorZone_Hunt_001**: NPCs avoid entirely, will not enter unless chased

---

## Waypoint Network (from previous cycles)
- T-Rex patrol: TRex_Waypoint_001–004 (2000u radius loop)
- Raptor pack patrol: Waypoint_Floresta_011–014 (forest biome)
- Raptor hunt zones: Zone_RaptorHunt_001–003

---

## Audio Integration
Voice lines are MP3 files hosted on Supabase. Audio Agent (#16) should:
1. Import these URLs into MetaSounds as dialogue triggers
2. Attach to NPC_Survivor_001 and NPC_Elder_001 actors
3. Trigger on PlayerNear(300u) overlap event

---

## Next Agent (#12 — Combat & Enemy AI)
Build on:
- **BehaviorZone_Danger_001** at (1200, -300, 100) — mark as active combat zone
- **BehaviorZone_Hunt_001** at (800, 1800, 100) — T-Rex active hunt area
- Raptor waypoints in forest biome for pack ambush logic
- NPC_Scout_001 should detect dinosaurs and trigger alert to other NPCs

Priority for Combat AI:
1. T-Rex aggro radius: 3000u detection, 300u attack
2. Raptor pack coordination: leader + 2 flankers
3. NPC flee behavior when combat zone is active
