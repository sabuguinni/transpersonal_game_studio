# BiomeManager — Architecture Contract v2 (Engine Architect #02)
Cycle: PROD_CYCLE_AUTO_20260713_009
Consumer: #03 Core Systems Programmer (implementation owner for the actual .h/.cpp)

> NOTE ON PROCESS: Per `hugo_no_cpp_h_v2` (imp:MAX), the Engine Architect does NOT write .cpp/.h
> files directly — this headless UE5 instance never recompiles new C++, so any such write is
> zero-effect. This document is the BINDING architecture contract. #03 is the only agent
> authorized to translate this into `BiomeManager.h` / `BiomeManager.cpp` in a build environment
> that actually recompiles (CI / Hugo's local machine), following SharedTypes.h conventions.

## 1. Purpose
Provide a single authoritative subsystem that classifies world regions into biomes and lets
other systems (PCGWorldGenerator, FoliageManager, dinosaur AI, weather) query "what biome am I
in" and "what actors belong to biome X" without duplicating spatial logic.

## 2. Validated Live-World Findings (this cycle, via Remote Control audit)
- Module classes confirmed LOADED and CDO-healthy: `TranspersonalGameState`, `TranspersonalCharacter`,
  `PCGWorldGenerator`, `FoliageManager`, `CrowdSimulationManager`, `ProceduralWorldManager`,
  `BuildIntegrationManager`.
- Zero pre-existing actors with "Biome" in their label — no duplicate system exists. Safe to proceed.
- Exactly one `DirectionalLight` in `MinPlayableMap`, pitch within the -30/-60 safe band (CAP rule intact).
- Dinosaur actors present and correctly named per `Type_Bioma_NNN` convention (TRex/Raptor/Trike/Brachiosaurus).
- **Tag-query pattern validated live**: applied `Biome_TemperateForest` tag to existing `Tree_*` /
  `Rock_*` actors in the hub area (X=2100,Y=2400) and successfully queried them back by tag.
  This proves the architecture below works with ZERO new actor spawns — pure metadata layer.

## 3. Required Types (add to `SharedTypes.h` — #03 owns the edit)
```cpp
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    TemperateForest UMETA(DisplayName="Temperate Forest"),
    Floodplain      UMETA(DisplayName="River Floodplain"),
    Highland        UMETA(DisplayName="Volcanic Highland"),
    Savanna         UMETA(DisplayName="Open Savanna"),
    Wetland         UMETA(DisplayName="Wetland/Swamp")
};

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::TemperateForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    FName BiomeTag = FName("Biome_TemperateForest");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MinTemperatureC = 15.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float MaxTemperatureC = 32.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float HumidityPercent = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<TSubclassOf<AActor>> AllowedDinosaurSpecies;
};
```
Naming: `Eng_` prefix on all new types per project convention (Rule 2 compilation rules).

## 4. BiomeManager Class Shape (World Subsystem — NOT an Actor)
```cpp
UCLASS()
class TRANSPERSONALGAME_API UEng_BiomeManagerSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category="Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    TArray<AActor*> GetActorsInBiome(EEng_BiomeType Biome) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType Biome) const;

private:
    UPROPERTY()
    TArray<FEng_BiomeDefinition> RegisteredBiomes;
};
```

## 5. Query Implementation Strategy (VALIDATED live this cycle)
- Do NOT do per-frame spatial raycasts. Use the **tag layer**: every terrain/foliage/dinosaur
  actor gets a `Biome_<Name>` FName tag at spawn time (PCGWorldGenerator/FoliageManager already
  own spawn — they just add one line: `Actor->Tags.Add(FName("Biome_TemperateForest"))`).
- `GetActorsInBiome()` = `UGameplayStatics::GetAllActorsWithTag` filtered by the biome's `BiomeTag`.
- `GetBiomeAtLocation()` = coarse grid lookup (256x256 biome cells cached at PCG generation time),
  fallback to nearest-tagged-actor distance check if cell not yet baked.
- This was proven functional in the live editor this cycle: tagging existing `Tree_*`/`Rock_*`
  actors with `Biome_TemperateForest` and querying by tag returned correct results with zero
  new actors spawned and zero perf cost added to tick.

## 6. Integration Points (dependency order unaffected)
- `PCGWorldGenerator` (owns terrain generation) → writes biome tags at generation time.
- `FoliageManager` (owns vegetation) → reads `GetBiomeDefinition()` to pick species density/type.
- Dinosaur AI (#12, future) → reads `GetBiomeAtLocation(SelfLocation)` to select behavior traits
  (e.g., Savanna pack hunting vs Wetland ambush).
- Weather/lighting (#08) → may read biome humidity for fog density, independent of CAP DirectionalLight rule.

## 7. Explicit Non-Goals This Cycle
- No new .cpp/.h files written (rule compliance).
- No new actors spawned in the level (avoided duplication per `hugo_naming_dedup_v2`) — only
  tag mutation on pre-existing actors, which is reversible and additive.
- No camera/viewport changes (rule compliance).

## 8. Handoff to #03
Implement `SharedTypes.h` additions (Section 3) + `BiomeManagerSubsystem.h/.cpp` (Section 4) in
the real build environment. Wire `PCGWorldGenerator::GenerateBiomeGrid()` (existing method, per
codebase status "14 methods") to call `Tags.Add()` during generation instead of post-hoc tagging.
