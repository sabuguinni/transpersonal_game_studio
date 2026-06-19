# Crowd & Traffic Simulation System — Cycle 021 (PROD_CYCLE_AUTO_20260619_002)

## Agent #13 — Crowd & Traffic Simulation

---

## Overview

This cycle implements the crowd simulation layer that integrates with Agent #12's combat AI zones. The system defines herbivore herd movement corridors, crowd exclusion zones around predator territory, safe gathering density nodes, and LOD performance boundaries.

---

## Actors Deployed in MinPlayableMap

### Herbivore Herd Waypoints (Cyan PointLights)
| Label | Position | Purpose |
|-------|----------|---------|
| `HerdWP_Brachio_01_Grazing` | (-1500, 2000, 100) | Brachiosaurus herd grazing start |
| `HerdWP_Brachio_02_River` | (500, 1500, 100) | River crossing point |
| `HerdWP_Brachio_03_Flee` | (-2000, 1500, 100) | Flee destination (matches BT_Zone_Flee_Escape) |
| `HerdWP_Stego_01_Forest` | (800, 3500, 100) | Stegosaur forest edge grazing |
| `HerdWP_Stego_02_Clearing` | (-500, 2800, 100) | Open clearing movement |
| `HerdWP_Stego_03_Safe` | (-1800, 2200, 100) | Safe zone arrival |
| `StampedeAlert_TRex_Corridor` | (2000, 1200, 150) | Stampede trigger — TRex proximity detection |

**Stampede Logic:** When TRex enters patrol node `TRex_Patrol_02_River` (3000, 1000), herbivores within 2000u of `StampedeAlert_TRex_Corridor` flee toward `HerdWP_Brachio_03_Flee` / `HerdWP_Stego_03_Safe`. This gives the player ~8-12 seconds of visual warning before TRex becomes visible.

---

### Crowd Exclusion Zones (Red PointLights)
| Label | Position | Radius | Reason |
|-------|----------|--------|--------|
| `CrowdExclude_Raptor_Alpha` | (5000, 2000, 120) | 800u | Raptor pack leader territory |
| `CrowdExclude_Raptor_Beta` | (5800, 1500, 120) | 800u | Eastern raptor flank |
| `CrowdExclude_Raptor_Gamma` | (4500, 2800, 120) | 800u | Raptor ambush point |
| `CrowdExclude_TRex_River` | (3000, 1000, 120) | 800u | TRex patrol corridor |
| `CrowdExclude_TRex_Perimeter` | (5500, 2500, 120) | 800u | TRex outer perimeter |

**Rule:** No crowd agents spawn or navigate within 800u of any exclusion zone. Maximum 20 crowd agents within 2000u of any combat zone (performance + gameplay clarity).

---

### Crowd Density Nodes (Yellow PointLights)
| Label | Position | Max Agents | Purpose |
|-------|----------|-----------|---------|
| `CrowdDensity_Camp_Center` | (0, 0, 100) | 20 | Main camp hub |
| `CrowdDensity_Camp_East` | (400, 300, 100) | 15 | Camp eastern quarter |
| `CrowdDensity_Forest_Safe` | (-800, 2000, 100) | 10 | Forest safe gathering |
| `CrowdDensity_River_Safe` | (200, 1000, 100) | 8 | River safe crossing |
| `CrowdDensity_Hillside_Watch` | (-600, -800, 200) | 5 | Elevated lookout post |

**Total max simultaneous crowd agents: 58** (well within performance budget)

---

### LOD Zone Markers (White PointLights)
| Label | LOD Level | Distance | Simulation Mode |
|-------|-----------|----------|----------------|
| `CrowdLOD_Near_Ring` | LOD 0 | 0-1500u | Full simulation, all behaviors |
| `CrowdLOD_Mid_Ring` | LOD 1 | 1500-4000u | Reduced update rate (5Hz vs 30Hz) |
| `CrowdLOD_Far_Ring` | LOD 2 | 4000u+ | Billboard/impostor only |

### Crowd Flow Direction Nodes (White PointLights)
| Label | Position | Flow Direction |
|-------|----------|---------------|
| `CrowdFlow_Camp_to_Forest` | (-400, 1000, 100) | Camp → Forest |
| `CrowdFlow_Forest_to_River` | (200, 1800, 100) | Forest → River |
| `CrowdFlow_River_to_Camp` | (100, 500, 100) | River → Camp |

---

## Crowd Simulation Architecture

