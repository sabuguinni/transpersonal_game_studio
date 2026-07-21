# NPC Behavior System — Agent #11 Production Cycle 011

## Overview
This document captures the NPC behavior architecture deployed across cycles 008–011 for the prehistoric survival game. All behavior zones are represented as PointLight actors in MinPlayableMap with color-coded threat levels.

---

## Behavior Zone Color Legend

| Color | Hex | Zone Type | Threat Level |
|-------|-----|-----------|--------------|
| 🔴 Red | `(1.0, 0.1, 0.1)` | T-Rex patrol territory | CRITICAL |
| 🟠 Amber | `(1.0, 0.6, 0.0)` | Raptor ambush zones | HIGH |
| 🟢 Green | `(0.1, 1.0, 0.2)` | NPC safe encounter points | SAFE |
| 🔵 Blue | `(0.2, 0.5, 1.0)` | Herbivore grazing zones | PASSIVE |
| 🟡 Yellow | `(1.0, 1.0, 0.0)` | Daily routine anchors | NEUTRAL |
| 🟣 Purple | `(0.6, 0.0, 1.0)` | Behavior Tree trigger zones | SYSTEM |

---

## Deployed Actors (Cycle 011)

### T-Rex Patrol Waypoints (5 actors)
Patrol loop covering the central savanna. T-Rex AI follows these in sequence.

```
TRex_Patrol_WP_01  →  (2000, 1000, 100)
TRex_Patrol_WP_02  →  (3500, 2500, 100)
TRex_Patrol_WP_03  →  (2800, 4000, 100)
TRex_Patrol_WP_04  →  (1200, 3800, 100)
TRex_Patrol_WP_05  →  (800, 2200, 100)
```

**Patrol Behavior:**
- Patrol radius: 5000 units from center
- Chase trigger: Player within 3000 units
- Attack trigger: Player within 300 units
- Detection: Line-of-sight + sound radius (1500 units)

### NPC Encounter Points (3 actors)
Safe zones where survivor NPCs can be found. Player can receive information/quests here.

```
NPC_Encounter_RiverBank    →  (-1500, 2000, 100)
NPC_Encounter_CaveEntrance →  (4500, -500, 200)
NPC_Encounter_Hilltop      →  (-800, -2500, 350)
```

**NPC Behavior at Encounter Points:**
- NPCs idle/forage when player is absent
- NPCs react (turn, acknowledge) when player enters 400-unit radius
- NPCs flee if dinosaur enters 1500-unit radius
- NPCs have memory: remember player interactions for session duration

### Raptor Ambush Zones (3 actors)
High-density vegetation areas where raptor packs set up flanking ambushes.

```
Raptor_AmbushZone_Forest_01  →  (-2000, 1500, 100)
Raptor_AmbushZone_Ravine_01  →  (1800, -1800, 80)
Raptor_AmbushZone_Swamp_01   →  (-3000, -1000, 50)
```

**Raptor Pack Behavior:**
- Pack size: 2–5 raptors per zone
- Flanking: 2 raptors circle while 1 feints from front
- Memory: Raptors remember player location for 45 seconds after losing sight
- Retreat: Pack retreats if 2+ members are injured

### Herbivore Grazing Zones (2 actors)
Peaceful areas where large herbivores (Brachiosaurus, Stegosaurus) graze.

```
Herbivore_GrazeZone_Plains_01  →  (3000, -2000, 100)
Herbivore_GrazeZone_River_01   →  (-500, 3500, 80)
```

**Herbivore Behavior:**
- Graze passively, move slowly between food sources
- Stampede if startled (player runs within 200 units)
- Stampede direction: Away from threat, can injure player
- T-Rex proximity (within 2000 units) triggers herd movement

---

## NPC Dialogue Audio Assets

### HunterElder Voice Line
- **File:** `SurvivorNPC_HunterElder.mp3`
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781810174214_SurvivorNPC_HunterElder.mp3`
- **Content:** T-Rex avoidance advice — freeze behavior, treeline escape
- **Trigger:** Player enters `NPC_Encounter_CaveEntrance` zone

### ScoutWoman Voice Line
- **File:** `SurvivorNPC_ScoutWoman.mp3`
- **URL:** `https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781810195595_SurvivorNPC_ScoutWoman.mp3`
- **Content:** Raptor pack behavior — circling, fire as deterrent
- **Trigger:** Player enters `NPC_Encounter_RiverBank` zone

---

## Behavior Tree Architecture (Design)

### T-Rex BT Structure
```
BT_TRex_Root
├── Selector: CombatOrPatrol
│   ├── Sequence: CombatBranch [condition: PlayerInRange(3000)]
│   │   ├── Task: ChasePlayer
│   │   ├── Sequence: AttackBranch [condition: PlayerInRange(300)]
│   │   │   └── Task: AttackPlayer (stomp + bite)
│   └── Sequence: PatrolBranch
│       ├── Task: MoveToNextWaypoint
│       └── Task: WaitAtWaypoint (5-15s random)
```

### Raptor Pack BT Structure
```
BT_Raptor_Root
├── Selector: PackBehavior
│   ├── Sequence: FlankAttack [condition: PackSize >= 2, PlayerDetected]
│   │   ├── Task: AssignRoles (flanker/feint)
│   │   ├── Task: CircleTarget
│   │   └── Task: CoordinatedAttack
│   ├── Sequence: SoloHunt [condition: PackSize == 1, PlayerDetected]
│   │   └── Task: DirectChase
│   └── Sequence: Patrol
│       └── Task: PatrolAmbushZone
```

### Survivor NPC BT Structure
```
BT_SurvivorNPC_Root
├── Selector: NPCBehavior
│   ├── Sequence: FleeFromDanger [condition: DinoInRadius(1500)]
│   │   └── Task: FleeToSafePoint
│   ├── Sequence: InteractWithPlayer [condition: PlayerInRadius(400)]
│   │   ├── Task: FacePlayer
│   │   ├── Task: PlayDialogue
│   │   └── Task: OfferQuest (if applicable)
│   └── Sequence: DailyRoutine
│       ├── Task: Forage (morning)
│       ├── Task: Rest (midday)
│       └── Task: ReturnToShelter (evening)
```

---

## NPC Memory System Design

### Short-Term Memory (Session)
- Last known player position
- Last seen dinosaur positions
- Dialogue lines already spoken (no repeat)
- Resources gathered this session

### Behavioral Flags
- `bHasMetPlayer` — changes greeting dialogue
- `bIsInjured` — reduces movement speed, changes dialogue
- `bHasWarnedAboutTRex` — prevents duplicate warnings
- `bKnowsPlayerName` — personalizes dialogue after first meeting

---

## Integration Notes for Agent #12 (Combat AI)

The behavior zones deployed here define **territory boundaries** for Combat AI:
- T-Rex patrol waypoints define the aggression zone boundary
- Raptor ambush zones define flanking initiation points
- Herbivore zones define stampede trigger areas

Agent #12 should use these zone actors as **spatial anchors** for combat state transitions.

---

## Cumulative Behavior Zone Count (Cycles 008–011)

| Cycle | Actors Added | Zone Types |
|-------|-------------|------------|
| 008 | 5 | BTZone triggers (purple) |
| 009 | 5 | Behavior zone anchors (orange) |
| 010 | 8 | Daily routine anchors (yellow) |
| 011 | 10 | Patrol WPs (red) + Encounters (green) + Ambush (amber) + Graze (blue) |
| **Total** | **28** | **6 zone types** |

---

*Generated by NPC Behavior Agent #11 — PROD_CYCLE_AUTO_20260618_011*
