# NPC Behavior Tree Configuration — Cycle 009
**Agent #11 — NPC Behavior Agent**
**Cycle:** PROD_CYCLE_AUTO_20260618_009

---

## Deployed Assets

### UE5 Map Actors (MinPlayableMap)

#### Behavior Zone Anchors (PointLight markers)
| Label | Location | Purpose |
|-------|----------|---------|
| BTZone_TRex_Patrol_001 | (3000, 2000, 200) | T-Rex patrol territory center |
| BTZone_TRex_Chase_001 | (3500, 2500, 200) | T-Rex aggro trigger zone |
| BTZone_Raptor_Ambush_001 | (-2000, 1500, 200) | Raptor pack ambush point A |
| BTZone_Raptor_Ambush_002 | (-2500, 2000, 200) | Raptor pack ambush point B |
| BTZone_Brachio_Graze_001 | (5000, -1000, 200) | Brachiosaurus peaceful grazing zone |

#### Patrol Waypoints (TargetPoint actors)
| Label | Location | Species |
|-------|----------|---------|
| WP_TRex_001 | (2500, 1500, 200) | T-Rex |
| WP_TRex_002 | (3500, 1500, 200) | T-Rex |
| WP_TRex_003 | (3500, 2500, 200) | T-Rex |
| WP_TRex_004 | (2500, 2500, 200) | T-Rex |
| WP_Raptor_001 | (-1500, 1000, 200) | Raptor Pack |
| WP_Raptor_002 | (-2500, 1000, 200) | Raptor Pack |
| WP_Raptor_003 | (-2500, 2000, 200) | Raptor Pack |
| WP_Raptor_004 | (-1500, 2000, 200) | Raptor Pack |

---

## Behavior Tree Specifications

### T-Rex (Tyrannosaurus Rex)
```
BT_TRex:
  ROOT
  ├── Selector (Priority)
  │   ├── Sequence: ATTACK
  │   │   ├── Condition: PlayerDistance < 300
  │   │   └── Task: MeleeAttack (damage=80, cooldown=2.5s)
  │   ├── Sequence: CHASE
  │   │   ├── Condition: PlayerDistance < 3000
  │   │   ├── Condition: HasLineOfSight == true
  │   │   └── Task: MoveToPlayer (speed=600, acceleration=200)
  │   ├── Sequence: INVESTIGATE
  │   │   ├── Condition: HeardNoise == true
  │   │   └── Task: MoveToNoiseSource (speed=300)
  │   └── Sequence: PATROL
  │       ├── Task: MoveToWaypoint (WP_TRex_001..004, loop)
  │       └── Task: Wait (duration=3.0s at each waypoint)

Perception:
  - SightRadius: 3000 units
  - HearingRadius: 5000 units
  - LoseSightRadius: 4000 units
  - PeripheralVisionAngle: 60°

Stats:
  - Health: 2000
  - MoveSpeed_Walk: 200
  - MoveSpeed_Chase: 600
  - AttackDamage: 80
  - AttackRange: 300
  - TerritoryRadius: 5000
```

### Raptor Pack (Velociraptor)
```
BT_Raptor:
  ROOT
  ├── Selector (Priority)
  │   ├── Sequence: PACK_ATTACK
  │   │   ├── Condition: PackSize >= 2
  │   │   ├── Condition: PlayerDistance < 200
  │   │   └── Task: CoordinatedAttack (flank_left, flank_right, direct_charge)
  │   ├── Sequence: SOLO_ATTACK
  │   │   ├── Condition: PackSize < 2
  │   │   ├── Condition: PlayerDistance < 150
  │   │   └── Task: MeleeAttack (damage=25, cooldown=0.8s)
  │   ├── Sequence: CHASE
  │   │   ├── Condition: PlayerDistance < 2000
  │   │   └── Task: MoveToPlayer (speed=700, acceleration=400)
  │   ├── Sequence: AMBUSH
  │   │   ├── Condition: PlayerInAmbushZone == true
  │   │   └── Task: WaitAndPounce (wait=2.0s, pounce_range=400)
  │   └── Sequence: PATROL
  │       └── Task: MoveToWaypoint (WP_Raptor_001..004, loop)

Perception:
  - SightRadius: 2000 units
  - HearingRadius: 3000 units
  - PeripheralVisionAngle: 90° (wider than T-Rex)

Stats:
  - Health: 300
  - MoveSpeed_Walk: 300
  - MoveSpeed_Chase: 700
  - AttackDamage: 25
  - AttackRange: 150
  - PackCoordination: true
```

### Brachiosaurus (Peaceful Herbivore)
```
BT_Brachio:
  ROOT
  ├── Selector (Priority)
  │   ├── Sequence: FLEE
  │   │   ├── Condition: ThreatDistance < 1500
  │   │   └── Task: FleeFromThreat (speed=400, duration=10s)
  │   ├── Sequence: GRAZE
  │   │   ├── Condition: IsHungry == true
  │   │   └── Task: GrazeAtLocation (BTZone_Brachio_Graze_001, duration=30s)
  │   └── Sequence: WANDER
  │       └── Task: WanderInRadius (center=BTZone_Brachio_Graze_001, radius=2000)

Stats:
  - Health: 5000
  - MoveSpeed_Walk: 150
  - MoveSpeed_Flee: 400
  - IsAggressive: false
  - FleeThreshold: 1500
```

---

## Voice Lines Generated

| File | URL | Duration | Content |
|------|-----|----------|---------|
| SurvivalNarrator_TRex.mp3 | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781788322125_SurvivalNarrator_TRex.mp3 | ~16s | T-Rex detection warning — lateral movement advice |
| SurvivalNarrator_Raptor.mp3 | https://thdlkizjbpwdndtggleb.supabase.co/storage/v1/object/public/game-assets/tts/1781788352486_SurvivalNarrator_Raptor.mp3 | ~18s | Raptor pack coordination warning — evasion tactics |

---

## Cumulative NPC Behavior Infrastructure (Cycles 006-009)

### Total Actors Deployed in MinPlayableMap
- Behavior Zone Anchors: 5 (BTZone_*)
- Patrol Waypoints: 8 (WP_*)
- Previous BT anchors from cycles 007-008: ~9 actors
- **Total NPC behavior infrastructure: ~22 actors**

### Behavior Systems Defined
1. T-Rex territorial patrol + chase + attack BT
2. Raptor pack coordinated ambush + flanking BT
3. Brachiosaurus peaceful grazing + flee BT
4. Perception system (sight/hearing radii per species)
5. Patrol waypoint circuits (T-Rex 4-point, Raptor 4-point)

---

## Handoff to Agent #12 — Combat & Enemy AI

Agent #12 should build on:
1. **Waypoints WP_TRex_001..004** — wire these to actual AI Controller patrol logic
2. **BTZone_TRex_Chase_001** — use as aggro radius trigger for combat state
3. **BTZone_Raptor_Ambush_001/002** — implement flanking logic using these as reference positions
4. **Voice lines** — integrate TRex/Raptor narrator lines into combat encounter triggers
5. **Behavior specs above** — implement as actual UE5 Behavior Tree assets (BT_TRex, BT_Raptor, BT_Brachio)

Priority for #12: Wire the patrol waypoints to actual dinosaur pawn AI Controllers so dinosaurs visibly move in the map.
