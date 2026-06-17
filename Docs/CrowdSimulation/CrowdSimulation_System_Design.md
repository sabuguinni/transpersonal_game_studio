# Crowd & Traffic Simulation System — Agent #13
## Transpersonal Game Studio — Prehistoric Survival Game

---

## Overview

The crowd simulation system manages up to 50,000 simultaneous agents using UE5 Mass AI framework.
This document defines the behavioral architecture, LOD strategy, and integration points with
Combat AI (#12) and Quest/Mission systems (#14).

---

## System Architecture

### 1. Agent Categories

#### Herbivore Herds
- **Brachiosaurus Herd** — 3-6 individuals, slow-moving, grazes at `Herd_Brachio_Grazing_001`
- **Triceratops Herd** — 4-8 individuals, defensive formation, grazes at `Herd_Trike_Grazing_001`
- **Parasaurolophus Flock** — 8-15 individuals, fast scatter, grazes at `Herd_Parasaur_Grazing_001`
- **Stegosaurus Herd** — 3-5 individuals, slow, defensive tail-swing, at `Herd_Stego_Grazing_001`

#### Tribal NPCs (Settlement)
- **Hunters** — Patrol `Crowd_Hunting_Prep_Zone_001` → leave camp → return with food
- **Gatherers** — Move between `Crowd_WaterSource_Zone_001` and `Crowd_FoodStorage_Zone_001`
- **Crafters** — Stationary at `Crowd_Crafting_Zone_001`, tool-making animations
- **Sentries** — Rotate between `Crowd_Lookout_Post_001` positions, scan for predators
- **Elders** — Stationary near `Crowd_TribalFire_Center_001`, interaction triggers quest dialogue

#### Ambient Wildlife (Background)
- Small birds, insects, fish — pure visual LOD, no pathfinding
- Pterosaurs — aerial patrol paths, scatter on loud noise events

---

## 2. Behavioral State Machine

### Herbivore States
```
GRAZING → ALERT → FLEE → STAMPEDE → SCATTER → CALM → GRAZING
```

**GRAZING**: Agent wanders within 800-unit radius of grazing anchor point
- Speed: 80-120 cm/s
- Animation: Walk cycle + head-down eating

**ALERT**: Triggered when predator enters detection radius (1200 units)
- Speed: 0 (frozen, head-up, scanning)
- Duration: 2-4 seconds before FLEE or return to GRAZING

**FLEE**: Directed movement away from threat vector
- Speed: 400-600 cm/s
- Target: Nearest `Stampede_ScatterPoint_*` in opposite direction from threat

**STAMPEDE**: Mass simultaneous FLEE — triggered when 3+ herd members enter FLEE state
- All herd members switch to FLEE simultaneously
- Collision avoidance disabled (stampede overrides — agents push through obstacles)
- Player can be knocked down if in stampede path (damage: 15 HP)

**SCATTER**: Herd disperses to individual scatter points after leaving threat zone
- Duration: 30-60 seconds
- Agents gradually return to CALM

**CALM**: Reduced alertness, slow return to grazing anchor
- Duration: 60-120 seconds before full GRAZING resumes

### Tribal NPC States
```
SLEEP → MORNING_ROUTINE → WORK → MIDDAY_REST → WORK → EVENING_GATHERING → SLEEP
```

**Daily Schedule** (in-game time):
- 06:00-08:00: Wake, gather at campfire (`Crowd_TribalFire_Center_001`)
- 08:00-12:00: Work assignments (hunt, gather, craft, sentry)
- 12:00-14:00: Rest near food storage
- 14:00-18:00: Work assignments resume
- 18:00-21:00: Evening gathering at campfire — storytelling, social
- 21:00-06:00: Sleep in shelters

**Emergency Override**: Any NPC within 2000 units of active predator → PANIC state
- Drop current task
- Run to nearest shelter or sentry post
- Alert other NPCs (chain reaction within 500 units)

---

## 3. LOD Strategy

### LOD Tier Definitions

| Tier | Distance | Max Agents | Animation | Collision | Pathfinding |
|------|----------|------------|-----------|-----------|-------------|
| HIGH | 0-1200u  | 20         | Full skeletal + IK | Full capsule | NavMesh full |
| MED  | 1200-2500u | 100      | Simplified (no IK) | No collision | NavMesh simplified |
| LOW  | 2500-5000u | 500      | Static mesh swap | None | Straight-line only |
| CULL | >5000u   | N/A        | Hidden | None | None |

### LOD Zone Markers in MinPlayableMap
- `CrowdLOD_High_Zone_001` — Green light, radius 1200u, centered on PlayerStart
- `CrowdLOD_Med_Zone_001` — Yellow light, radius 2500u
- `CrowdLOD_Low_Zone_001` — Orange light, radius 5000u

### Performance Targets
- HIGH tier: 60 FPS minimum (PC), 30 FPS (console)
- MED tier: No frame budget impact (simplified update tick: 10Hz vs HIGH 60Hz)
- LOW tier: Static mesh instancing via HISM — zero animation cost
- Total crowd budget: 2ms per frame on target hardware

---

## 4. Stampede System

### Trigger Conditions
1. T-Rex enters `Stampede_TriggerZone_TRex_001` (overlaps AggroZone at 2000, 1500)
2. Raptor pack enters `Stampede_TriggerZone_Raptor_001` (at -1500, 2000)
3. Player fires weapon within 500 units of herd
4. Loud environmental event (volcanic eruption, thunder)

### Scatter Destinations
- `Stampede_ScatterPoint_NW_001` — (-3000, -2000) — Primary Brachio escape route
- `Stampede_ScatterPoint_W_001` — (-3500, 0) — Trike herd escape
- `Stampede_ScatterPoint_S_001` — (0, -3500) — Parasaur flock scatter
- `Stampede_ScatterPoint_SW_001` — (-1000, -3000) — Mixed herd convergence

### Player Interaction
- Player caught in stampede path: 15 HP damage per second, knockback force
- Player can redirect stampede by standing in path (high risk, high reward)
- Stampede can be used as weapon against predators (T-Rex caught in stampede: 50 HP)

---

## 5. Integration Points

### From Agent #12 (Combat AI)
- `Combat_TRex_AggroZone_001` → triggers herbivore ALERT state within 1500 units
- `Combat_Raptor_AmbushZone_001` → triggers tribal NPC PANIC state
- `VisionCone_TRex_001` (radius 1500) → crowd awareness boundary
- `Combat_DeathZone_TRex_001` → agents within zone are removed from simulation

### To Agent #14 (Quest & Mission Designer)
- Stampede events can be quest triggers: "Survive the stampede"
- Tribal NPC daily routines expose quest interaction windows
- Herd migration patterns define exploration quest objectives
- Crowd panic events can reveal hidden areas (NPCs flee to previously unknown locations)

### To Agent #11 (NPC Behavior)
- Tribal NPC crowd states feed into individual NPC behavior trees
- Crowd PANIC overrides individual NPC behavior tree priorities
- Sentry NPCs at `Crowd_Lookout_Post_001` share detection data with crowd system

---

## 6. Mass AI Configuration (UE5 Mass Framework)

### Entity Processors (execution order)
1. `CrowdLODProcessor` — Update LOD tier per agent each frame
2. `CrowdThreatDetectionProcessor` — Check proximity to predator zones
3. `CrowdStateTransitionProcessor` — Apply state machine transitions
4. `CrowdMovementProcessor` — Execute movement based on current state
5. `CrowdAnimationProcessor` — Select animation based on state + LOD tier

### Mass Traits per Species
```
BrachiosaurusHerdTrait:
  - DetectionRadius: 1200
  - FleeSpeed: 350
  - StampedeThreshold: 2  # 2 herd members fleeing triggers stampede
  - GrazingRadius: 800

TriceratopsHerdTrait:
  - DetectionRadius: 900
  - FleeSpeed: 450
  - DefenseFormation: true  # Form circle when threatened
  - ChargeAttack: true  # Can charge predators if cornered

ParasaurolophusFlockTrait:
  - DetectionRadius: 1500  # Most alert species
  - FleeSpeed: 650
  - AlarmCall: true  # Vocalizes to alert other species
  - FlockRadius: 400  # Tight flock formation
```

---

## 7. Actors Created in MinPlayableMap

### Herd Grazing Anchors (Green lights)
- `Herd_Brachio_Grazing_001` — (-2000, -1500, 100)
- `Herd_Trike_Grazing_001` — (-2500, 500, 100)
- `Herd_Parasaur_Grazing_001` — (500, -2800, 100)
- `Herd_Stego_Grazing_001` — (-1800, 2500, 100)

### Stampede Scatter Points (Yellow lights)
- `Stampede_ScatterPoint_NW_001` — (-3000, -2000, 100)
- `Stampede_ScatterPoint_W_001` — (-3500, 0, 100)
- `Stampede_ScatterPoint_S_001` — (0, -3500, 100)
- `Stampede_ScatterPoint_SW_001` — (-1000, -3000, 100)

### Stampede Trigger Zones (TriggerBoxes)
- `Stampede_TriggerZone_TRex_001` — (2000, 1500, 50) — overlaps TRex AggroZone
- `Stampede_TriggerZone_Raptor_001` — (-1500, 2000, 50) — overlaps Raptor AmbushZone

### Tribal Activity Zones (Colored lights)
- `Crowd_TribalFire_Center_001` — (0, 0, 100) — Orange
- `Crowd_Crafting_Zone_001` — (300, 100, 100) — Brown
- `Crowd_Hunting_Prep_Zone_001` — (-300, 200, 100) — Red
- `Crowd_WaterSource_Zone_001` — (100, -400, 100) — Blue
- `Crowd_Lookout_Post_001` — (-200, -300, 100) — White
- `Crowd_FoodStorage_Zone_001` — (400, -200, 100) — Green

### LOD Zone Markers (Altitude-stacked lights)
- `CrowdLOD_High_Zone_001` — (0, 0, 300) — Green, radius 1200
- `CrowdLOD_Med_Zone_001` — (0, 0, 400) — Yellow, radius 2500
- `CrowdLOD_Low_Zone_001` — (0, 0, 500) — Orange, radius 5000

---

## 8. Next Steps for Agent #14 (Quest & Mission Designer)

1. **Stampede Quest**: "The Great Migration" — player must guide a Parasaur flock safely past raptor territory
2. **Tribal Integration**: NPC daily routines at activity zones expose quest givers at specific times
3. **Crowd Witness Events**: Stampede triggered by T-Rex = cinematic moment that unlocks story beat
4. **Survival Challenge**: Player must reach `Crowd_WaterSource_Zone_001` while avoiding predator zones
5. **Herd Escort**: Protect Brachio herd from `Herd_Brachio_Grazing_001` to new grazing area

---

*Generated by Agent #13 — Crowd & Traffic Simulation*
*Cycle: PROD_CYCLE_AUTO_20260617_005*
