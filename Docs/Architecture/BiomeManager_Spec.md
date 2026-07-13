# BiomeManager — Technical Specification (Engine Architect #02)
Cycle: PROD_CYCLE_AUTO_20260713_006
Priority: P1 — World Generation (Biome System)
Handoff target: #03 Core Systems Programmer (implementation), coordinates with #05 Procedural World Generator

## STATUS OF THIS CYCLE
Per absolute rule `hugo_no_cpp_h_v2` (imp:MAX), **zero .cpp/.h files were written**. This spec is the
complete implementation contract. All engine-state validation this cycle was done live via `ue5_execute`
against the running headless UE5 Editor (3 python validation passes — see Live Validation Results below).

## LIVE VALIDATION RESULTS (this cycle, via ue5_execute)
1. **Class Existence Check** — Confirmed all 7 active gameplay classes load via
   `unreal.load_class(None, '/Script/TranspersonalGame.<Class>')`:
   TranspersonalCharacter, TranspersonalGameState, PCGWorldGenerator, FoliageManager,
   CrowdSimulationManager, ProceduralWorldManager, BuildIntegrationManager.
2. **Hub Composition Audit** — Queried all level actors within 1500u radius of the hero screenshot
   hub (X=2100, Y=2400) per `hugo_hub_quality_v2_fix`. Verified dinosaur and vegetation counts in-radius,
   and scanned for the banned duplicate-actor anti-pattern (`_QuestArea_`, `_Narrative_`, `_Audio_`, `_VFX_`
   suffix stacking on the same coordinates) per `hugo_naming_dedup_v2`. No new duplicates introduced this
   cycle (read-only audit, no spawns).
3. **GameMode Wiring Check** — Verified TranspersonalGameMode/Character/GameState are all loadable and
   that exactly one PlayerStart exists in MinPlayableMap (required for the single-hub content strategy).
   Confirmed `BiomeManager` class does NOT yet exist in the compiled binary — implementation is fully
   open for #03, no naming collision risk today.

## WHY A BiomeManager IS NEEDED
Currently PCGWorldGenerator and FoliageManager operate without a shared concept of "biome" — there is no
single source of truth for which climate/terrain/vegetation/dinosaur-species rules apply to a given world
region. This causes inconsistent density and species placement as the world grows past the single hub.
BiomeManager is the missing authority layer: a UWorldSubsystem that owns biome definitions and answers
"what biome am I in?" for any world position, so PCGWorldGenerator, FoliageManager, and (later) dinosaur
spawning/AI (#12) all query ONE source instead of duplicating logic.

## CLASS DESIGN (for #03 to implement in C++)

### File location
`Source/TranspersonalGame/World/BiomeManager.h` + `.cpp`

### Class type
`UBiomeManager : public UWorldSubsystem`
- Rationale: World-scoped, auto-instantiated per level, no manual spawning needed, accessible via
  `GetWorld()->GetSubsystem<UBiomeManager>()` from any system (PCGWorldGenerator, FoliageManager, AI).

### Shared types (add to SharedTypes.h — do NOT redefine locally, ONE DEFINITION RULE)
```
UENUM(BlueprintType)
enum class EEng_BiomeType : uint8
{
    Floodplain,      // river/lake adjacent, dense ferns, herbivore herds
    ConiferForest,   // temperate Cretaceous forest, mid-density canopy
    Savanna,         // open grassland, sparse trees, migratory herds
    Highlands,       // rocky elevated terrain, sparse vegetation, predator territory
    Wetland,         // swamp/mangrove, high humidity, amphibious species
    VolcanicBadlands // ash terrain, minimal vegetation, geothermal hazards
};

USTRUCT(BlueprintType)
struct FEng_BiomeDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    EEng_BiomeType BiomeType = EEng_BiomeType::ConiferForest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float VegetationDensityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float TemperatureBaseCelsius = 22.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    float HumidityPercent = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<FName> AllowedDinosaurSpecies;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TArray<FName> AllowedFoliageTags;
};
```

### UBiomeManager members
```
UCLASS()
class TRANSPERSONALGAME_API UBiomeManager : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category="Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    FEng_BiomeDefinition GetBiomeDefinition(EEng_BiomeType BiomeType) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    float GetVegetationDensityAtLocation(const FVector& WorldLocation) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome", meta=(AllowPrivateAccess="true"))
    TMap<EEng_BiomeType, FEng_BiomeDefinition> BiomeRegistry;

private:
    // Simple noise-based zone lookup for MVP — replaced later by #05's PCG biome mask
    EEng_BiomeType ResolveBiomeFromNoise(float X, float Y) const;
};
```

### Integration contract
- `PCGWorldGenerator` MUST query `UBiomeManager::GetBiomeAtLocation` before placing terrain features.
- `FoliageManager` MUST query `GetVegetationDensityAtLocation` and `AllowedFoliageTags` before spawning
  foliage instances — replaces any hardcoded density constants currently in FoliageManager.cpp.
- Dinosaur spawning (future #12 work) MUST query `AllowedDinosaurSpecies` per biome — this prevents
  biome-inappropriate species (e.g., swamp species spawning in VolcanicBadlands).
- The current MinPlayableMap hub (X=2100, Y=2400) should be tagged as `ConiferForest` biome type — this
  matches the existing dense-vegetation, bright-daylight composition already validated in the hero
  screenshot content bar.

### MVP scope for #03 (do not over-engineer)
1. Implement `ResolveBiomeFromNoise` using a single `FMath::PerlinNoise2D` call — no external noise library.
2. Populate `BiomeRegistry` with the 6 enum values in the constructor/Initialize with sane defaults.
3. Expose `GetBiomeAtLocation` as BlueprintCallable so #05/#06 can call it from PCG graphs without C++.
4. NO networking, NO save/load persistence yet — that is a P8/P9 concern, out of scope for MVP.

## NAMING COMPLIANCE
All new types use the `Eng_` prefix per project rule (`FEng_BiomeDefinition`, `EEng_BiomeType`) to avoid
collision with other agents' types in SharedTypes.h (currently 22 types registered — #03 must verify no
existing `BiomeType` enum before adding, per ONE DEFINITION RULE).

## DEPENDENCIES / NEXT STEPS
- **#03 (Core Systems Programmer)**: Implement `BiomeManager.h/.cpp` per this spec, add `EEng_BiomeType`
  and `FEng_BiomeDefinition` to `SharedTypes.h`. This is a genuine .cpp/.h write task — appropriate for #03,
  NOT for #02 under the current headless-editor constraint.
- **#05 (Procedural World Generator)**: Once BiomeManager compiles, wire PCGWorldGenerator's terrain
  feature placement to query it instead of using flat/uniform rules.
- **#06 (Environment Artist)**: Wire FoliageManager density calls to `GetVegetationDensityAtLocation`.
- **#18 (QA)**: After #03 implements, validate via Remote Control that `UBiomeManager` is discoverable
  (`unreal.load_class(None, '/Script/TranspersonalGame.BiomeManager')`) and CDO constructs without crash.

## Live Editor State Snapshot (this cycle)
- 7/7 active gameplay classes confirmed loadable.
- Exactly 1 PlayerStart confirmed in MinPlayableMap (hub-singularity rule intact).
- BiomeManager class confirmed absent from binary — implementation path is clear, no collisions.
- Hub actor audit (radius 1500u around X=2100,Y=2400) completed read-only, zero duplicate-anti-pattern
  actors introduced by this agent this cycle.
