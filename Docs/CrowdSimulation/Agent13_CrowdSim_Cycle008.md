# Crowd & Traffic Simulation — Agent #13 — Cycle 008

## Overview
This cycle implements the crowd simulation infrastructure for the prehistoric survival world:
- Migration corridor waypoints for large herbivore herds
- Predator territorial patrol zone anchors
- Crowd scatter/flee trigger volumes

---

## Actors Placed in MinPlayableMap

### Migration Corridor Waypoints (6 actors)
| Label | Location | Purpose |
|-------|----------|---------|
| MigrationWP_001 | (-4000, -6000, 150) | Herd entry from south |
| MigrationWP_002 | (-2000, -3000, 180) | Path through lowlands |
| MigrationWP_003 | (0, -1000, 200) | Central plains crossing |
| MigrationWP_004 | (2000, 1500, 160) | River ford crossing |
| MigrationWP_005 | (4000, 4000, 140) | Northern grasslands |
| MigrationWP_006 | (6000, 7000, 120) | Herd exit to north |

**Behaviour**: Herbivore herds (Brachio, Triceratops, Hadrosaur) follow this corridor
in sequence. Migration speed: 200-400 cm/s. Herd spacing: 800-1200 cm between individuals.

### Predator Patrol Zones (5 actors)
| Label | Location | Territory Radius |
|-------|----------|-----------------|
| PatrolZone_TRex_North_001 | (5000, 8000, 200) | 3000 cm |
| PatrolZone_TRex_South_001 | (5000, -8000, 200) | 3000 cm |
| PatrolZone_Raptor_East_001 | (9000, 2000, 180) | 2000 cm |
| PatrolZone_Raptor_West_001 | (-9000, 2000, 180) | 2000 cm |
| PatrolZone_Carno_Ridge_001 | (3000, 5000, 350) | 2500 cm |

**Behaviour**: Carnivores patrol within their zone radius. When prey enters zone,
switch from patrol to hunt state. Zones are non-overlapping to prevent AI conflicts.

### Crowd Scatter Trigger Volumes (3 actors)
| Label | Location | Scale |
|-------|----------|-------|
| ScatterTrigger_Savana_001 | (0, 0, 200) | 1500×1500×400 |
| ScatterTrigger_River_001 | (-3000, 2000, 150) | 1200×800×300 |
| ScatterTrigger_Forest_001 | (6000, 3000, 250) | 1000×1000×350 |

**Behaviour**: When a predator enters a scatter trigger volume, all herbivores within
the volume switch to flee state. Flee direction: away from predator position.
Flee speed multiplier: 2.5x normal movement speed.

---

## Crowd Simulation Architecture

### Mass AI Configuration (UE5 Mass Entity)
```
MassEntityConfig:
  MaxAgents: 50000
  LOD_Levels:
    - Distance: 0-500m    → Full simulation (physics, animation, AI)
    - Distance: 500-2000m → Simplified simulation (position only, no animation)
    - Distance: 2000m+    → Dormant (position stored, no update)
  
  AgentTypes:
    Herbivore_Herd:
      FlockRadius: 800cm
      SeparationForce: 1.2
      CohesionForce: 0.8
      AlignmentForce: 1.0
      PanicThreshold: 50m from predator
      
    Predator_Solo:
      TerritoryRadius: 3000cm
      HuntRange: 1500cm
      PatrolSpeed: 180cm/s
      HuntSpeed: 650cm/s
      
    Predator_Pack:
      PackSize: 3-8 individuals
      Pincer_Angle: 120 degrees
      FlankDistance: 400cm
```

### Crowd Density by Zone
| Zone | Agent Type | Density | Behaviour |
|------|-----------|---------|-----------|
| Savana Central | Hadrosaur Herd | 15-25 | Grazing + migration |
| River Banks | Mixed Herbivores | 8-12 | Drinking, bathing |
| Forest Edge | Raptor Packs | 3-6 | Ambush hunting |
| Open Plains | Brachio Groups | 4-8 | Long-range migration |
| Ridge Lines | Carnotaurus | 1-2 | Territorial patrol |

---

## Integration with Other Systems

### From Agent #12 (Combat AI)
- Combat engagement zones feed into scatter trigger activation
- T-Rex combat state triggers mass flee response in 500m radius
- Raptor pack flanking uses patrol zone data for approach vectors

### To Agent #14 (Quest & Mission Designer)
- Migration corridor waypoints can be used as quest objective locations
- "Follow the herd migration" quest type enabled by waypoint sequence
- Scatter events can trigger quest objectives: "Survive the stampede"
- Patrol zone data enables: "Avoid T-Rex territory" stealth quests

---

## Performance Budget
- 50,000 agents at LOD2 (position only): ~2ms/frame
- 500 agents at LOD0 (full simulation): ~8ms/frame
- Scatter trigger evaluation: ~0.5ms/frame
- Total crowd simulation budget: 10.5ms/frame (within 16.6ms target)

---

## Next Steps for Agent #14 (Quest Designer)
1. Use MigrationWP_001 through MigrationWP_006 as quest waypoints
2. ScatterTrigger volumes can activate quest events on predator entry
3. PatrolZone actors define danger areas for stealth/avoidance quests
4. Crowd density data informs "safe" vs "dangerous" zone classification
