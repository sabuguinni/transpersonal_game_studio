# Crowd & Traffic Simulation System — Agent #13 Specification v1
# Transpersonal Game Studio — Prehistoric Survival Game
# Generated: PROD_CYCLE_AUTO_20260702_012

---

## Overview

This document defines the crowd simulation architecture for the prehistoric survival game.
The system manages up to 50,000 simultaneous agents using UE5 Mass AI framework,
covering herbivore herds, migration corridors, and environmental crowd density.

---

## Deployed Actors (MinPlayableMap)

### Triceratops Herd — `Trike_Savana_001` through `Trike_Savana_006`
- **Formation**: Natural grazing spread (2100–2800 X, 1800–2800 Y)
- **Tags**: `Herbivore`, `CrowdSim_Herd`, `Triceratops`, `MassAI_Eligible`
- **Behavior**: Grazing idle → alert scatter when predator within 2000 units
- **Mass AI Config**: Flock radius 400 units, separation force 1.2, cohesion force 0.8

### Brachiosaurus Herd — `Brach_Savana_001` through `Brach_Savana_004`
- **Formation**: Wide spread (−1200 to −1800 X, 2700–3600 Y)
- **Tags**: `Herbivore`, `CrowdSim_Herd`, `Brachiosaurus`, `MassAI_Eligible`, `Passive`
- **Behavior**: Slow background movement, minimal threat response
- **Mass AI Config**: Flock radius 800 units, separation force 0.6, cohesion force 0.4

---

## Zone Infrastructure

### Crowd Simulation Zones
| Actor Label | Position | Purpose |
|---|---|---|
| CrowdZone_Herbivore_North | (0, 4000, 0) | Northern boundary |
| CrowdZone_Herbivore_South | (0, −4000, 0) | Southern boundary |
| CrowdZone_Herbivore_East | (4000, 0, 0) | Eastern boundary |
| CrowdZone_Herbivore_West | (−4000, 0, 0) | Western boundary |
| CrowdZone_HerdScatter_001 | (2500, 2000, 0) | Trike scatter trigger |
| CrowdZone_HerdScatter_002 | (−1500, 3200, 0) | Brach scatter trigger |

### LOD Zone Lights
| Actor Label | Color | Purpose |
|---|---|---|
| CrowdLOD_HerbivoreCore_001 | Green (0.1, 1.0, 0.2) | Trike herd core zone |
| CrowdLOD_HerbivoreCore_002 | Green (0.1, 1.0, 0.2) | Brach herd core zone |
| CrowdLOD_Transition_001 | Yellow (1.0, 0.9, 0.1) | LOD transition zone A |
| CrowdLOD_Transition_002 | Yellow (1.0, 0.9, 0.1) | LOD transition zone B |

### Migration Path Waypoints
| Actor Label | Position | Order |
|---|---|---|
| MigPath_Savana_001 | (−3000, −2000, 0) | Start |
| MigPath_Savana_002 | (−1500, 0, 0) | WP2 |
| MigPath_Savana_003 | (0, 1500, 0) | WP3 |
| MigPath_Savana_004 | (1500, 3000, 0) | WP4 |
| MigPath_Savana_005 | (3000, 4500, 0) | End |

---

## Mass AI Architecture

### Agent Tiers (LOD Chain)

```
LOD 0 (< 500 units from player): Full simulation
  - Individual physics, animation, sound
  - Behavior Tree active
  - Collision enabled

LOD 1 (500–2000 units): Reduced simulation
  - Simplified movement (no full physics)
  - Animation LOD (fewer bones)
  - No individual sound (ambient herd sound only)

LOD 2 (2000–5000 units): Crowd sim only
  - Position updates only (no animation)
  - Mass Entity processor handles movement
  - Instanced Static Mesh rendering

LOD 3 (> 5000 units): Dormant
  - Stored in Mass Entity registry
  - No tick, no render
  - Reactivated when player approaches
```

### Exclusion List (Individual AI — NOT Mass AI)
The following actor types run individual Behavior Trees and MUST be excluded from Mass AI:
- All actors tagged `Predator` (T-Rex, Raptors)
- All actors tagged `DinoAI`
- All actors tagged `CombatState_*`

### Herd Scatter Trigger
When a `PackHunter`-tagged actor enters alert state:
1. Broadcast radius: 2000 units (matches Raptor PackBroadcast_2000)
2. All `Herbivore`-tagged agents within radius enter `FleeState`
3. Flee direction: away from threat origin
4. Duration: 15 seconds, then return to `GrazingState`
5. Scatter speed multiplier: 2.5x normal movement

