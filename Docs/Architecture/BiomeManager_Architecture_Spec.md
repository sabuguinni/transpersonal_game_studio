# BiomeManager — Technical Architecture Specification
**Author:** Engine Architect (#02) | **Cycle:** PROD_CYCLE_AUTO_20260708_003
**Status:** APPROVED SPEC — no .cpp/.h emitted (see constraint note below)

## Constraint Note (read first)
Per `hugo_no_cpp_h_v2` (importance MAX, standing rule confirmed across 3 prior cycles):
this headless UE5 instance runs a **pre-built binary that never recompiles**. Writing new
`.cpp`/`.h` files via `github_file_write` is understood to be **100% inert** — it does not
change the running Editor, cannot be validated by Remote Control, and wastes tool budget.
This spec is therefore the deliverable of record for P1 (World Generation / Biome System);
implementation for Core Systems Programmer (#03) happens ONLY when a real build pipeline
(local compile + editor restart) is confirmed available. Until then, all live-world changes
continue via `ue5_execute` Python against the running instance, which is the only channel
proven to affect the actual game state.

## 1. Purpose
Defines a single authority for classifying world-space regions into biomes and exposing
per-biome environmental parameters (foliage density, dinosaur species pool, weather bias,
ambient temperature) to every downstream system (#05 World Gen, #06 Environment Artist,
#08 Lighting, #11 NPC Behavior, #12 Combat AI).

## 2. Class Contract (for #03 to implement when build pipeline exists)
```
UCLASS()
class TRANSPERSONALGAME_API ABiomeManager : public AActor
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Biome")
    EBiomeType GetBiomeAtLocation(const FVector& WorldLocation) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    TArray<EDinosaurSpecies> GetSpeciesPoolForBiome(EBiomeType Biome) const;

    UFUNCTION(BlueprintCallable, Category="Biome")
    float GetFoliageDensityMultiplier(EBiomeType Biome) const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Biome")
    TMap<EBiomeType, FBiomeParameters> BiomeTable;
};
```
Uses existing `EBiomeType` (7 values: Swampland, Forest, Savanna, Desert, Mountains,
RiverValley, Coastline) and `EDinosaurSpecies` (10 values) already defined in
`SharedTypes.h` — CONFIRMED via `github_file_read` this cycle. No new enums required;
Rule 3 (Shared Types) satisfied by reuse, zero duplication risk.

`FBiomeParameters` (new USTRUCT, global scope, `Eng_` prefix to avoid collision):
```
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeParameters
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FoliageDensity = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AmbientTemperatureC = 25.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<EDinosaurSpecies> SpeciesPool;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EWeatherType DominantWeather = EWeatherType::Clear;
};
```

## 3. Runtime Zone Mapping (region-lookup, not grid-authored)
Region classification uses distance-from-center + Perlin noise blend, computed once at
`BeginPlay` and cached into a `TMap<FIntPoint, EBiomeType>` keyed by 500-unit grid cells.
This avoids per-tick cost and keeps the O(1) lookup contract for #12 Combat AI queries.

## 4. Live-World Validation Performed This Cycle (via ue5_execute, not spec-only)
1. **Class audit** — confirmed 7/7 active classes (`TranspersonalGameState`,
   `TranspersonalCharacter`, `PCGWorldGenerator`, `FoliageManager`,
   `CrowdSimulationManager`, `ProceduralWorldManager`, `BuildIntegrationManager`) load
   via `unreal.load_class()` against the live binary — architecture contract intact,
   no missing/renamed classes since last cycle.
2. **Content-hub density audit** — scanned all actors within 1500u radius of
   (X=2100, Y=2400), the mandated hero-shot composition zone. Logged full actor list
   with distances and classes; checked for `hugo_naming_dedup_v2` violations (>3 actors
   sharing the same `Type_` prefix stacked at near-identical coordinates). No forced
   corrective spawn issued this cycle — audit-only, since Studio Director (#01) already
   added `Trike_ContentHub_002` / `Anky_ContentHub_003` last cycle and a second density
   pass now would risk overstacking.
3. **Artifact/lighting audit** — confirmed zero residual `Brush`/`Volume`-class actors
   remain after #01's cleanup pass, and confirmed exactly one `DirectionalLight` is
   present (architecture rule: single-sun-source, no duplicate directional lights that
   cause double-shadowing or bloom conflicts).

## 5. Dependencies / Next Steps
- **#03 Core Systems Programmer**: implement `ABiomeManager` per contract above the
  moment a real (non-headless) build pipeline is confirmed; until then, biome params
  can be simulated via `ue5_execute` by tagging existing actors with a `Biome` string
  metadata field for #05/#06 to read at runtime.
- **#05 World Generator**: needs `GetBiomeAtLocation` semantics confirmed before wiring
  PCG spawn rules to biome table — spec above is the frozen contract.
- **#06 Environment Artist**: `GetFoliageDensityMultiplier` return value drives per-biome
  foliage instance count multiplier in `FoliageManager`.
