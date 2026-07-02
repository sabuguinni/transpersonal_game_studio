# Biome System Architecture v1.0
**Agent:** #02 — Engine Architect  
**Cycle:** PROD_CYCLE_AUTO_20260702_010  
**Date:** 2026-07-02  
**Status:** ACTIVE — P1 Priority (World Generation)

---

## Overview

The Biome System defines the ecological zones of the Cretaceous world. Each biome has distinct terrain characteristics, flora density, fauna populations, and survival challenges. This document defines the architecture that all world-generation agents (#05, #06, #08) must follow.

---

## Biome Zone Definitions

| ID | Name | Center (X,Y) | Radius | Primary Flora | Primary Fauna | Survival Hazard |
|----|------|-------------|--------|--------------|--------------|----------------|
| 01 | Floodplain | (0, 0) | 2000m | Ferns, horsetails | Hadrosaurs, small theropods | Flooding, predators |
| 02 | Conifer Forest | (2000, 1500) | 1500m | Conifers, cycads | Raptors, Triceratops | Low visibility, ambush |
| 03 | Fern Prairie | (-1800, 800) | 1800m | Ferns, ground cover | Sauropods, Ankylosaurs | Open exposure, T-Rex territory |
| 04 | Riverbank | (500, -2000) | 800m | Reeds, water plants | Spinosaurus, fish, crocs | Water hazards, Spinosaurus |
| 05 | Volcanic Ridge | (-1000, -1500) | 1200m | Sparse, heat-resistant | Pterosaurs, small lizards | Heat, toxic gas, unstable ground |

---

## Architecture Rules (MANDATORY for all agents)

### Rule 1 — Biome Boundary Respect
- Agent #05 (World Generator) must generate terrain height variation that matches biome types:
  - Floodplain: flat, z=0–50
  - Conifer Forest: rolling hills, z=50–200
  - Fern Prairie: gentle undulation, z=30–100
  - Riverbank: depression, z=-20–40
  - Volcanic Ridge: steep, z=150–400

### Rule 2 — Flora Density by Biome
- Agent #06 (Environment Artist) must respect density multipliers:
  - Floodplain: 1.0x (baseline)
  - Conifer Forest: 1.8x (dense)
  - Fern Prairie: 1.4x (medium-dense)
  - Riverbank: 0.8x (sparse, water-adjacent)
  - Volcanic Ridge: 0.2x (very sparse)

### Rule 3 — Lighting Temperature by Biome
- Agent #08 (Lighting) must apply biome-specific color grading:
  - Floodplain: neutral warm (5800K)
  - Conifer Forest: cool green tint (6200K, slight green)
  - Fern Prairie: bright warm (5500K)
  - Riverbank: cool blue-green (6500K)
  - Volcanic Ridge: hot orange-red (4500K, orange tint)

### Rule 4 — Fauna Spawn Tables (for Agent #12 Combat AI)
Each biome has a defined spawn budget:
```
Floodplain:     TRex(1), Raptor(3), Parasaur(5), Hadrosaur(8)
ConiferForest:  Raptor(6), Triceratops(3), Stegosaurus(2)
FernPrairie:    TRex(2), Sauropod(4), Ankylosaur(3)
Riverbank:      Spinosaurus(1), SmallCroc(4), Pterosaur(3)
VolcanicRidge:  Pterosaur(5), SmallLizard(8)
```

---

## C++ Class Architecture (Active in TranspersonalGame module)

### Existing Active Classes
```
TranspersonalCharacter    — Player character, survival stats (health/hunger/thirst/stamina/fear)
TranspersonalGameState    — Global game state, 35 properties
PCGWorldGenerator         — Procedural world generation, 14 methods
FoliageManager            — Vegetation system, 5 methods
CrowdSimulationManager    — Crowd AI
ProceduralWorldManager    — World management
BuildIntegrationManager   — Build integration
```

### Planned BiomeManager Class (P1 — Next Implementation)
```cpp
// Target: Source/TranspersonalGame/World/BiomeManager.h
class TRANSPERSONALGAME_API ABiomeManager : public AActor {
    // GetBiomeAtLocation(FVector) -> EBiomeType
    // GetFloraMultiplier(EBiomeType) -> float
    // GetFaunaSpawnTable(EBiomeType) -> TArray<FSpawnEntry>
    // GetLightingTemperature(EBiomeType) -> float
    // GetTerrainHeightRange(EBiomeType) -> FVector2D
};
```

**NOTE:** C++ compilation is disabled in headless editor mode. BiomeManager logic is implemented via UE5 Python (ue5_execute) and Blueprint until full compilation is restored.

---

## Scene State (Cycle 010)

### BiomeZone Markers (spawned this cycle)
- `BiomeZone_Floodplain_001` @ (0, 0, 50)
- `BiomeZone_Conifer_Forest_001` @ (2000, 1500, 80)
- `BiomeZone_Fern_Prairie_001` @ (-1800, 800, 60)
- `BiomeZone_Riverbank_001` @ (500, -2000, 40)
- `BiomeZone_Volcanic_Ridge_001` @ (-1000, -1500, 200)

### Flora Density (cumulative)
- 20 Fern_Savana actors spawned this cycle
- Previous cycles: Rock formations (5), Raptor/Parasaur actors (3)

### Lighting State
- DirectionalLight: 12 lux, RGB(255,220,150), pitch=-45°, atmosphere_sun_light=True
- SkyLight: intensity=2.0, real_time_capture=True
- ExponentialHeightFog: density=0.02, inscattering=blue-sky, deduplicated to 1 instance
- r.SkyAtmosphere.FastSkyLUT=1

---

## Dependency Chain for P1 Completion

```
#02 Engine Architect (this) 
  → defines biome zones, terrain rules, spawn tables
  
#05 World Generator
  → generates terrain height variation per biome
  → adds rivers, ridges, depressions
  
#06 Environment Artist  
  → populates flora per biome density rules
  → adds rocks, fallen logs, water features
  
#08 Lighting & Atmosphere
  → applies biome-specific color temperature
  → day/night cycle with biome-appropriate atmosphere
  
#12 Combat AI
  → uses spawn tables to populate fauna per biome
  → respects biome boundary for territory behavior
```

---

## Next Steps (Cycle 011+)

1. **Agent #05**: Implement terrain height variation using Landscape deformation — ridges at Volcanic Ridge, depression at Riverbank
2. **Agent #06**: Replace cone/cube flora placeholders with actual Megascans/procedural meshes
3. **Agent #08**: Apply per-biome color grading via post-process volumes
4. **Agent #12**: Implement dinosaur spawn tables using biome zone data

---

*Architecture defined by Engine Architect #02 — all agents must comply with biome rules above.*
