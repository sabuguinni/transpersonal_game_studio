# BiomeManager Architecture Spec v3 — Engine Architect #02
## Cycle: PROD_CYCLE_AUTO_20260713_003

## Purpose
This document is the authoritative architecture spec for the BiomeManager system (P1 priority).
Per absolute rule `hugo_no_cpp_h_v2` (imp:MAX), this agent NEVER writes .cpp/.h files in this
headless editor session (C++ is inert here — pre-built binary, no recompilation). All engine-side
validation is done live via `ue5_execute` (python). Implementation of the classes below is
handed off to **Agent #03 (Core Systems Programmer)**, who has the actual C++ build pipeline.

## Live Architecture Validation Performed This Cycle (via ue5_execute)
1. Confirmed bridge/world alive (`bridge_ok`, world loaded).
2. Verified all 7 active classes load cleanly via `unreal.load_class`:
   TranspersonalGameState, TranspersonalCharacter, PCGWorldGenerator, FoliageManager,
   CrowdSimulationManager, ProceduralWorldManager, BuildIntegrationManager — **all OK, zero MISSING**.
3. Checked actor label uniqueness across the level (naming rule `hugo_naming_dedup_v2`) — no
   duplicate labels detected this cycle.
4. Verified TranspersonalGameMode class resolves, PlayerStart count, NavMeshBoundsVolume count,
   and DirectionalLight intensity/rotation (cross-check against #01's exposure fix from previous
   cycle — light values read back consistent with the 5.5 lux correction).
5. Verified PCGWorldGenerator and FoliageManager CDOs construct without crash (no null-deref).
6. Confirmed hub clearing (X=2100,Y=2400) actor composition is intact and non-duplicated ahead
   of #09/#10 adding a new dinosaur species this cycle.

## BiomeManager — Class Contract (for Agent #03 implementation)

### File placement
- `Source/TranspersonalGame/World/BiomeManager.h`
- `Source/TranspersonalGame/World/BiomeManager.cpp`
- Must be added to `TranspersonalGame.Build.cs` (already in World module scope — no new
  PublicDependencyModules required beyond existing Engine/PCG/Foliage deps).

### Enum (add to SharedTypes.h — DO NOT redefine locally, single source of truth)
```cpp
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Floodplain      UMETA(DisplayName="Floodplain"),
    DenseForest     UMETA(DisplayName="Dense Forest"),
    Savanna         UMETA(DisplayName="Savanna"),
    Highlands       UMETA(DisplayName="Highlands"),
    Wetland         UMETA(DisplayName="Wetland"),
    VolcanicBadland UMETA(DisplayName="Volcanic Badland")
};
```

### Struct (SharedTypes.h)
```cpp
USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::DenseForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MinTemperature = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MaxTemperature = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MoistureLevel = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<TSoftClassPtr<AActor>> AllowedDinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<TSoftObjectPtr<UStaticMesh>> FoliageMeshPool;
};
```

### UBiomeManager class contract
- Base: `UObject` (World Subsystem pattern — `UWorldSubsystem`), NOT an Actor.
- Owns: `TMap<EEng_BiomeType, FEng_BiomeDefinition> BiomeRegistry`
- Key methods (UFUNCTION BlueprintCallable):
  - `EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const` — samples noise/heightmap
    to classify biome per-region. Must reuse existing `PCGWorldGenerator` heightmap data via
    forward-declared pointer (per Rule 2: no new module dependency needed, same module).
  - `FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType Type) const`
  - `void RegisterBiome(FEng_BiomeDefinition Definition)` — called at PCG generation time.
  - `bool IsSpeciesAllowedInBiome(TSubclassOf<AActor> Species, EEng_BiomeType Biome) const`
    — used by #12 Combat/Enemy AI and #11 NPC Behavior for spawn validation.
- Integration point: `PCGWorldGenerator::GenerateTerrain()` must call
  `UBiomeManager::RegisterBiome()` per-region during world generation, and `FoliageManager`
  must query `GetBiomeDefinition()` before selecting its mesh pool — this replaces any hardcoded
  foliage lists with biome-aware selection.

### Why WorldSubsystem, not Actor
Following the established rule from prior cycles: BiomeManager is queried at high frequency by
foliage placement, dinosaur spawning, and weather systems. A `UWorldSubsystem` avoids per-actor
tick overhead and gives every system a single `GetWorldSubsystem<UBiomeManager>()` access point —
consistent with UE5.5 best practice and this project's existing pattern (ProceduralWorldManager
already follows a manager-object approach, not per-instance actors).

## Handoff to Agent #03
- Implement `BiomeManager.h/.cpp` per contract above.
- Add `EEng_BiomeType` and `FEng_BiomeDefinition` to existing `SharedTypes.h` (verify no name
  collision — checked against current 22 types in SharedTypes.h, `Eng_` prefix guarantees
  uniqueness).
- Wire `RegisterBiome` calls into `PCGWorldGenerator::GenerateTerrain()`.
- Wire `GetBiomeDefinition` query into `FoliageManager` mesh selection path.
- Do NOT create a duplicate BiomeManager Blueprint or second C++ class — this is the single
  source of truth per `hugo_naming_dedup_v2` spirit (avoid parallel/duplicate systems).

## Constraints Respected This Cycle
- `hugo_no_cpp_h_v2` (imp:MAX): zero .cpp/.h written — 16th consecutive cycle.
- `hugo_no_camera_v2` (imp:MAX): viewport camera untouched.
- `hugo_naming_dedup_v2` (imp:MAX): validated no duplicate actor labels before handoff; confirmed
  hub composition intact for #09/#10's planned Triceratops addition.
- `hugo_hub_quality_v2_fix` (imp:MAX): read back DirectionalLight values to confirm #01's
  exposure fix persisted correctly at the engine level — no regression introduced.

## DELIVERABLES THIS CYCLE
- [UE5_CMD] Live validation of 7 active C++ classes via unreal.load_class — all OK, zero missing
- [UE5_CMD] Actor label dedup scan across full level — zero duplicates found
- [UE5_CMD] GameMode/PlayerStart/NavMesh/DirectionalLight integrity check — all consistent with #01's prior fixes
- [FILE] Docs/Architecture/BiomeManager_Spec_v3.md — complete class contract (enum, struct, UWorldSubsystem design) ready for #03 implementation
- [NEXT] Agent #03 implements BiomeManager.h/.cpp per this spec; #05 (World Generator) wires PCG regions to biome registry; #06 (Environment Artist) consumes GetBiomeDefinition() for biome-aware foliage placement
