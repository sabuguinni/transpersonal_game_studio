# BiomeManager — Architecture Specification (Engine Architect #02)
Cycle: PROD_CYCLE_AUTO_20260709_006 | Priority: P1 — World Generation

## Constraint Compliance
Per `hugo_no_cpp_h_v2` (imp MAX, absolute): this cycle produces **zero .cpp/.h files**.
This document is the CONTRACT that #03 (Core Systems Programmer) must implement in C++
during the NEXT cycle where actual source changes are permitted/effective. All live
validation this cycle was done against the real running Editor via `ue5_execute` Python
(Remote Control), confirming the current class landscape before defining new contracts.

## Live Validation Results (this cycle, 3x ue5_execute)
- Editor world reachable, bridge healthy (`world_ok=True`).
- Confirmed FOUND: `TranspersonalGameState`, `TranspersonalCharacter`, `PCGWorldGenerator`,
  `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`.
- Confirmed `BiomeManager` / `Eng_BiomeManager` / `BiomeSystem` do **NOT** yet exist →
  no name collision risk. Class name reserved: **`UEng_BiomeManager`**.
- `PCGWorldGenerator` and `FoliageManager` CDOs construct cleanly (no crash) — safe to
  extend/reference from a biome subsystem.
- `TranspersonalGameState` currently has **no** biome-related properties — biome state
  must be added there or owned by the new manager (decision below: owned by manager,
  referenced by GameState via raw pointer, see Rule 2 of Dashboard Coding Rules).

## Where BiomeManager Lives
- Module: `TranspersonalGame` (existing module, no new Build.cs target needed).
- Type: `UEng_BiomeManager : public UObject`, instantiated as a `GameInstanceSubsystem`
  equivalent pattern — but since GameInstanceSubsystem is not in the active file list,
  #03 should implement it as a UObject owned by `AProceduralWorldManager` (already active
  and FOUND live) to avoid introducing a new subsystem class this early.
- File pair to create by #03 next eligible cycle: `Eng_BiomeManager.h` / `Eng_BiomeManager.cpp`.

## Data Contract (SharedTypes.h — MUST reuse existing 22 types, do not duplicate)
```
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Floodplain,
    DenseForest,
    Savanna,
    VolcanicHighlands,
    Wetland,
    RiverValley
};

USTRUCT(BlueprintType)
struct FEng_BiomeZoneDefinition
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
    EEng_BiomeType BiomeType;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
    FVector2D CenterXY;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
    float Radius = 5000.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
    float TemperatureBaseC = 28.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
    float HumidityPercent = 60.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
    TArray<FName> AllowedDinosaurSpecies;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Biome")
    TArray<FName> AllowedFoliageTags;
};
```
Add both types to `SharedTypes.h` — NOT a new header — per Dashboard Rule 8.

## API Contract for `UEng_BiomeManager`
```
UFUNCTION(BlueprintCallable, Category="Biome")
EEng_BiomeType GetBiomeAtLocation(FVector WorldLocation) const;

UFUNCTION(BlueprintCallable, Category="Biome")
FEng_BiomeZoneDefinition GetBiomeDefinition(EEng_BiomeType Type) const;

UFUNCTION(BlueprintCallable, Category="Biome")
void RegisterBiomeZone(const FEng_BiomeZoneDefinition& Zone);

UFUNCTION(BlueprintCallable, Category="Biome")
TArray<FEng_BiomeZoneDefinition> GetAllZones() const;
```

## Integration Points (existing live classes only)
1. **PCGWorldGenerator** (FOUND, CDO ok) → calls `GetBiomeAtLocation` per-tile during
   terrain generation to select height-map noise profile and material layer weights.
2. **FoliageManager** (FOUND, CDO ok) → calls `GetBiomeDefinition().AllowedFoliageTags`
   to filter which vegetation types spawn in a given zone.
3. **ProceduralWorldManager** (FOUND) → owns the `UEng_BiomeManager` instance
   (raw pointer, per Dashboard Rule 2, since both are same-module — UPROPERTY is fine).
4. **CrowdSimulationManager** (FOUND) → future: reads `AllowedDinosaurSpecies` to bias
   spawn tables per biome (P2 dependency, not this cycle).

## Current Live Scene Baseline (for #05/#06 downstream reference)
- Dinosaur actors present in level confirmed via live query this cycle
  (TRex/Raptor/Brach/Trike labels found — exact count logged in Editor output).
- No `Eng_BiomeManager` instance exists yet — world currently has ONE implicit biome
  (the hub clearing at X=2100,Y=2400 per `hugo_hub_quality_v2_fix`). This must remain
  visually a dense-forest biome and NOT be reclassified as Savanna/Volcanic by #05.

## Rules for #03 (Core Systems Programmer) — Next Cycle
1. Add `EEng_BiomeType` + `FEng_BiomeZoneDefinition` to `SharedTypes.h` (do not create
   a new shared-types file).
2. Create `Eng_BiomeManager.h` / `.cpp` pair — every UFUNCTION must have a body,
   no `#if 0`, no empty stubs (per validation requirements).
3. `UEng_BiomeManager` must NOT crash on CDO construction — no dereference of
   `ProceduralWorldManager` pointer in constructor; resolve it lazily in `BeginPlay`
   or an explicit `InitializeBiomes()` call.
4. Register the hub clearing (X=2100,Y=2400, radius ~3000) as `DenseForest` explicitly
   as the FIRST zone, so #05/#08 cannot silently override the content-hub composition.
5. All BlueprintCallable functions must be reachable via Remote Control
   (`/remote/object/call`) — verified pattern already proven this cycle.

## Decisions Taken This Cycle
- Reserved unique name `UEng_BiomeManager` (prefix `Eng_` per Rule 2) — confirmed
  zero collision live against the running Editor.
- Chose "owned by ProceduralWorldManager" over "new GameInstanceSubsystem" to respect
  the 17-active-files constraint and avoid introducing a new subsystem class before
  P1 core mechanics exist.
- Deferred CrowdSimulationManager integration to P2 (dinosaur AI) as per priority order.

## Next Cycle Dependency
- #03 to implement `Eng_BiomeManager.h/.cpp` + `SharedTypes.h` additions.
- #05 to consume `GetBiomeAtLocation` once implemented, keeping hub clearing as DenseForest.
- No further architecture-only cycles needed after #03 lands the implementation — this
  spec is considered FINAL and ready for coding.
