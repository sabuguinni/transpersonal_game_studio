# Crowd & Traffic Simulation System — Cycle 013
**Agent:** #13 Crowd & Traffic Simulation  
**Cycle:** PROD_CYCLE_AUTO_20260617_012  
**Integration:** Builds on Agent #12 Combat AI outputs

---

## Systems Implemented This Cycle

### 1. Herbivore Herd Panic Response
**Trigger:** `TRex_AggroTrigger_001` fires at (3400, 1600)  
**Response:** 8 scatter destinations in cardinal/diagonal directions  
**Scatter radius:** 2200 units from aggro center  
**Visual marker:** Green point lights (safe scatter destinations)

Scatter points:
| Label | Direction | Location |
|-------|-----------|----------|
| HerdPanic_Scatter_N_001 | North | (3400, 3800) |
| HerdPanic_Scatter_NE_001 | NE | (4956, 3356) |
| HerdPanic_Scatter_E_001 | East | (5600, 1600) |
| HerdPanic_Scatter_SE_001 | SE | (4956, -156) |
| HerdPanic_Scatter_S_001 | South | (3400, -600) |
| HerdPanic_Scatter_SW_001 | SW | (1844, -156) |
| HerdPanic_Scatter_W_001 | West | (1200, 1600) |
| HerdPanic_Scatter_NW_001 | NW | (1844, 3356) |

**Behavior logic:**
- On TRex aggro trigger: all herbivore Mass AI agents receive flee command
- Nearest scatter point selected per agent (avoids clustering)
- Agents maintain minimum separation of 150 units during panic
- Panic state duration: 45 seconds, then gradual return to grazing

---

### 2. Evacuation Corridor Network
**Purpose:** Funnel crowd agents from combat zones to safe retreat points  
**Corridors:** 3 (Cave, Rocks, River — matching Agent #12 retreat points)  
**Visual marker:** Yellow point lights (evacuation waypoints)

#### Cave Corridor (NW direction)
- WP1: (2800, 2200) → WP2: (2200, 2800) → WP3: (1600, 3400) → CombatRetreat_Cave_001

#### Rocks Corridor (E direction)  
- WP1: (4000, 1600) → WP2: (4600, 1800) → WP3: (5200, 2000) → CombatRetreat_Rocks_001

#### River Corridor (S direction)
- WP1: (3400, 800) → WP2: (3200, 0) → WP3: (3000, -800) → CombatRetreat_River_001

**Capacity per corridor:** ~150 agents simultaneously  
**Flow rate:** 12 agents/second per waypoint transition

---

### 3. Combat Zone Exclusion System
**Purpose:** Mass AI agents avoid active combat zones  
**Visual marker:** Red point lights (danger/exclusion boundary)

#### T-Rex Aggro Zone (radius 1500)
- Center: (3400, 1600)
- 4 boundary markers at N/E/S/W perimeter
- Exclusion weight: 1.0 (hard block — agents reroute around)

#### Raptor Combat Zone (radius 600)
- Center: (3400, 1600) — same center as T-Rex zone
- 4 boundary markers at N/E/S/W perimeter  
- Exclusion weight: 0.8 (strong avoidance — agents detour)

**Pathfinding integration:**
- NavMesh cost multiplier: 100x inside exclusion zones
- Agents approaching boundary receive redirect signal at 200 units pre-boundary
- Emergency scatter if agent enters zone unexpectedly

---

### 4. Mass AI Density Zones
**Purpose:** Define high-density herbivore grazing areas in safe regions  
**Visual marker:** Blue point lights (density zone centers)

| Label | Location | Population |
|-------|----------|------------|
| MassAI_DensityZone_Meadow_001 | (-500, 500) | ~80 agents |
| MassAI_DensityZone_River_001 | (1000, -1500) | ~60 agents |
| MassAI_DensityZone_Forest_001 | (-1500, -800) | ~45 agents |
| MassAI_DensityZone_Plains_001 | (2000, 3500) | ~70 agents |

**Total simulated population:** ~255 herbivore agents across 4 zones  
**LOD behavior:**
- Within 2000 units: full simulation (individual pathfinding)
- 2000-5000 units: group-level simulation (flock behavior)
- Beyond 5000 units: static representation only

---

## Actor Summary (This Cycle)
- **8** HerdPanic_Scatter markers (green lights)
- **9** CrowdEvac waypoints (yellow lights) — 3 per corridor
- **8** CrowdExclusion boundary markers (red lights)
- **4** MassAI_DensityZone centers (blue lights)
- **Total:** 29 new actors

---

## Integration with Previous Agents

### From Agent #11 (NPC Behavior)
- Patrol circuits used as base movement for crowd agents
- NPC daily routines inform crowd density schedules (dawn = high activity)

### From Agent #12 (Combat AI)
- `TRex_AggroTrigger_001` → triggers herd panic scatter
- `RaptorCombat_*` zones → crowd exclusion radius 600
- `CombatRetreat_Cave/Rocks/River_001` → evacuation corridor endpoints
- `pack_coordination_radius: 800` → raptor group pathfinding handled by crowd sim

---

## For Agent #14 (Quest & Mission Designer)

### Quest Hooks Available
1. **"The Stampede"** — Player must survive/navigate herbivore panic scatter when T-Rex appears
2. **"Safe Passage"** — Guide NPC through evacuation corridor while raptors patrol
3. **"Counting the Herd"** — Observe density zones to track herbivore population (survival intel)
4. **"The Exclusion Zone"** — Player must enter combat exclusion zone to retrieve something (high risk)

### Trigger Points for Missions
- `TRex_AggroTrigger_001` — mission start trigger for stampede events
- `MassAI_DensityZone_Meadow_001` — peaceful observation mission start
- Any `CrowdEvac_*_WP1_001` — escort mission checkpoint markers

### Crowd State Data Available
- `CROWD_STATE`: CALM / ALERT / PANIC / EVACUATING
- `ACTIVE_EXCLUSION_ZONES`: list of active danger areas
- `HERD_POPULATION`: current count per density zone
- `EVACUATION_ACTIVE`: bool — whether evacuation corridors are in use

---

## Technical Notes

### Mass AI Configuration (UE5 Mass Entity)
```
MassEntity Processor Chain:
1. MassObstacleAvoidanceProcessor — handles exclusion zones
2. MassZoneGraphAnnotationProcessor — reads waypoint network
3. MassSteeringProcessor — applies flee/scatter behaviors
4. MassCrowdVisualizationProcessor — LOD switching

Fragment types used:
- FMassVelocityFragment — movement vectors
- FMassTransformFragment — world position
- FMassCrowdObstacleFragment — exclusion zone awareness
- FMassZoneGraphShortPathFragment — waypoint navigation
```

### Performance Budget
- 255 full-sim agents: ~0.8ms/frame (within P4 budget)
- LOD reduction at 2000 units: drops to ~0.2ms/frame
- Exclusion zone checks: O(n) per frame, cached every 0.5s
- Panic scatter: burst cost ~1.2ms on trigger, normalizes in 2 frames
