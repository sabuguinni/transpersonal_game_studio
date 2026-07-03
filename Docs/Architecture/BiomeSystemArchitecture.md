# Biome System Architecture — Engine Architect #02
## PROD_CYCLE_AUTO_20260703_005

### Overview
The Biome System is the foundational world-generation architecture for the Transpersonal Game Studio prehistoric survival game. It defines how terrain, vegetation, dinosaur populations, and environmental conditions are organized across the game world.

---

## Core Architecture Principles

### 1. World Partition Integration
- All biomes operate within UE5 World Partition (mandatory for worlds >4km²)
- Each biome cell = 512×512m streaming unit
- LOD streaming: 3 levels (near/mid/far) per biome cell

### 2. Biome Hierarchy
```
WorldBiomeManager (UGameInstanceSubsystem)
├── BiomeRegistry (static data table)
├── ActiveBiomeGrid (runtime 2D array)
├── BiomeTransitionZones (blend regions)
└── BiomeCellPool (streaming management)
```

### 3. Biome Types (Cretaceous Period)
| ID | Name | Climate | Primary Fauna | Vegetation Density |
|----|------|---------|---------------|-------------------|
| 0 | Coastal Lowlands | Humid/Warm | Mosasaur, Pterosaur | Medium |
| 1 | Cretaceous Forest | Tropical | T-Rex, Raptors | Very High |
| 2 | Open Savanna | Semi-arid | Triceratops, Ankylosaur | Low |
| 3 | River Delta | Wetland | Spinosaurus, Iguanodon | High |
| 4 | Highland Plateau | Cool/Dry | Pachycephalosaurus | Low |
| 5 | Volcanic Badlands | Hot/Toxic | Abelisaurus | Very Low |

---

## Data Structures

### FBiomeCell (per 512×512m cell)
```cpp
USTRUCT(BlueprintType)
struct FEng_BiomeCell {
    UPROPERTY() int32 BiomeID;
    UPROPERTY() float Temperature;      // Celsius
    UPROPERTY() float Humidity;         // 0.0–1.0
    UPROPERTY() float VegetationDensity; // 0.0–1.0
    UPROPERTY() float DinosaurDensity;  // 0.0–1.0
    UPROPERTY() FVector2D GridCoords;   // World partition cell
    UPROPERTY() TArray<int32> ActiveDinoSpecies; // Species IDs present
};
```

### FBiomeTransition (blend zones between biomes)
```cpp
USTRUCT(BlueprintType)
struct FEng_BiomeTransition {
    UPROPERTY() int32 BiomeA;
    UPROPERTY() int32 BiomeB;
    UPROPERTY() float BlendWidth;   // meters
    UPROPERTY() ECurveType BlendCurve; // Linear/Smooth/Step
};
```

---

## System Interfaces

### IBiomeQuery (read-only runtime queries)
```
GetBiomeAtLocation(FVector WorldPos) → FEng_BiomeCell
GetTemperatureAtLocation(FVector WorldPos) → float
GetVegetationDensityAtLocation(FVector WorldPos) → float
GetActiveDinoSpecies(FVector WorldPos) → TArray<int32>
```

### IBiomeEvents (event bus for cross-system communication)
```
OnBiomeCellLoaded(FEng_BiomeCell Cell)
OnBiomeCellUnloaded(FEng_BiomeCell Cell)
OnBiomeTransitionEntered(int32 FromBiome, int32 ToBiome)
OnWeatherChanged(int32 BiomeID, EWeatherType NewWeather)
```

---

## Integration Points

### → Core Systems (#03)
- Biome provides terrain collision data per cell
- Physics parameters vary by biome (mud density, water depth)

### → Procedural World Generator (#05)
- BiomeManager seeds the PCG graph per cell
- Vegetation placement driven by VegetationDensity property

### → Environment Artist (#06)
- Biome ID determines material layer blending
- Foliage type tables indexed by BiomeID

### → Dinosaur AI (#12)
- Species spawn tables filtered by ActiveDinoSpecies per cell
- Territory radius checked against biome boundaries

### → Weather/Atmosphere (#08)
- Temperature + Humidity → weather probability tables
- Fog density driven by Humidity value

---

## Hub Clearing Composition (X=2100, Y=2400)
Current scene state (validated PROD_CYCLE_AUTO_20260703_005):
- **Biome**: Cretaceous Forest (ID=1)
- **Dinosaurs**: TRex_Savana_001, Raptor_Savana_001/002/003, Trike_Savana_001, Brachio_Savana_001
- **Vegetation**: 16 trees (Tree_Floresta_050–065), 20 fern patches (Fern_Hub_000–019)
- **Lighting**: Sun pitch -35°, warm amber, PostProcess cinematic mode
- **Status**: READY for Environment Artist mesh replacement

---

## Implementation Priority (P1 — World Generation)

### Phase 1 (Current): Scene Composition ✅
- Hub clearing populated with dinosaur actors
- Vegetation ring established
- Lighting/atmosphere configured

### Phase 2 (Next 2 cycles): BiomeManager C++ Class
- `UEng_BiomeManager : UGameInstanceSubsystem`
- Data table integration with `FEng_BiomeCell`
- Runtime query interface

### Phase 3: PCG Integration
- PCG graph nodes per biome type
- Streaming cell management
- Transition zone blending

### Phase 4: Full World Partition
- 16×16 grid of 512m cells = 8km² world
- Async streaming with priority queue
- LOD chain per biome type

---

## Architectural Rules (Enforced)

1. **No biome-specific code in character classes** — query through IBiomeQuery only
2. **No hardcoded biome IDs** — always use BiomeRegistry enum
3. **Biome transitions are always gradual** — minimum 50m blend width
4. **Dinosaur spawning is biome-gated** — species cannot appear outside their biome
5. **Weather is biome-local** — each cell has independent weather state
6. **World Partition is mandatory** — no monolithic level loading

---

*Engine Architect #02 — PROD_CYCLE_AUTO_20260703_005*
*Next: Agent #03 (Core Systems) implements physics parameters per biome*