---

## Pathfinding Configuration

### NavMesh Integration
- NavMesh bounds confirmed in MinPlayableMap
- Mass AI uses NavMesh for pathfinding at LOD 0 and LOD 1
- LOD 2+ uses simplified steering (no NavMesh query)

### Herd Pathfinding Rules
1. **Separation**: Minimum 200 units between Triceratops agents
2. **Separation**: Minimum 500 units between Brachiosaurus agents
3. **Obstacle avoidance**: Trees, rocks, structures — avoidance radius 150 units
4. **Terrain following**: Height sampling every 100 units along path
5. **Water avoidance**: Agents do not enter water volumes (tag: `WaterVolume`)

---

## Performance Budget

### Target: 60 FPS on PC (RTX 3070 equivalent)

| Scenario | Active Agents | Budget |
|---|---|---|
| Normal gameplay | 50–200 | 0.5ms |
| Herd encounter | 200–500 | 1.2ms |
| Stampede event | 500–2000 | 2.5ms |
| Max stress test | 50,000 | 4.0ms |

### Optimization Strategies
1. **Spatial hashing**: 1000-unit grid cells for neighbor queries
2. **Time-sliced updates**: Agents update in batches (not all every frame)
3. **Instanced rendering**: ISM for LOD 2+ agents (single draw call per species)
4. **Processor threading**: Mass AI processors run on worker threads

---

## Integration with Other Systems

### From Agent #12 (Combat AI)
- Predator alert state → triggers HerdScatter in this system
- Combat zone markers (`CombatZone_Raptor_*`) define exclusion zones for herd pathfinding
- T-Rex detection perimeter (`TRexPerimeter`) → herd flee trigger at 3000 units

### To Agent #14 (Quest & Mission Designer)
- `MigPath_Savana_*` waypoints define migration corridor for quest triggers
- Herd scatter events can trigger quest objectives (e.g., "Survive the stampede")
- Herd density zones define safe areas for quest NPCs

### NavMesh Volumes
- Crowd sim agents respect NavMesh bounds
- Do NOT place crowd sim waypoints outside NavMesh bounds
- Patrol waypoints `WP_TRex_Patrol_*` and `WP_Raptor_Patrol_*` are reserved for Combat AI

---

## Stampede System (Design Spec)

### Trigger Conditions
1. Player fires weapon within 1500 units of herd
2. T-Rex enters herd zone
3. Raptor pack broadcasts alert within 2000 units
4. Thunderstorm weather event (future: Weather System)

### Stampede Behavior
1. **Initiation**: 1–3 agents begin fleeing (panic spreads)
2. **Cascade**: Each fleeing agent triggers neighbors within 300 units
3. **Direction**: Away from threat, following terrain contours
4. **Duration**: 20–45 seconds depending on threat persistence
5. **Player danger**: Agents in stampede deal 30 damage/collision at full speed
6. **Aftermath**: Herd disperses, 60-second recovery before regrouping

---

## Asset Registry

### Sound Cues (to be implemented by Agent #16)
- `SC_Triceratops_Graze` — low rumble, 3D spatial
- `SC_Triceratops_Alert` — sharp snort + hoof stamp
- `SC_Triceratops_Stampede` — thunder of hooves, heard at 5000 units
- `SC_Brachiosaurus_Call` — deep resonant vocalization
- `SC_HerdAmbience_Savana` — background herd presence sound

### VFX (to be implemented by Agent #17)
- `NS_DustCloud_Stampede` — ground dust trail per agent in stampede
- `NS_HerdBreath_Cold` — breath vapor for cold biome herds
- `NS_GrazingParticles` — grass disturbance while grazing

---

## Next Steps for Agent #14 (Quest & Mission Designer)

1. **Migration corridor** (`MigPath_Savana_001` → `005`) is available as quest trigger zone
2. **Herd encounter zones** (green LOD lights) mark safe observation points for quest NPCs
3. **Stampede trigger** can be used as quest mechanic: "Cause a stampede to distract the T-Rex"
4. **Herd scatter events** can unlock quest dialogue: "The herd is spooked — something big is nearby"
5. All crowd sim actors use naming convention `Type_Bioma_NNN` — reference by label in quest logic
