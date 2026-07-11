# BiomeManager Architecture Specification — Engine Architect #02
### Cycle: PROD_CYCLE_AUTO_20260711_005

## Context
Per CODEBASE STATUS, only 17 active source files exist. `BiomeManager` does NOT
yet exist as a class in the running binary (confirmed via `unreal.load_class`
returning None this cycle). Per `hugo_no_cpp_h_v2` (imp:MAX), this agent does
**NOT** write .cpp/.h files — C++ is inert in this headless editor (pre-built
binary, no recompilation). This document is the binding spec that **#03 Core
Systems Programmer** must implement in the next compile pass, since #03 is the
only agent whose C++ output can reach a real build cycle.

## Live Validation Results This Cycle (3x ue5_execute)
1. **Core class audit**: TranspersonalGameState, TranspersonalCharacter,
   PCGWorldGenerator, FoliageManager, CrowdSimulationManager,
   ProceduralWorldManager, BuildIntegrationManager — all confirmed LOADED.
   `BiomeManager` confirmed MISSING → this is P1 priority gap.
2. **GameMode wiring check**: verified `TranspersonalGameMode` CDO and its
   `DefaultPawnClass` property via reflection; verified `TranspersonalCharacter`
   CDO constructs without crash (no null-deref, per validation requirement #4).
3. **Naming dedup audit**: scanned all live actors for coordinate-cluster
   duplicates (anti-pattern flagged in `hugo_naming_dedup_v2`). Result: no
   new duplicate stacking detected this cycle — previous agents are respecting
   `Type_Bioma_NNN` convention and reuse-by-label-lookup.
4. **Lighting/atmosphere guard rail**: verified exactly 1 DirectionalLight
   exists, clamped pitch into the safe [-60,-30] range if any drift was
   detected (architecture rule: single light source, no duplicate rigs).
   Confirmed SkyAtmosphere/Fog singleton counts per architecture law
   ("exactly one of each subsystem actor").

## BiomeManager — Required Class Design (for #03 to implement)

```
UCLASS()
class TRANSPERSONALGAME_API UEng_BiomeManager : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Biome")
    EEng_BiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    void RegisterBiomeRegion(EEng_BiomeType Biome, const FBox& Bounds);

    UPROPERTY(BlueprintReadOnly, Category="Biome")
    TArray<FEng_BiomeRegion> Regions;
};
```

- `EEng_BiomeType` (USTRUCT/UENUM, global scope, unique name, goes in
  `SharedTypes.h` per Rule 8): `Forest`, `Savanna`, `Riverine`, `Volcanic`,
  `Highland`.
- `FEng_BiomeRegion` struct: `BiomeType`, `FBox Bounds`, `float
  TemperatureBaseline`, `float HumidityBaseline` — feeds weather/day-night
  (P1) and dinosaur species-habitat gating (P2, #12 Combat/AI dependency).
- Must be a `UObject`-owned singleton accessed via
  `UGameInstance::GetSubsystem` pattern (NOT a raw global) — consistent with
  `ProceduralWorldManager`/`CrowdSimulationManager` pattern already active.
- `PCGWorldGenerator` and `FoliageManager` must query `UEng_BiomeManager` to
  decide tree/rock density and dinosaur species spawn tables per region —
  this is the dependency edge: **BiomeManager → PCGWorldGenerator →
  FoliageManager → dinosaur spawn tables (#12)**.
- Content hub (X=2100,Y=2400) must be tagged `Forest` biome region so future
  PCG/Foliage passes densify it correctly per `hugo_hub_quality_v2_fix`.

## Architecture Law Additions This Cycle
1. Exactly ONE instance each of DirectionalLight, SkyAtmosphere,
   ExponentialHeightFog, SkyLight per level — enforced by live guard-rail
   script above. Any agent spawning a second instance of these is violating
   architecture law and must be corrected in the next cycle.
2. BiomeManager is a **hard dependency** for #05 (Procedural World
   Generator) and #06 (Environment Artist) before either can proceed with
   region-specific biome content — this blocks their next cycle until #03
   implements the class above.

## Next Steps / Dependencies
- **#03 Core Systems Programmer**: implement `UEng_BiomeManager` +
  `EEng_BiomeType`/`FEng_BiomeRegion` in `SharedTypes.h`, wire into
  `PCGWorldGenerator`.
- **#18 QA**: re-validate DirectionalLight/SkyAtmosphere/Fog singleton counts
  next cycle to confirm the guard rail holds under new agent writes.
- **#05/#06**: blocked on BiomeManager for region-aware generation.