### Agent Population Budget
```
Total world capacity: 50,000 agents (Mass AI theoretical max)
Active simulation: 58 agents max (performance-safe for MinPlayableMap)
LOD breakdown:
  - LOD 0 (full): max 20 agents within 1500u of player
  - LOD 1 (reduced): max 38 agents at 1500-4000u
  - LOD 2 (billboard): unlimited beyond 4000u (not yet implemented)
```

### Herd Behavior States
1. **GRAZING** — Default state at density nodes, slow movement, feeding animations
2. **ALERT** — Head-raising, group clustering when predator detected at 1500u
3. **FLEE** — Stampede toward designated safe waypoints, max speed
4. **DISPERSED** — Post-stampede scatter, individuals rejoin herd slowly

### Stampede Early Warning System
The `StampedeAlert_TRex_Corridor` at (2000, 1200) acts as a passive detection zone:
- Herbivores within 2000u of this point begin ALERT state when TRex is active
- Stampede direction: away from (3000, 1000) toward (-2000, 1500)
- Player sees herd panic 8-12 seconds before TRex visual contact
- Creates emergent gameplay: "Why are the brachiosaurs running?"

---

## Integration Map

### Receives from Agent #12 (Combat AI):
- `TRex_Patrol_02_River` (3000, 1000) — stampede trigger reference point
- `Raptor_CombatZone_Alpha/Beta/Gamma` — exclusion zone positions
- `BT_Zone_Flee_Escape` (-2000, 1500) — herd flee destination

### Provides to Agent #11 (NPC Behavior):
- `CrowdDensity_*` nodes — NPC safe gathering locations
- `CrowdExclude_*` zones — NPC avoidance areas (same as crowd)
- Stampede alert radius — NPCs react to herd panic same as player

### Provides to Agent #14 (Quest & Mission):
- Stampede corridor as quest trigger: "Follow the fleeing herd to find the TRex"
- `CrowdDensity_Hillside_Watch` as quest location: elevated observation post
- Herd waypoints as navigation references for escort/tracking quests

---

## Performance Specifications

| Metric | Value | Target |
|--------|-------|--------|
| Max simultaneous agents | 58 | <100 |
| LOD 0 update rate | 30Hz | 30Hz |
| LOD 1 update rate | 5Hz | 5-10Hz |
| LOD 2 | Billboard | Billboard |
| Exclusion zone checks | Per-frame within 5000u | <1ms |
| Stampede trigger latency | <100ms | <200ms |

---

## Color Coding Reference (Full System)

| Color | Hex | System | Agent |
|-------|-----|--------|-------|
| Deep Red | #FF0000 | TRex danger zones | #12 |
| Orange | #FF8000 | Raptor threat zones | #12 |
| Magenta | #FF00FF | Combat triggers | #12 |
| Green | #00FF00 | BT behavior zones | #11 |
| Blue | #0000FF | NPC patrol routes | #11 |
| Yellow | #FFFF00 | Crowd density nodes | #13 |
| Cyan | #00FFFF | Herd waypoints | #13 |
| Red (exclusion) | #FF0000 | Crowd exclusion | #13 |
| White | #FFFFFF | LOD boundaries | #13 |

---

## Agent #14 Handoff — Quest & Mission Designer

**Priority inputs from this system:**

1. **Stampede Quest Hook:** Herbivore stampede is a natural quest trigger — "Something big is coming. Follow the herd's panic direction to investigate." Quest: `QST_TrackThePredator`

2. **Herd Escort Quest:** Player must guide a separated juvenile brachiosaur from `HerdWP_Brachio_02_River` back to `HerdWP_Brachio_01_Grazing` while avoiding TRex patrol. Quest: `QST_LostCalf`

3. **Observation Post:** `CrowdDensity_Hillside_Watch` at (-600, -800, 200) is elevated — ideal for a quest where player must scout predator movements before the tribe can safely travel. Quest: `QST_ScoutAhead`

4. **Exclusion Zone Quests:** Raptor triangle (Alpha/Beta/Gamma) is a natural "forbidden zone" — quest could require player to retrieve something from inside. Quest: `QST_RaptorNest`

5. **Crowd as Quest Witnesses:** Density node NPCs can react to quest events — camp NPCs at `CrowdDensity_Camp_Center` panic when player triggers `CombatTrigger_TRex_Detect`, creating emergent storytelling.

---

*Generated by Agent #13 — Crowd & Traffic Simulation*
*Cycle: PROD_CYCLE_AUTO_20260619_002*
*Depends on: Agent #12 combat zones, Agent #11 BT zones*
*Consumed by: Agent #14 Quest Designer, Agent #11 NPC Behavior*
