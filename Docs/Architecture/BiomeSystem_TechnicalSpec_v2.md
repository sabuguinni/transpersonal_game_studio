# Biome System — Technical Architecture Specification v2
**Agent**: #02 Engine Architect  
**Cycle**: PROD_CYCLE_AUTO_20260617_005  
**Status**: ACTIVE — deployed in MinPlayableMap

---

## Overview

The Biome System defines spatial zones across the game world. Each biome has distinct ecological properties that drive dinosaur behavior, environmental conditions, resource distribution, and survival challenges.

---

## Biome Zones (Deployed in MinPlayableMap)

| Label | Location (X,Y,Z) | Biome Type | Key Characteristics |
|-------|-----------------|------------|---------------------|
| Arch_Savanna_Center | (3000, 0, 150) | Savanna | Open plains, high visibility, herbivore herds, T-Rex territory |
| Arch_Forest_Center | (-3000, 0, 150) | Dense Forest | Low visibility, Raptor packs, rich resources, ambush danger |
| Arch_Swamp_Center | (0, 3000, 100) | Swamp | Slow movement, disease risk, Spinosaurus, unique plants |
| Arch_Mountain_Center | (0, -3000, 400) | Mountain | High altitude, cold, Pterosaur nests, rare minerals |
| Arch_River_Crossing | (1500, 1500, 120) | River/Transition | Water source, crossing point, predator ambush zone |

---

## Biome Data Schema

Each biome zone must expose these properties for downstream agents:

```cpp
// BiomeData — used by World Generator (#05), Environment Artist (#06), Combat AI (#12)
struct FBiomeData {
    FName BiomeID;              // "Savanna", "Forest", "Swamp", "Mountain", "River"
    float Temperature;          // Celsius — affects player stamina drain
    float Humidity;             // 0.0-1.0 — affects disease risk
    float Visibility;           // 0.0-1.0 — affects predator detection range
    float MovementModifier;     // 1.0 = normal, 0.7 = swamp slow
    TArray<FName> DinoSpecies;  // Species that inhabit this biome
    TArray<FName> Resources;    // Available crafting resources
    FLinearColor FogColor;      // Atmospheric color for Lighting agent (#08)
    float FogDensity;           // Exponential height fog density
};
```

---

## Agent Dependency Map

```
Engine Architect (#02)
    └── BiomeSystem Architecture
            ├── World Generator (#05) — terrain height per biome, PCG rules
            ├── Environment Artist (#06) — foliage density, rock types per biome
            ├── Lighting (#08) — fog color, sun angle, atmosphere per biome
            ├── Combat AI (#12) — dinosaur species per biome, patrol zones
            └── Survival (#09) — temperature/humidity effects on player stats
```

---

## Biome Transition Rules

1. **Savanna → Forest**: Gradual tree density increase over 500 units
2. **Forest → Swamp**: Terrain drops 50 units, fog density doubles
3. **Swamp → River**: Water plane appears, movement penalty activates
4. **Savanna → Mountain**: Terrain rises 300+ units, temperature drops
5. **All biomes**: NavMesh must be continuous — no isolated islands

---

## Spatial Layout (Top-Down)

```
         MOUNTAIN (-3000Y)
              |
FOREST ---- CENTER ---- SAVANNA
(-3000X)     |          (+3000X)
             |
           SWAMP (+3000Y)
           
    RIVER CROSSING at (1500, 1500)
    connects Savanna ↔ Swamp
```

---

## Performance Constraints (from Agent #04)

- Max 8000 actors total in scene
- Max 150 dinosaurs simultaneously active
- Biome zones use streaming: only 2 adjacent biomes fully loaded at once
- LOD chain: Full detail within 2000 units, reduced at 5000, culled at 10000

---

## Validation Status (Cycle 005)

| Check | Status |
|-------|--------|
| Biome landmark actors deployed | ✅ 5 deployed |
| Map saved | ✅ MinPlayableMap |
| NavMesh bounds present | ✅ Verified |
| PlayerStart present | ✅ Verified |
| Architecture integrity | ✅ PASS |

---

## Next Steps for Downstream Agents

- **#05 World Generator**: Use biome center coordinates to drive PCG terrain rules
- **#06 Environment Artist**: Place foliage clusters within 2000 units of each biome center
- **#08 Lighting**: Apply fog color per biome zone (Savanna=warm amber, Swamp=green, Mountain=blue)
- **#12 Combat AI**: Assign dinosaur species to biome zones using DinoSpecies array
