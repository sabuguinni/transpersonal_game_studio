# Combat & Enemy AI System — Agent #12 Specification v2
## Transpersonal Game Studio — Prehistoric Survival Game

---

## Overview
This document defines the Combat AI system for all dinosaur and NPC enemy encounters.
All AI runs via UE5 Behavior Trees, Blackboards, and EQS — no custom C++ required for this phase.
All configuration is applied via actor tags and Blueprint-accessible data.

---

## Combat State Machine

Each dinosaur AI operates through 5 states:

```
IDLE → PATROL → ALERT → CHASE → ATTACK → RETREAT
         ↑                              ↓
         └──────────── PATROL ←─────────┘
```

### State Transitions
| From | To | Trigger |
|------|----|---------|
| IDLE | PATROL | Timer (30-120s random) |
| PATROL | ALERT | Player detected within sight/hearing radius |
| ALERT | CHASE | Player confirmed within 2x detection radius |
| CHASE | ATTACK | Player within melee range |
| ATTACK | RETREAT | Health < 20% OR player out of range for 10s |
| RETREAT | PATROL | Distance > 3000 units from last known player position |

---

## Dinosaur Combat Profiles

### T-Rex (Tyrannosaurus Rex)
- **Behavior Tree**: `/Game/AI/Dinosaurs/TRex/BT_TRex`
- **Blackboard**: `/Game/AI/Dinosaurs/TRex/BB_TRex`
- **Sight Radius**: 3000 units
- **Hearing Radius**: 1500 units
- **Attack Damage**: 80 HP per hit
- **Attack Speed**: 1 hit per 2.5 seconds
- **Move Speed**: Patrol=300, Chase=600, Attack=700
- **Patrol Route**: WP_TRex_Patrol_001 → 005 (river valley loop)
- **Special Behavior**: Circles back after missed charge. Does NOT stop on first miss.
- **Actor Tags**: `DinoAI, Predator, TRex, PatrolActive, SightRadius_3000, HearingRadius_1500, AttackDamage_80`

### Velociraptor (Pack Hunter)
- **Behavior Tree**: `/Game/AI/Dinosaurs/Raptor/BT_Raptor`
- **Blackboard**: `/Game/AI/Dinosaurs/Raptor/BB_Raptor`
- **Sight Radius**: 2000 units
- **Hearing Radius**: 800 units
- **Attack Damage**: 25 HP per hit
- **Attack Speed**: 1 hit per 0.8 seconds
- **Move Speed**: Patrol=400, Chase=900, Attack=1000
- **Pack Broadcast Radius**: 2000 units (alert all PackHunter-tagged actors)
- **Ambush Positions**: RaptorAmbush_Flank_Left_001, RaptorAmbush_Flank_Right_001, RaptorAmbush_Center_001
- **Special Behavior**: Alpha detects player → broadcasts to pack → flanking triangle formation
- **Actor Tags**: `DinoAI, Predator, Raptor, PackHunter, SightRadius_2000, HearingRadius_800, AttackDamage_25, PackBroadcast_2000`

### Triceratops (Territorial Herbivore)
- **Sight Radius**: 1500 units
- **Hearing Radius**: 600 units
- **Attack Damage**: 60 HP per hit (charge)
- **Behavior**: Passive until player enters 800-unit territory radius, then charges
- **Actor Tags**: `DinoAI, Herbivore, Triceratops, Territorial, SightRadius_1500, AttackDamage_60`

### Brachiosaurus (Passive Herbivore)
- **Sight Radius**: 2500 units (panoramic)
- **Hearing Radius**: 1000 units
- **Attack Damage**: 40 HP (stomp — accidental)
- **Behavior**: Never aggresses. Flees from T-Rex. Stomps player if player is underfoot.
- **Actor Tags**: `DinoAI, Herbivore, Brachiosaurus, Passive, SightRadius_2500, AttackDamage_40`

---

## Combat Encounter Zones

### Raptor Ambush Triangle (Near Player Spawn)
```
RaptorAmbush_Flank_Left_001   (-800, -600, 100)
RaptorAmbush_Flank_Right_001  (-800,  600, 100)
RaptorAmbush_Center_001       (-1200,   0, 100)
```
Formation: Triangle with player at apex. Center raptor charges first, flankers close in.

### T-Rex Detection Perimeter
```
TRex_DetectionPerimeter_N  (-3000,    0, 100)
TRex_DetectionPerimeter_S  ( 3000,    0, 100)
TRex_DetectionPerimeter_E  (    0, 3000, 100)
TRex_DetectionPerimeter_W  (    0,-3000, 100)
```
Radius: 3000 units from T-Rex home position.

### T-Rex Patrol Route
```
WP_TRex_Patrol_001  (-3000,    0, 0)
WP_TRex_Patrol_002  (-1500,  500, 0)
WP_TRex_Patrol_003  (    0, 1000, 0)
WP_TRex_Patrol_004  ( 1500,  500, 0)
WP_TRex_Patrol_005  ( 3000,    0, 0)
```
Loop: 001 → 002 → 003 → 004 → 005 → 001 (river valley patrol)

---

## Combat Zones — Visual Markers
Red point lights (CombatZone_Raptor_001/002/003) mark active combat trigger areas.
These are editor-only helpers — hidden at runtime.

---

## Player Damage Reference
| Source | Damage | Frequency | Notes |
|--------|--------|-----------|-------|
| T-Rex bite | 80 HP | 1 per 2.5s | Instant kill if health < 80 |
| Raptor slash | 25 HP | 1 per 0.8s | Pack of 3 = 75 HP/0.8s |
| Triceratops charge | 60 HP | 1 per 3s | Knockback effect |
| Brachiosaurus stomp | 40 HP | Accidental | Only if player underfoot |

Player max health: 100 HP (TranspersonalCharacter)

---

## Audio Assets
| File | Character | URL |
|------|-----------|-----|
| NPC_Tracker_Combat.mp3 | NPC Tracker — raptor pack warning | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783017329917_NPC_Tracker_Combat.mp3 |
| NPC_Survivor_TRex.mp3 | NPC Survivor — T-Rex charge account | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783017383952_NPC_Survivor_TRex.mp3 |
| NPC_Hunter_Savana.mp3 | NPC Hunter — raptor pack intel | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783017239902_NPC_Hunter_Savana.mp3 |
| NPC_Elder_Tribe.mp3 | NPC Elder — T-Rex patrol route | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1783017244404_NPC_Elder_Tribe.mp3 |

---

## AI Directories Created
```
/Game/AI/
├── Dinosaurs/
│   ├── TRex/
│   │   ├── BB_TRex (Blackboard)
│   │   └── BT_TRex (Behavior Tree)
│   └── Raptor/
│       ├── BB_Raptor (Blackboard)
│       └── BT_Raptor (Behavior Tree)
└── NPCBehavior/
```

---

## Next Steps for Agent #13 (Crowd & Traffic Simulation)
1. Use `PackHunter`-tagged actors for pack coordination in Mass AI
2. Reference `DinoAI`-tagged actors for crowd simulation exclusion (individual AI, not crowd)
3. Herbivore herds (Triceratops, Brachiosaurus) CAN use Mass AI for background herd movement
4. NavMesh is confirmed present — Mass AI zones can be placed within NavMesh bounds
5. Patrol waypoints (WP_TRex_Patrol_*, WP_Raptor_Patrol_*) are TargetPoint actors with tags

---

*Generated by Agent #12 — Combat & Enemy AI Agent*
*Cycle: PROD_CYCLE_AUTO_20260702_012*
