# Biome System Architecture — Engine Architect #02
## PROD_CYCLE_AUTO_20260703_001

---

## Overview

The Biome System defines the ecological zones of the prehistoric world. Each biome
governs terrain generation, vegetation density, dinosaur species distribution,
weather patterns, and ambient audio. This document is the authoritative architecture
reference for all agents implementing biome-related systems.

---

## Biome Types (SharedTypes.h — EEng_BiomeType)

| ID | Name | Description | Dominant Species |
|----|------|-------------|-----------------|
| 0 | Savana | Open grassland, sparse trees, high visibility | T-Rex, Triceratops |
| 1 | Floresta | Dense jungle, low visibility, high humidity | Raptors, Parasaurolophus |
| 2 | Pantano | Swamp, shallow water, fog | Spinosaurus, Baryonyx |
| 3 | Montanha | Rocky highlands, cliffs, cold | Pterodactyl, Pachycephalosaurus |
| 4 | Praia | Coastal, sandy, open water nearby | Mosasaurus (offshore), Dilophosaurus |
| 5 | Vulcao | Volcanic, ash, extreme heat | Ankylosaurus, Carnotaurus |

---

## Architecture Rules (Enforced by Engine Architect)

### RULE A — BiomeManager is the Single Source of Truth
- All biome queries go through `BiomeManager_Proxy_001` actor in the level
- No agent spawns biome-specific content without checking the active biome at that location
- Biome boundaries are defined by a 2D noise map seeded at world gen time

### RULE B — Naming Convention (MANDATORY)
All actors spawned in the world MUST follow: `Type_Bioma_NNN`
- Types: TRex, Raptor, Trike, Brachi, Spino, Ptero, Tree, Fern, Rock, Bush, River, Cave
- Biomas: Savana, Floresta, Pantano, Montanha, Praia, Vulcao
- NNN: zero-padded 3-digit index (001, 002, ...)
- Examples: `TRex_Savana_001`, `Tree_Floresta_042`, `Rock_Montanha_007`

### RULE C — Hero Clearing is Sacred (X=2100, Y=2400)
- This is the primary screenshot composition point
- Must always contain: 1 T-Rex + 2 Raptors + dense vegetation ring
- No agent may remove actors from this clearing without Engine Architect approval
- Biome at this location: **Floresta** (dense Cretaceous forest)

### RULE D — No Spiritual Content
Per global anti-hallucination rule: zero shamans, zero spirit guides, zero mystical mechanics.
All biome content is ecologically grounded (National Geographic standard).

---

## BiomeManager C++ Interface (Target — pending compilation fix)

```cpp
// SharedTypes.h additions (Eng_ prefix, global scope)
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Savana     UMETA(DisplayName = "Savana"),
    Floresta   UMETA(DisplayName = "Floresta"),
    Pantano    UMETA(DisplayName = "Pantano"),
    Montanha   UMETA(DisplayName = "Montanha"),
    Praia      UMETA(DisplayName = "Praia"),
    Vulcao     UMETA(DisplayName = "Vulcao"),
    MAX        UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FEng_BiomeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Savana;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float VegetationDensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float DinosaurSpawnRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float AmbientTemperature = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    FLinearColor FogColor = FLinearColor(0.4f, 0.6f, 1.0f, 1.0f);
};
```

> NOTE: C++ compilation is blocked by 218 UHT errors in the pre-built binary.
> All runtime changes go through UE5 Python (ue5_execute). C++ structs above
> are documentation targets for when the compilation pipeline is restored.

---

## Current World State (Validated this cycle)

### Hero Clearing Population (X=2100, Y=2400)
| Actor Label | Type | Scale | Status |
|-------------|------|-------|--------|
| TRex_Savana_001 | StaticMeshActor | 4x4x4 | ✅ Spawned |
| Raptor_Floresta_001 | StaticMeshActor | 2.5x2.5x2.5 | ✅ Spawned |
| Raptor_Floresta_002 | StaticMeshActor | 2.5x2.5x2.5 | ✅ Spawned |
| Trike_Savana_001 | StaticMeshActor | 3.5x3.5x3.0 | ✅ Spawned |
| Brachi_Savana_001 | StaticMeshActor | 6x6x8 | ✅ Spawned |
| Tree_Floresta_001..012 | StaticMeshActor | 2.5-3.5x scale | ✅ 12 trees |
| Fern_Floresta_001..008 | StaticMeshActor | 1.5x1.5x1.2 | ✅ 8 ferns |
| BiomeManager_Proxy_001 | Actor | — | ✅ Architecture anchor |

### CAP Enforcement (Applied this cycle)
- Sun pitch: -45° (golden hour), intensity=8.0, warm amber RGB(255,220,150)
- Fog: 1x ExponentialHeightFog, density=0.02, blue-sky inscattering
- SkyLight: real_time_capture=True, intensity=2.0
- FastSkyLUT=1 via console

---

## Agent Directives for Next Cycle

### Agent #3 — Core Systems Programmer
- Implement `DinosaurBase` movement component (patrol radius, idle animation trigger)
- Use existing `TRex_Savana_001`, `Raptor_Floresta_001/002` as test targets
- DO NOT spawn new dino actors — modify existing ones

### Agent #5 — World Generator
- Ground the floating terrain tiles (Z=0 baseline)
- Extend terrain to cover hero clearing with height variation (hills, not flat)
- Biome map: hero clearing = Floresta biome

### Agent #6 — Environment Artist
- Assign real mesh assets to Tree_Floresta_001..012 (use UE5 starter content trees)
- Assign fern meshes to Fern_Floresta_001..008
- Dense canopy over hero clearing is the priority

### Agent #8 — Lighting
- SkyAtmosphere must be present (fix void blue sky)
- Golden hour: sun at -45°, warm amber, volumetric god rays through tree canopy
- Target: hero clearing looks like a living Cretaceous forest at dusk

### Agent #9 — Character Artist
- Assign dinosaur mesh assets to TRex_Savana_001, Raptor_Floresta_001/002
- Use UE5 content browser assets or procedural mesh if real assets unavailable
- Pose: T-Rex facing camera (yaw=180°), Raptors flanking

---

## Architecture Decisions This Cycle

1. **BiomeManager as proxy actor** — Until C++ compilation is restored, the biome
   system is anchored by `BiomeManager_Proxy_001` at the hero clearing. All agents
   query this actor's location as the Floresta biome center.

2. **Naming convention enforced** — All 21 actors spawned this cycle follow
   `Type_Bioma_NNN` convention. No duplicates created (label_exists check).

3. **Hero clearing is the single quality bar** — Architecture decisions prioritize
   what is visible at X=2100, Y=2400. Abstract systems are deferred.

4. **No C++ writes** — Per global rule `hugo_no_cpp_h_v2`, all changes go through
   ue5_execute Python. C++ documentation above is for future pipeline restoration.

---

*Engine Architect #02 — PROD_CYCLE_AUTO_20260703_001*
