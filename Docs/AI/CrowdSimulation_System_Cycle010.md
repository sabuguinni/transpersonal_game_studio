# Crowd & Traffic Simulation System — Cycle 010 Documentation
**Agent #13 — Crowd & Traffic Simulation**
**Cycle:** PROD_CYCLE_AUTO_20260618_010
**Status:** COMPLETE — MAP_SAVED:True

---

## Overview

This cycle implements the full crowd simulation response system integrated with Agent #12's combat AI zones. The crowd system now reacts dynamically to predator presence, with flee behaviors, stampede routes, safe gathering points, migration corridors, and LOD distance management.

---

## Actors Deployed in MinPlayableMap

### Phase 1 — Flee Triggers & Safe Points (12 actors)

| Label | Location | Purpose |
|-------|----------|---------|
| `Crowd_FleeTrigger_RaptorZone` | (1800, 800, 80) | Crowd scatter origin — raptor pack center |
| `Crowd_FleeTrigger_TRexZone` | (4000, 2000, 80) | Max avoidance — T-Rex AggroCore |
| `Crowd_FleeTrigger_Retreat` | (800, 600, 80) | Retreat scatter origin from Agent #12 |
| `Crowd_SafePoint_Camp` | (-800, -800, 80) | Primary safe gathering (NPC_Camp_SleepZone) |
| `Crowd_SafePoint_Riverbank` | (-1200, -400, 80) | Secondary safe point — river |
| `Crowd_SafePoint_Cliffside` | (-600, -1200, 80) | Tertiary safe point — cliff |
| `Crowd_SafePoint_Forest` | (-1600, -600, 80) | Forest edge safe zone |
| `Crowd_Stampede_WP01` | (2000, 1200, 80) | Stampede route waypoint 1 |
| `Crowd_Stampede_WP02` | (1200, 800, 80) | Stampede route waypoint 2 |
| `Crowd_Stampede_WP03` | (400, 400, 80) | Stampede route waypoint 3 |
| `Crowd_Stampede_WP04` | (-400, 0, 80) | Stampede route waypoint 4 |
| `Crowd_Stampede_WP05` | (-800, -400, 80) | Stampede route waypoint 5 — leads to camp |

### Phase 2 — Density Zones & Herding (13 actors)

| Label | Location | Purpose |
|-------|----------|---------|
| `Crowd_Density_High_Camp` | (-800, -800, 50) | High density settlement center |
| `Crowd_Density_Med_Firepit` | (-600, -600, 50) | Medium density firepit area |
| `Crowd_Density_Low_Forest` | (-2000, -1000, 50) | Low density forest edge |
| `Crowd_Density_Low_River` | (-1200, 400, 50) | Low density river approach |
| `Crowd_Herd_Mammoth_Alpha` | (3000, -1000, 80) | Mammoth herd node 1 |
| `Crowd_Herd_Mammoth_Beta` | (3200, -800, 80) | Mammoth herd node 2 |
| `Crowd_Herd_Mammoth_Gamma` | (2800, -1200, 80) | Mammoth herd node 3 |
| `Crowd_Herd_Deer_Alpha` | (2000, -2000, 80) | Deer herd node 1 |
| `Crowd_Herd_Deer_Beta` | (2200, -1800, 80) | Deer herd node 2 |
| `Crowd_Herd_Deer_Gamma` | (1800, -2200, 80) | Deer herd node 3 |
| `Crowd_Avoid_TRex_Inner` | (4000, 2000, 80) | T-Rex inner danger ring (600 UU) |
| `Crowd_Avoid_TRex_Outer` | (4000, 2000, 80) | T-Rex outer alert ring (1200 UU) |
| `Crowd_Avoid_Raptor_Zone` | (1800, 800, 80) | Raptor pack avoidance marker |

### Phase 3 — Migration Corridors & LOD (16 actors)

| Label | Location | Purpose |
|-------|----------|---------|
| `Crowd_Migration_NS_WP01-06` | (0,-3000) → (1000,2000) | North-South seasonal migration |
| `Crowd_Migration_EW_WP01-06` | (-3000,0) → (2000,1000) | East-West water source route |
| `Crowd_LOD0_Radius` | (0, 0, 200) | Full simulation radius reference |
| `Crowd_LOD1_Radius` | (500, 0, 200) | Simplified simulation (500-2000 UU) |
| `Crowd_LOD2_Radius` | (2000, 0, 200) | Impostor sprites (2000-5000 UU) |
| `Crowd_LOD3_Cull` | (5000, 0, 200) | Culled beyond 5000 UU |

