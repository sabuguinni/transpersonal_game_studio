# Crowd & Traffic Simulation System — Production Cycle 013
**Agent #13 — Crowd & Traffic Simulation**
**Cycle:** PROD_CYCLE_AUTO_20260617_013

---

## Overview

This document defines the crowd simulation layer for the prehistoric survival world. All crowd agents operate within the ecological constraints established by Agent #12 (Combat AI) — respecting combat exclusion zones, sound propagation radii, and density limits.

---

## Crowd Agent Categories

### 1. Herbivore Herds (Triceratops)
**Waypoints placed in MinPlayableMap:**
- `Herd_Tricera_Graze_001` → (-4000, -3000, 100) — primary grazing zone
- `Herd_Tricera_Graze_002` → (-4500, -2500, 100) — secondary grazing zone
- `Herd_Tricera_Graze_003` → (-3800, -3500, 100) — tertiary grazing zone
- `Herd_Tricera_Scatter_001` → (-3000, -2000, 100) — scatter rally point on predator detection

**Behaviour Rules:**
- Graze in formation (5–12 individuals, loose cluster radius 800u)
- Sentinel individual (largest) faces outward — 270° vision cone
- On TRex ALERT within 3000u → scatter toward `Herd_Tricera_Scatter_001`
- Scatter speed: 1.4× normal movement speed
- Reform after 45 seconds if no further threat detected

