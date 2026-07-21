# BiomeManager — Architecture Specification (Cycle PROD_CYCLE_AUTO_20260713_008)
## Owner: Engine Architect #02 → Implementer: Core Systems Programmer #03

### CONTEXT
Live editor audit this cycle confirms `BiomeManager` does NOT exist as a compiled class
(`unreal.load_class(None, '/Script/TranspersonalGame.BiomeManager')` returns None).
This is the P1 architecture gap for World Generation. Below is the binding contract
#03 must implement — no deviation on names/types (SharedTypes.h enums must be reused,
not redefined).

### CLASS: `AEng_BiomeManager` (AActor subclass, one instance per level, spawned in MinPlayableMap)
NOTE: Prefixed `Eng_` because this is an architecture-owned type; #03 may rename to
project convention (`ABiomeManager`) ONLY if no existing class with that name exists —
verify via `unreal.load_class` before creating to avoid duplicate-type violations.

**Responsibilities (single source of truth for biome data — NOT a gameplay actor):**
1. Owns a `TArray<FEng_BiomeDefinition>` — one entry per biome (Forest, Savanna, Wetland,
   Volcanic Highlands, Coastal) — data-only struct, no logic.
2. Exposes `EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation)` — pure query,
   O(1) or O(log n) via a coarse grid lookup (NOT per-actor raycasts).
3. Exposes `FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType Type)` for consumers
   (FoliageManager, PCGWorldGenerator, dinosaur spawn tables, weather system).
4. Does NOT spawn meshes, foliage, or actors itself — it is a DATA + QUERY layer.
   FoliageManager and PCGWorldGenerator READ from it; they do not write to it at runtime.

### DATA CONTRACT (add to SharedTypes.h — check first, do not duplicate if present)
```cpp
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Forest,
    Savanna,
    Wetland,
    VolcanicHighlands,
    Coastal,
    None
};

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::Forest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float BaseTemperature = 25.0f; // Celsius, used by survival system (hunger/thirst/temperature stats)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    float HumidityLevel = 0.5f; // 0-1, drives fog density and vegetation density

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftClassPtr<AActor>> DinosaurSpeciesPool; // which dino types can spawn here

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> VegetationMeshPool;
};
```

### INTEGRATION RULES (enforced — violations block QA #18)
- `PCGWorldGenerator` MUST query `AEng_BiomeManager::GetBiomeAtLocation` before placing
  terrain features — no independent biome logic duplicated in PCGWorldGenerator.
- `FoliageManager` MUST query the biome's `VegetationMeshPool` — never hardcode mesh
  paths per-biome inside FoliageManager itself.
- Dinosaur spawn logic (future #12 Combat/AI work) MUST query `DinosaurSpeciesPool`
  per biome — this is how territorial/ecology rules (GDD P1/P2) stay data-driven.
- One `AEng_BiomeManager` instance per level — do NOT spawn duplicates. Verify with
  `unreal.GameplayStatics.get_all_actors_of_class` before spawning in Python setup scripts.

### WHY THIS SHAPE (justification)
Carmack principle applied: biome data must be a flat, query-only layer so every other
system (foliage, AI, weather, survival stats) reads the same ground truth instead of
each re-implementing biome detection (which is what currently risks happening across
#05/#06/#11/#12). This prevents 5 different "what biome am I in" implementations
drifting out of sync — a classic architecture debt trap for a multi-year project.

### CURRENT LIVE EDITOR STATE (verified this cycle via Remote Control)
- Core classes confirmed loadable: TranspersonalGameState, TranspersonalCharacter,
  PCGWorldGenerator, FoliageManager, CrowdSimulationManager, ProceduralWorldManager,
  BuildIntegrationManager, TranspersonalGameMode.
- `BiomeManager` class: NOT FOUND — confirms this is genuinely unbuilt, not a naming
  mismatch.
- Hub composition at (X=2100, Y=2400, r=2500): dinosaur and vegetation actor counts
  audited and logged to editor Output Log this cycle (see "ARCHITECTURE_AUDIT_COMPLETE_CYCLE_008"
  marker in log for exact counts).
- DirectionalLight count and intensity logged — used to confirm #01's single-sun CAP fix
  from cycle 007/008 is holding (no duplicate lights re-introduced).
- No duplicate actor labels detected in this pass (naming/dedup rule holding).

### NEXT STEPS FOR #03
1. Add `EEng_BiomeType` + `FEng_BiomeDefinition` to `SharedTypes.h` (check for existing
   biome-related types first — do not duplicate).
2. Implement `AEng_BiomeManager.h/.cpp` per contract above.
3. Populate 5 `FEng_BiomeDefinition` entries matching the existing MinPlayableMap biomes
   (the hub area at 2100/2400 reads as Forest — use that as the Forest reference values).
4. Wire `PCGWorldGenerator` and `FoliageManager` to query it (do not duplicate biome logic).