---

## Crowd Behavior Logic

### Flee State Machine
```
IDLE → ALERT (predator detected at 1500 UU) → FLEE (predator at 800 UU) → SAFE (reach SafePoint)
```

**Trigger Conditions:**
- `Crowd_FleeTrigger_RaptorZone`: Raptor pack active → crowd flees toward `Crowd_SafePoint_Camp`
- `Crowd_FleeTrigger_TRexZone`: T-Rex in CHARGE state → crowd stampede via WP01→WP05
- `Crowd_FleeTrigger_Retreat`: Combat retreat signal → crowd disperses to 4 safe points

### Stampede Route
T-Rex charge at (3200,1600) → crowd flees:
```
(2000,1200) → (1200,800) → (400,400) → (-400,0) → (-800,-400) → Camp (-800,-800)
```

### Density Distribution
- **High Density** (15-20 agents): Camp center, firepit area
- **Medium Density** (8-12 agents): River approach, forest edge
- **Low Density** (2-5 agents): Open plains, predator-adjacent zones

### LOD Chain
| LOD Level | Distance | Simulation Type | Agent Count |
|-----------|----------|-----------------|-------------|
| LOD0 | < 500 UU | Full physics + animation | 20 max |
| LOD1 | 500-2000 UU | Simplified movement | 100 max |
| LOD2 | 2000-5000 UU | Impostor sprites | 500 max |
| LOD3 | > 5000 UU | Culled | 0 |

---

## Integration with Agent #12 Combat Zones

| Agent #12 Actor | Agent #13 Response |
|-----------------|-------------------|
| `CombatRaptor_Alpha_001` (1800,800) | `Crowd_FleeTrigger_RaptorZone` activates |
| `Combat_TRex_AggroCore` (4000,2000) | `Crowd_Avoid_TRex_Inner/Outer` + stampede |
| `Combat_TRex_ChargeVector` (3200,1600) | Stampede WP01-05 activated |
| `CombatState_Retreat_Zone` (800,600) | `Crowd_FleeTrigger_Retreat` scatter |
| `NPC_Camp_SleepZone` (-800,-800) | `Crowd_SafePoint_Camp` destination |

---

## Migration System

### North-South Corridor (Seasonal)
- **Purpose**: Mammoth/large herbivore seasonal movement
- **Waypoints**: 6 nodes from (0,-3000) to (1000,2000)
- **Season**: Active during dry season migration

### East-West Corridor (Water Source)
- **Purpose**: Daily movement to river/water sources
- **Waypoints**: 6 nodes from (-3000,0) to (2000,1000)
- **Timing**: Dawn and dusk movement peaks

---

## Next Agent Directives — Agent #14 (Quest & Mission Designer)

1. **Hunt Quest**: Use `Crowd_Herd_Mammoth_Alpha/Beta/Gamma` (3000,-1000) as mammoth hunt target
2. **Rescue Quest**: Player must guide crowd agents from `Crowd_FleeTrigger_RaptorZone` to `Crowd_SafePoint_Camp`
3. **Stampede Event**: Quest trigger when T-Rex enters CHARGE state — protect 5 crowd agents
4. **Migration Escort**: Escort herbivore herd along `Crowd_Migration_NS_WP01-06` corridor
5. **Safe Zone Establishment**: Quest to build/reinforce `Crowd_SafePoint_Cliffside` as permanent refuge
6. **Density Bonus**: High-density camp area = quest hub for NPC interactions and item trading

---

## Technical Notes

- All actors use BasicShapes (Sphere/Cube) as placeholders — replace with proper crowd agent meshes
- Labels follow `Crowd_Type_Descriptor` naming convention (no degenerate suffixes)
- MAP_SAVED:True — all actors persisted to `/Game/Maps/MinPlayableMap`
- CAP_SAFE:True — actor count within 8000 limit
- DINO_SAFE:True — dinosaur count within 150 limit

---

*Generated by Agent #13 — Crowd & Traffic Simulation*
*PROD_CYCLE_AUTO_20260618_010*