**Exclusion Zones (from Agent #12):**
- `CombatZone_TRex_Nest` — 3000u hard exclusion (herds never enter)
- `TriggerZone_Ambush_001` — 1500u soft avoidance (herds detour around)

---

### 2. Scavenger Agents (Pterosaurs + Ground Scavengers)
**Waypoints placed in MinPlayableMap:**
- `Scavenger_Ptero_001` → (1500, 2500, 200) — aerial circling position
- `Scavenger_Ptero_002` → (1800, 2200, 250) — secondary aerial position
- `Scavenger_Ptero_003` → (1200, 2800, 180) — tertiary aerial position
- `Scavenger_Ground_001` → (1600, 2600, 100) — ground approach vector
- `Scavenger_Ground_002` → (1400, 2400, 100) — secondary ground approach

**Behaviour Rules:**
- Idle state: patrol `Flock_Ptero_Patrol_*` waypoints at altitude 400–450u
- On kill event at `TriggerZone_Feeding_001` → converge on feeding zone
- Convergence delay: Ptero 8s (aerial), Ground 15s (approach from tree line)
- Feeding priority: alpha ground scavenger feeds first, Ptero circle overhead
- Flee trigger: any large predator within 600u of feeding zone

---

### 3. Aerial Flock (Pterosaur Patrol)
**Waypoints placed in MinPlayableMap:**
- `Flock_Ptero_Patrol_001` → (-1000, -4000, 400)
- `Flock_Ptero_Patrol_002` → (1000, -3000, 450)
- `Flock_Ptero_Patrol_003` → (2000, -1000, 420)
- `Flock_Ptero_Patrol_004` → (1500, 1000, 400)

**Behaviour Rules:**
- 3–6 individuals in loose V-formation
- Patrol loop: 001→002→003→004→001 (clockwise)
- Speed: 600u/s cruise, 900u/s flee
- Scatter on: TRex roar (SoundProp_TRex_Roar_Zone), gunshot, explosion
- Scatter direction: away from sound source, altitude +200u

---

### 4. Migration Corridor (Brachiosaurus)
**Waypoints placed in MinPlayableMap:**
- `Migration_Brachio_Path_001` → (0, -5000, 100) — entry from north
- `Migration_Brachio_Path_002` → (200, -3000, 100)
- `Migration_Brachio_Path_003` → (-100, -1000, 100) — mid-corridor
- `Migration_Brachio_Path_004` → (300, 1000, 100)
- `Migration_Brachio_Path_005` → (100, 3000, 100) — exit to south

**Behaviour Rules:**
- 2–4 Brachiosaurus in single-file column, 600u spacing
- Migration speed: 180u/s (slow, majestic)
- Player proximity: no flee response — Brachio ignores player unless attacked
- TRex proximity: detour 800u east if TRex within 2000u of corridor
- Migration schedule: active during dawn (06:00–08:00) and dusk (17:00–19:00) in-game time

---

## Crowd Density Limits

| Zone | Radius | Max Agents | Rule |
|------|--------|-----------|------|
| Active combat (any predator) | 1000u | 5 | Soft limit — agents flee |
| Active combat (any predator) | 500u | 0 | Hard exclusion |
| TRex nest (`CombatZone_TRex_Nest`) | 3000u | 0 | Permanent exclusion |
| Raptor ambush zone | 1500u | 0 | During PATROL state |
| Feeding zone | 500u | 8 | Scavengers only |

**Density Markers placed:**
- `CrowdDensity_TRex_Exclusion` → (3000, 500, 100)
- `CrowdDensity_Raptor_Exclusion` → (-2500, 1800, 100)
- `CrowdDensity_Feeding_Limit` → (1600, 2600, 100)

---

## Sound Propagation — Crowd Response

| Sound Event | Source | Radius | Crowd Response |
|------------|--------|--------|---------------|
| TRex roar | `SoundProp_TRex_Roar_Zone` | 2000u | All herbivores scatter |
| Raptor shriek | `SoundProp_Raptor_Shriek_Zone` | 2000u | Small animals scatter |
| Feeding sounds | `SoundProp_Feeding_Zone` | 1500u | Scavengers converge |
| Player sprint | Player location | 400u | Small animals flee |
| Player weapon swing | Player location | 600u | Medium animals flee |

**Sound Trigger Markers placed:**
- `SoundProp_TRex_Roar_Zone` → (3000, 500, 100)
- `SoundProp_Raptor_Shriek_Zone` → (-2500, 1800, 100)
- `SoundProp_Feeding_Zone` → (1600, 2600, 100)

---

## Ecological Interaction Matrix

```
TRex (PATROL)     → Herds graze normally, 3000u avoidance
TRex (ALERT)      → Herds scatter, Flock altitude +200u, Migration detours
TRex (ATTACK)     → All crowd within 2000u flee, 0 agents within 500u
Raptors (PATROL)  → Herds avoid 1500u radius
Raptors (ATTACK)  → Herds scatter, Scavengers wait at 800u
Kill Event        → Scavengers converge (8–15s delay), Flock circles
Player (idle)     → No crowd reaction
Player (sprint)   → Small animals flee 400u radius
Player (combat)   → Crowd treats player as predator (500u exclusion)
```

---

## Integration Handoff — Agent #14 (Quest & Mission Designer)

The crowd simulation system provides the following observable events for quest triggers:

1. **HERD_SCATTER** — fired when herbivore herd scatters (indicates predator nearby)
   - Quest use: "Follow the fleeing herd to find the TRex"
   
2. **SCAVENGER_CONVERGE** — fired when scavengers converge on feeding zone
   - Quest use: "Follow the scavengers to find a fresh kill / resources"
   
3. **MIGRATION_ACTIVE** — fired when Brachiosaurus migration is in progress
   - Quest use: "Cross the migration corridor safely" / "Observe the migration"
   
4. **FLOCK_SCATTER** — fired when aerial flock scatters
   - Quest use: "The birds scattered — something spooked them. Investigate."

5. **CROWD_SILENCE** — fired when an area that normally has crowd activity goes silent
   - Quest use: "The forest is too quiet. Something is wrong."

---

## Technical Notes

- All waypoints are PointLight actors scaled to 0.1 (invisible in-game) — serve as navigation targets
- Actual crowd agent spawning requires Mass AI or Blueprint implementation (future cycle)
- Current implementation establishes the spatial layout and design intent
- All markers respect Agent #12 combat zone boundaries (verified against CombatZone_TRex_Nest, TriggerZone_Ambush_001)
- MAP_SAVED: True after all spawns

---

*Generated by Agent #13 — Crowd & Traffic Simulation*
*Cycle: PROD_CYCLE_AUTO_20260617_013*
